#include "synth_bu.h"
#include "../utils/expr2sygus.h"
#include "../utils/execute_expr.h"
#include "../utils/util.h"
#include <util/arith_tools.h>
#include <iostream>
#include <util/mathematical_expr.h>
#include <util/replace_expr.h>
#include <ranges>
#include <util/expr_iterator.h>



bool replace_first_expr(const exprt &what, const exprt &by, exprt &dest)
{
  PRECONDITION_WITH_DIAGNOSTICS(
      what.type() == by.type(),
      "types to be replaced should match. old type:\n" + what.type().pretty() +
          "\nnew type:\n" + by.type().pretty());

  bool no_change = true;

  for (auto it = dest.depth_begin(), itend = dest.depth_end();
       it != itend;) // no ++it
  {
    if (*it == what)
    {
      it.mutate() = by;
      return false;
    }
    else
      ++it;
  }

  return no_change;
}
// Function to do cartesian product of N sets of exprts
std::vector<std::vector<exprt>> cartesian(std::vector<std::vector<exprt>> &sets)
{
  std::vector<std::vector<exprt>> temp(1, std::vector<exprt>());
  for (int i = 0; i < sets.size(); i++)
  {
    std::vector<std::vector<exprt>> newTemp;
    for (const std::vector<exprt> &product : temp)
    {
      for (const exprt &element : sets[i])
      {
        std::vector<exprt> tempCopy = product;
        tempCopy.push_back(element);
        newTemp.push_back(tempCopy);
      }
    }
    temp = newTemp;
  }
  return temp;
}

bool bottom_up_syntht::initialise_program_pool()
{
  // initialise program set with all terminals in the grammar
  for (const auto &nt : grammar.nt_ids)
  {
    (*current_pool)[nt] = std::set<exprt>();
    for (const auto &nt_rule : grammar.production_rules.at(nt))
    {
      if (!contains_nonterminal(nt_rule, grammar))
      {
        (*current_pool)[nt].insert(nt_rule);
        if(verify_against_counterexamples(nt_rule))
          return true;
      }
    }
  }
  return false;
}

void bottom_up_syntht::empty_current_pool()
{
  (*current_pool).clear();
  // initialise program set with an empty set for each nonterminal
  for (const auto &nt : grammar.nt_ids)
  {
    (*current_pool)[nt] = std::set<exprt>();
  }
}

std::string print_pool(std::map<irep_idt, std::set<exprt>> &pool)
{
  std::string result;
  for (const auto &nt : pool)
  {
    result += id2string(nt.first) + ": ";
    for (const auto &expr : nt.second)
    {

      result += expr2sygus(expr) + ", ";
    }
    result += "\n\n";
  }
  return result;
}


bool bottom_up_syntht::is_observationally_unique(const exprt &expr, const irep_idt &nt)
{
  if(counterexamples.size()==0)
    return true;
  // create lamda expression
  lambda_exprt lambda(problem.synthesis_functions[0].parameters, expr);
  bool is_new=false;
  std::vector<int> outputs;
  for(int i=0; i<counterexamples.size(); i++)
  {
    std::vector<exprt> inputs;
    // construct inputs for the counterexample
    for(const auto &p: problem.synthesis_functions[0].parameters)
    {
      inputs.push_back(counterexamples[i].assignment.at(p));
    }
    exprt fapp = lambda.application(inputs);
    if(fapp.type().id()==ID_bool)
      outputs.push_back(execute_boolean_expr(fapp)?1:0);
    else if(fapp.type().id()==ID_integer)
      outputs.push_back(execute_integer_expr(fapp));
    else
      std::cout<<"observational equivalence not support for this type"<<std::endl;
  }
  std::cout<<"cex: " << counterexamples.size()<<" outputs: ";
  for(const auto &o: outputs)
    std::cout<<o<<", ";
  if(counterexample_results[nt].insert(outputs).second == true)
    is_new=true;
  if(is_new)
    std::cout<<"new program: "<<expr2sygus(expr)<<std::endl;
  else
    std::cout<<"not new: "<<expr2sygus(expr)<<std::endl;
  return is_new;
}


