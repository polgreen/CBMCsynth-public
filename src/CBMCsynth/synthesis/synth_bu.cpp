#include "synth_bu.h"
#include "../utils/expr2sygus.h"
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

void bottom_up_syntht::initialise_program_pool()
{
  // initialise program set with all terminals in the grammar
  for (const auto &nt : grammar.nt_ids)
  {
    (*current_pool)[nt] = std::set<exprt>();
    for (const auto &nt_rule : grammar.production_rules.at(nt))
    {
      if (!contains_nonterminal(nt_rule, grammar))
        (*current_pool)[nt].insert(nt_rule);
    }
  }
}

void bottom_up_syntht::empty_current_pool()
{
  (*current_pool).clear();
  // initialise program set with all terminals in the grammar
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
    result += "\n";
  }
  return result;
}

void bottom_up_syntht::get_next_programs()
{
  // in each iteration we switch set the current pool to be the previous pool
  // and clear the current pool
  std::swap(current_pool, prev_pool);
  empty_current_pool();
  if (prev_pool->size() == 0)
  {
    initialise_program_pool();
    return;
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
        std::cout << "adding " << expr2sygus(new_expr)<<std::endl;
        // TODO: simplify here
        basic_simplify(new_expr);
        std::cout << " simplified " << expr2sygus(new_expr) << std::endl;
        (*current_pool)[nt].insert(new_expr);
      }
    }
  }
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

bottom_up_syntht::resultt bottom_up_syntht::operator()()
{
  std::cout << "starting enumeration" << std::endl;
  while (true)
  {
    // maybe put this in a timeout
    get_next_programs();

    for (const auto &p : (*current_pool)[grammar.start])
    {
      std::cout << "Pool of solutions" << grammar.start << std::endl;
      std::cout << expr2sygus(p) << std::endl;
      last_solution.functions[symbol_exprt(problem.synthesis_functions[0].id,
                                           problem.synthesis_functions[0].type)] =
          lambda_exprt(problem.synthesis_functions[0].parameters, p);
      // check against counterexamples
      if (counterexamples.size() == 0)
      {
        std::cout << "No counterexamples, returning candidate" << std::endl;
        return CANDIDATE;
      }
      else if (cex_verifier(problem, last_solution, counterexamples) == mini_verifyt::resultt::PASS)
      {
        std::cout << "counterexample verifier passed " << std::endl;
        return CANDIDATE;
      }
      else
      {
        std::cout << "counterexample verifier failed " << std::endl;
      }
    }
  }
  return NO_SOLUTION;
}