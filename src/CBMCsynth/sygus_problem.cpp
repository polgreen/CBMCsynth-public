//
// Created by julian on 21.03.23.
//

#include "sygus_problem.h"
#include "utils/util.h"
#include "utils/expr2sygus.h"

#include <fstream>
#include <iostream>
#include <set>

#include <util/cmdline.h>
#include <util/cout_message.h>
#include <util/format_expr.h>
#include <util/namespace.h>
#include <util/replace_symbol.h>
#include <util/simplify_expr.h>
#include <util/symbol_table.h>
#include <util/std_expr.h>
#include <util/arith_tools.h>
#include <analyses/invariant_set.h>

#include <solvers/smt2/smt2_dec.h>
#include "smt2_frontend.h"
// #include "term_position.h"

#define UNEXPECTEDCASE(S) PRECONDITION_WITH_DIAGNOSTICS(false, S);

irep_idt nonterminalID(const typet &type)
{
  if (type.id() == ID_integer)
    return "NTInt";
  else if (type.id() == ID_bool)
    return "NTbool";
  else
  {
    UNEXPECTEDCASE("unsupported type in production rule type : " + type.id_string());
  }
}

std::map<irep_idt, std::vector<exprt>> production_rules(const synth_funt &f)
{
  irep_idt int_id = nonterminalID(integer_typet());
  irep_idt bool_id = nonterminalID(bool_typet());
  symbol_exprt NT_integer = symbol_exprt(int_id, integer_typet());
  symbol_exprt NT_bool = symbol_exprt(bool_id, bool_typet());
  std::vector<exprt> integer_rules;
  std::vector<exprt> bool_rules;
  std::map<irep_idt, std::vector<exprt>> rules;

  auto func_type = to_mathematical_function_type(f.type);
  for(const auto &p: f.parameters)
  {
    if(p.type().id()==ID_integer)
      integer_rules.push_back(p);
    else if(p.type().id()==ID_bool)
      bool_rules.push_back(p);
    else
    {
      UNEXPECTEDCASE("synthesis function parameter of unexpected type: " + p.type().id_string());
    }
  }
  // constants
  integer_rules.push_back(from_integer(0, integer_typet()));
  integer_rules.push_back(from_integer(1, integer_typet()));

  integer_rules.push_back(unary_minus_exprt(NT_integer));
  integer_rules.push_back(plus_exprt(NT_integer, NT_integer));
  integer_rules.push_back(minus_exprt(NT_integer, NT_integer));
  integer_rules.push_back(if_exprt(NT_bool, NT_integer, NT_integer));

  bool_rules.push_back(not_exprt(NT_bool));
  bool_rules.push_back(and_exprt(NT_bool, NT_bool));
  bool_rules.push_back(or_exprt(NT_bool, NT_bool));
  bool_rules.push_back(if_exprt(NT_bool, NT_bool, NT_bool));
  bool_rules.push_back(equal_exprt(NT_integer, NT_integer));
  bool_rules.push_back(less_than_exprt(NT_integer, NT_integer));
  bool_rules.push_back(greater_than_exprt(NT_integer, NT_integer));

  rules[int_id] = integer_rules;
  rules[bool_id] = bool_rules;

  return rules;
}

synth_funt copy_fun_add_grammar(const synth_funt &f)
{
  synth_funt result = f;
  add_grammar(result);
  return result;
}

void add_grammar(synth_funt &f)
{
  f.grammar.nt_ids.clear();
  f.grammar.production_rules.clear();

  auto func = to_mathematical_function_type(f.type);
  for (const auto &t : func.domain())
  {
    if (t.id() == ID_unsignedbv || t.id() == ID_signedbv)
      UNEXPECTEDCASE("bitvector default grammar not supported yet");
  }

  f.grammar.nt_ids.push_back(nonterminalID(func.codomain()));
  f.grammar.production_rules = production_rules(f);

  for (const auto &r : f.grammar.production_rules)
  {
    if (r.first == nonterminalID(func.codomain()))
      continue;
    f.grammar.nt_ids.push_back(r.first);
  }
  f.grammar.start = f.grammar.nt_ids[0];
  f.grammar.start_type = func.codomain();
}

void subtract_bonus_weights(syntactic_templatet &g)
{
  std::cout<<"Resetting bonus weights"<<std::endl;
  for (const auto &r: g.production_rules)
  {
    auto &weights = g.production_rule_weights[r.first];
    auto &bonus = g.bonus_weights[r.first];
    for(unsigned i=0; i<weights.size(); i++)
    {
      weights[i] -= bonus[i];
      bonus[i] = 0;
    }
  }
}



void add_grammar_weights(syntactic_templatet &g)
{
  INVARIANT(g.production_rules.size() > 0, "grammar must have production rules");
  g.production_rule_weights.clear();
  for (const auto &r : g.production_rules)
  {
    std::vector<unsigned> weights;
    for (unsigned int i=0; i<r.second.size(); i++)
    {
      weights.push_back(1);
    }
    g.production_rule_weights[r.first] = weights;
  }
  g.bonus_weights.clear();
  for (const auto &r : g.production_rules)
  {
    std::vector<unsigned> weights;
    for (unsigned int i=0; i<r.second.size(); i++)
    {
      weights.push_back(0);
    }
    g.bonus_weights[r.first] = weights;
  }

}

