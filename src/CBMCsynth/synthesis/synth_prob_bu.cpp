#include "synth_prob_bu.h"
#include "../utils/util.h" 
#include "../utils/expr2sygus.h"
#include <iostream>


double prob_bu_syntht::calculate_program_score(const double &rule_score, const std::vector<exprt> &operands, const exprt &new_expr)
{
  double new_score = rule_score;
  for(const auto &op : operands)
  {
    if(program_score.find(op) != program_score.end())
    {
      new_score *= program_score[op];
    }
  }
  return new_score;
}


void prob_bu_syntht::initialise_program_pool() 
{
  // initialise program set with all terminals in the grammar
  for (const auto &nt : grammar.nt_ids)
  {
    (*current_pool)[nt] = std::set<exprt>();
    for(unsigned i=0; i<grammar.production_rules.at(nt).size(); i++)
    {
      if (!contains_nonterminal(grammar.production_rules.at(nt)[i], grammar))
      {
        const auto &prog = grammar.production_rules.at(nt)[i];
        (*current_pool)[nt].insert(prog);
        program_score[prog] = normalised_rule_weights.at(nt)[i];
      }
    }
  }
}

void prob_bu_syntht::get_next_programs()
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
    for(unsigned i=0; i<grammar.production_rules.at(nt).size(); i++)
    {
      const auto &rule = grammar.production_rules.at(nt)[i];
      const auto &score = normalised_rule_weights.at(nt)[i];
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
        for (unsigned i = 0; i < tuple.size(); i++)
        {
          replace_first_expr(nonterminals[i], tuple[i], new_expr);
        }
        double new_score = calculate_program_score(score, tuple, new_expr);
        // TODO: simplify here
        basic_simplify(new_expr);
        if((*current_pool)[nt].insert(new_expr).second)
        {
          program_score[new_expr] = new_score;
        }
      }
    }
  }
}

void prob_bu_syntht::setup_rule_weights()
{
  for(const auto &nt : grammar.nt_ids)
  {
    double total_weight = 0; 
    for(unsigned i=0; i<grammar.production_rules.at(nt).size(); i++)
    {
      total_weight += grammar.production_rule_weights.at(nt)[i];
    }
    normalised_rule_weights[nt] = std::vector<double>();
    for(unsigned i=0; i<grammar.production_rules.at(nt).size(); i++)
    {
      normalised_rule_weights[nt].push_back((double)grammar.production_rule_weights.at(nt)[i]/total_weight);
    }
  }
}

prob_bu_syntht::resultt prob_bu_syntht::operator()()
{
  setup_rule_weights();
  initialise_program_pool();
  for(const auto &p : (*current_pool)[grammar.start])
  {
    sorted_solutions_to_check.insert(std::pair<double, exprt>(program_score[p], p));
  }

  // we do not consider any programs with scores lower than this threshold. 
  // we decrease this threshold by a factor of 10 each iteration.
  double threshold=0.00001;

  while (true)
  {
    threshold = threshold/10;
    // check solutions in the list
    std::multimap<double, exprt>::iterator iter = sorted_solutions_to_check.begin();
    while(iter != sorted_solutions_to_check.end())
    {
      // some lower bound on probability threshold for programs
      if(program_score[iter->second] < threshold)
      {
        break;
      }      
      message.debug()<<"candidate score is "<< program_score[iter->second]<<messaget::eom;
      if(verify_against_counterexamples(iter->second))
      {
        iter = sorted_solutions_to_check.erase(iter); 
        return CANDIDATE;
      }
      else
      {
        iter = sorted_solutions_to_check.erase(iter);
      }
    }
    // get the next programs
    get_next_programs();

    // remove equivalent programs
    // TO DO

    // add programs to the solution list
    for(const auto &p : (*current_pool)[grammar.start])
    {
      if(program_score.find(p) == program_score.end())
      {
        program_score[p] = 1;
      }
      sorted_solutions_to_check.insert(std::pair<double, exprt>(program_score[p], p));
    }
  }
  return NO_SOLUTION;
}