bool bottom_up_syntht::get_next_programs()
{
  // in each iteration we switch set the current pool to be the previous pool
  // and clear the current pool
  std::swap(current_pool, prev_pool);
  empty_current_pool();
  if (prev_pool->size() == 0)
  {
    return initialise_program_pool();
  }
  // get all possible combinations of programs for each nonterminal
  for (const auto &nt : grammar.nt_ids)
  {
    // for each production rule, we generate a new program for
    // every combination of replacements for the nonterminal in the rule
    for (const auto &rule : grammar.production_rules.at(nt))
    {
      std::vector<symbol_exprt> nonterminals = list_nonterminals(rule, grammar);
      std::vector<std::vector<exprt>> sets_of_progs;
      for (const auto &nt_id : nonterminals)
      {
        std::vector<exprt> progs;
        for (const auto &p : (*prev_pool)[nt_id.get_identifier()])
        {
          progs.push_back(p);
        }
        sets_of_progs.push_back(progs);
      }
      for (const auto &tuple : cartesian(sets_of_progs))
      {
        exprt new_expr = rule;
        for (int i = 0; i < tuple.size(); i++)
        {
          replace_first_expr(nonterminals[i], tuple[i], new_expr);
        }
        // std::cout << "adding " << expr2sygus(new_expr)<<std::endl;
        // TODO: check observational equivalence here before adding to the pool
        if (basic_simplify(new_expr) && is_observationally_unique(new_expr, nt))
        {
         if((*prev_pool).at(nt).find(new_expr) == (*prev_pool).at(nt).end())
          {
            (*current_pool)[nt].insert(new_expr);
            if (nt == grammar.start)
            {
              if(verify_against_counterexamples(new_expr))
                return true;
            }
          }
        }
      }
    }
  }
  return false;
}

solutiont bottom_up_syntht::get_solution() const
{
  return last_solution;
}

void bottom_up_syntht::set_program_size(std::size_t size)
{
  program_size = size;
}

void bottom_up_syntht::add_counterexample(const counterexamplet &cex)
{
  counterexamples.push_back(cex);
  // initialise the map of cex results
  if(counterexample_results.size()==0)
  {
    for(const auto &nt: grammar.nt_ids)
      counterexample_results[nt] = std::set<std::vector<int>>();
  }
}

void bottom_up_syntht::create_distributions()
{
  if (grammar.production_rule_weights.size() == 0)
  {
    std::cout << "adding default weights" << std::endl;
    add_grammar_weights(grammar);
  }
  for (auto &nt : grammar.production_rule_weights)
  {
    distributions[nt.first] = std::discrete_distribution<int>(nt.second.begin(), nt.second.end());
    std::cout << "created distribution for " << nt.first << std::endl;
  }
}

bool bottom_up_syntht::verify_against_counterexamples(const exprt & expr)
{
  std::cout << "checking solution " << expr2sygus(expr) << std::endl;
      last_solution.functions[symbol_exprt(problem.synthesis_functions[0].id,
                                           problem.synthesis_functions[0].type)] =
          lambda_exprt(problem.synthesis_functions[0].parameters, expr);
  if (counterexamples.size() == 0)
    return true;
  else if (cex_verifier(problem, last_solution, counterexamples) == mini_verifyt::resultt::PASS)
  {
    std::cout << "counterexample verifier passed " << std::endl;
        return true;
 }
 else
 {
   std::cout << "counterexample verifier failed " << std::endl;
   return false;
 }
}


bottom_up_syntht::resultt bottom_up_syntht::operator()()
{
  std::size_t iteration = 0;
  while (true)
  {
    std::cout<<"iteration "<<iteration<<std::endl;
    // maybe put this in a timeout
    if(get_next_programs())
      return CANDIDATE;
    iteration++;
  }
  return NO_SOLUTION;
}