void add_grammar_to_problem(sygus_problemt &problem)
{
  for (auto &f : problem.synthesis_functions)
  {
    add_grammar(f);
    add_grammar_weights(f.grammar);
  }
}

std::vector<exprt> sygus_problemt::nnf_constraints() const
{
  std::vector<exprt> result;
  for (auto &c : constraints)
  {
    std::cout<<"trying to NNF constraint "<< expr2sygus(c)<<std::endl;
    exprt copy = c;
    nnf(copy, false);
    std::cout<<"got "<< expr2sygus(copy)<<std::endl;
    std::cout<<"now DNFing" <<std::endl;
    dnf(copy);
    std::cout<<"got "<< expr2sygus(copy)<<std::endl;
    result.push_back(copy);
  }
  return result;
}

syntactic_templatet& sygus_problemt::get_grammar()
{
  PRECONDITION_WITH_DIAGNOSTICS(synthesis_functions.size() == 1, "only one synthesis function supported");
  if (synthesis_functions[0].grammar.nt_ids.size() == 0)
  {
    std::cout << "No grammar found, adding a default grammar" << std::endl;
    add_grammar_to_problem(const_cast<sygus_problemt &>(*this));
  }
  return synthesis_functions[0].grammar;
}

void parse_probs(std::string filename, sygus_problemt& problem)
{
   std::ifstream prob_file(filename);
   std::vector<int> weights;
   int weight;
   while(prob_file >> weight)
   {
     weights.push_back(weight);
   }
   if(weights.size()==0)
      UNEXPECTEDCASE("no weights in probability file or file not found");

   if(problem.synthesis_functions.size()!=1)
     UNEXPECTEDCASE("only one synthesis function supported, if probability file is given");
  
    auto &grammar = problem.synthesis_functions[0].grammar;
    // TODO: parse into weights
    std::size_t count=0;

    for(const auto &nt_id: grammar.nt_ids)
    {
      for(unsigned i=0; i<grammar.production_rules[nt_id].size(); i++)
      {
        std::cout<<"adding weight "<<weights[count]<<" to "<<expr2sygus(grammar.production_rules[nt_id][i])<<std::endl;
        if(count>=weights.size())
          UNEXPECTEDCASE("not enough weights in probability file");
        grammar.production_rule_weights[nt_id].push_back(weights[count]);
        count++;
      }
    }
}

bool contains_nonterminal(const exprt &expr, const syntactic_templatet& grammar)
{
  if (expr.id() == ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if (grammar.production_rules.find(symbol.get_identifier()) != grammar.production_rules.end())
    {
      return true;
    }
  }
  else
  {
    for (auto &op : expr.operands())
    {
      if (contains_nonterminal(op, grammar))
        return true;
    }
  }
  return false;
}

void count_nonterminals(const exprt &expr, const syntactic_templatet& grammar, std::size_t &count)
{

  if (expr.id() == ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if (grammar.production_rules.find(symbol.get_identifier()) != grammar.production_rules.end())
    {
      count++;
    }
  }
  else
  {
    for (auto &op : expr.operands())
    {
     count_nonterminals(op, grammar, count);
    }
  }
}

std::size_t count_nonterminals(const exprt &expr, const syntactic_templatet& grammar)
{
  std::size_t count=0;
  count_nonterminals(expr, grammar, count);
  return count;
}

void list_nonterminals(const exprt &expr, const syntactic_templatet& grammar, std::vector<symbol_exprt> &list)
{

  if (expr.id() == ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if (grammar.production_rules.find(symbol.get_identifier()) != grammar.production_rules.end())
    {
      list.push_back(to_symbol_expr(expr));
    }
  }
  else
  {
    for (auto &op : expr.operands())
    {
     list_nonterminals(op, grammar, list);
    }
  }
}


std::vector<symbol_exprt> list_nonterminals(const exprt &expr, const syntactic_templatet& grammar)
{
  std::vector<symbol_exprt> list;
  list_nonterminals(expr, grammar, list);
  return list;
}

void get_nonterminal_counts(const exprt &expr, const syntactic_templatet& grammar, std::unordered_map<irep_idt, unsigned> &counts)
{
  if (expr.id() == ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if (grammar.production_rules.find(symbol.get_identifier()) != grammar.production_rules.end())
    {
      if(counts.find(symbol.get_identifier())==counts.end())
        counts[symbol.get_identifier()]=1;
      else
        counts[symbol.get_identifier()]++;
    }
  }
  else
  {
    for (auto &op : expr.operands())
    {
     get_nonterminal_counts(op, grammar, counts);
    }
  }
}

std::unordered_map<irep_idt, unsigned> get_nonterminal_counts(const exprt &expr, const syntactic_templatet& grammar)
{
  std::unordered_map<irep_idt, unsigned> counts;
  get_nonterminal_counts(expr, grammar, counts);
  return counts;
}