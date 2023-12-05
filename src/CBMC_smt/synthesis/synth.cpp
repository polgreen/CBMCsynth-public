#include "synth.h"
#include "../utils/expr2sygus.h"
#include <util/arith_tools.h>
#include <iostream>

std::mt19937 rng(0);

bool syntht::contains_nonterminal(const exprt &expr)
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
      if (contains_nonterminal(op))
        return true;
    }
  }
  return false;
}

bool syntht::replace_nts(exprt &expr, std::size_t &current_depth)
{
  bool replaced_something=false;
  if (expr.id() == ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if (grammar.production_rules.find(symbol.get_identifier()) != grammar.production_rules.end())
    {
      auto &rules = grammar.production_rules.at(symbol.get_identifier());
      if (rules.size() > 0)
      {
        if (current_depth < program_size)
        {
          // randomly pick nonterminal
          std::size_t random_index = distributions[symbol.get_identifier()](rng);
          expr = rules[random_index];
          replaced_something=true;
        }
        else
        {
          // replace with a terminal
          for (auto &rule : rules)
          {
            if (!contains_nonterminal(rule))
            {
              expr = rule;
              replaced_something=true;
            }
          }
          if(replaced_something==false)
          {
            // there was no terminal so we make up a terminal of the correct type
            replaced_something=true;
            if (symbol.type().id() == ID_integer)
              expr = from_integer(0, integer_typet());
            else if (symbol.type().id() == ID_bool)
              expr = true_exprt();
            else
              UNEXPECTEDCASE("Unsupported type in enumeration " + id2string(symbol.get_identifier()));
          }
        }
      }
      else
      {
        UNEXPECTEDCASE("No production rules for nonterminal " + id2string(symbol.get_identifier()));
      }
    }
  }
  else
  {
    current_depth++;
    for (auto &op : expr.operands())
    {
      if (replace_nts(op, current_depth))
        replaced_something=true;
    }
  }
  return replaced_something;
}


void syntht::top_down_enumerate()
{
  exprt current_program = symbol_exprt(grammar.start, grammar.start_type);
  // enumerate through the grammar until a complete program is found
  while (true)
  {
    std::size_t current_depth = 0;
    // if we didn't replace anything, we had a complete program. Exit the loop
    if (!replace_nts(current_program, current_depth))
      break;
  }
  std::cout << "Complete prog: " << expr2sygus(current_program) << std::endl;
  last_solution.functions[symbol_exprt(problem.synthesis_functions[0].id, problem.synthesis_functions[0].type)] = current_program;
}

solutiont syntht::get_solution() const
{
  return last_solution;
}

void syntht::set_program_size(std::size_t size)
{
  program_size = size;
}


void syntht::create_distributions()
{
  if(grammar.production_rule_weights.size()==0)
  {
    std::cout<<"adding default weights"<<std::endl;
    add_grammar_weights(grammar);
  }
  for (auto &nt : grammar.production_rule_weights)
  {
    distributions[nt.first] = std::discrete_distribution<int>(nt.second.begin(), nt.second.end());
    std::cout<<"created distribution for "<<nt.first<<std::endl;
  }
}

syntht::resultt syntht::operator()()
{
  std::cout<<"starting enumeration"<<std::endl;
  top_down_enumerate();
  return CANDIDATE;
}