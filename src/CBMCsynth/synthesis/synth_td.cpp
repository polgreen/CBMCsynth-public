#include "synth_td.h"
#include "syntactic_feedback.h"
#include "../utils/expr2sygus.h"
#include "../utils/util.h"
#include <util/arith_tools.h>
#include <iostream>
#include <util/mathematical_expr.h>

std::mt19937 rng(rand());


std::string print_sequence(std::vector<unsigned> &sequence)
{
  std::string result;
  for(auto &s: sequence)
  {
    result+=std::to_string(s);
    result+=" ";
  }
  return result;
}


top_down_syntht::enum_resultt top_down_syntht::replace_nts(exprt &expr, std::size_t &current_depth, std::vector<unsigned> &sequence)
{
  enum_resultt result = enum_resultt::NO_CHANGE;
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
          while (result==enum_resultt::NO_CHANGE)
          {
            std::size_t random_index = distributions[symbol.get_identifier()](rng);
            sequence.push_back(random_index);
            if (prev_solutions.find(sequence) == prev_solutions.end())
            {
              expr = rules[random_index];
              result = enum_resultt::CHANGED;
            }
            else
            {
              sequence.pop_back();
            }
          }
        }
        else
        {
          // replace with a terminal
          for (unsigned i = 0; i < rules.size(); i++)
          {
            if (!contains_nonterminal(rules[i], grammar))
            {
              sequence.push_back(i);
              if (prev_solutions.find(sequence) == prev_solutions.end())
              {
                expr = rules[i];
                result = enum_resultt::CHANGED;
                break;
              }
              else
              {
                // we've seen this before
                sequence.pop_back();
              }
            }
          }
          if(result == enum_resultt::NO_CHANGE)
          {
            // should have replaced this nonterminal
            return enum_resultt::ABORT;
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
    for(auto &op: expr.operands())
    {
      switch(replace_nts(op, current_depth, sequence))
      {
        case enum_resultt::ABORT:
          return enum_resultt::ABORT;
        case enum_resultt::CHANGED:
          result = enum_resultt::CHANGED;
          break;
        case enum_resultt::NO_CHANGE:
          break;
      }
    }
  }
  return result;
}


void top_down_syntht::top_down_enumerate()
{
  syntactic_feedbackt feedback(problem, grammar, message.get_message_handler());
  feedback.update_grammar = update_grammar;
  exprt current_program = symbol_exprt(grammar.start, grammar.start_type);

  // enumerate through the grammar until a complete program is found
  std::vector<unsigned> sequence;
  bool no_solution=true;
  while (no_solution)
  {
    message.debug()<<"partial program: "<<expr2sygus(current_program)<<messaget::eom;
    std::size_t num_nonterminals = count_symbol_occurrences(current_program, grammar.nt_ids);
    // if we only have one nonterminal, ask the LLM for guidance
    if(num_nonterminals>=1 && use_syntactic_feedback  &&
      ((enumerations_since_LLM>frequency_of_LLM_calls && num_LLM_calls<max_LLM_calls) || 
      (iter==0 && !called_LLM_this_iter)))
    {
      called_LLM_this_iter=true;
      message.status()<<"called LLM. "<<messaget::eom;
      enumerations_since_LLM=0;
      num_LLM_calls++;
      std::size_t new_funcs = feedback.augment_grammar(current_program, problem);
      if(new_funcs>0)
      {

        create_distributions();
        message.status()<<"got "<< new_funcs <<" functions from LLM."<<messaget::eom;
        // resets the bonus weights so the next time we create distributions the counts
        // are reset
        if(use_bonus_weights)
          subtract_bonus_weights(grammar);

        if(feedback.last_solution.id()!=ID_nil)
        {
          message.debug()<<"LLM gave us a candidate solution: "<<expr2sygus(feedback.last_solution)<<messaget::eom;
          last_solution.functions[symbol_exprt(problem.synthesis_functions[0].id, 
          problem.synthesis_functions[0].type)] = 
          lambda_exprt(problem.synthesis_functions[0].parameters, feedback.last_solution);
          return;
        }
      }
    }
    else
    {
      enumerations_since_LLM++;
    }
    if(num_nonterminals==0)
    {
      prev_solutions.insert(sequence);
      no_solution=false;
      // we have a complete program
      break;
    }
    std::size_t current_depth = 0;
    // TODO: now we are counting the number of nonterminals, we can just exit
    // if we find none.
    // if we didn't replace anything, we had a complete program. Exit the loop
    switch(replace_nts(current_program, current_depth, sequence))
    {
      case enum_resultt::ABORT:
        current_program = symbol_exprt(grammar.start, grammar.start_type);
        sequence.clear();
        break;
      case enum_resultt::CHANGED:

        break;
      case enum_resultt::NO_CHANGE:
        // should never get here
        throw "No change in top down enumeration";
        break;
    }
  }
  message.debug() << "Complete prog: " << expr2sygus(current_program) << messaget::eom;

  last_solution.functions[symbol_exprt(problem.synthesis_functions[0].id, 
  problem.synthesis_functions[0].type)] = 
  lambda_exprt(problem.synthesis_functions[0].parameters, current_program);
}

solutiont top_down_syntht::get_solution() const
{
  return last_solution;
}

void top_down_syntht::set_program_size(std::size_t size)
{
  program_size = size;
}

void top_down_syntht::add_counterexample(const counterexamplet &cex)
{
  counterexamples.push_back(cex);
}


void top_down_syntht::create_distributions()
{
  if(grammar.production_rule_weights.size()==0)
  {
    message.debug()<<"adding default weights"<< messaget::eom;
    add_grammar_weights(grammar);
  }
  for (auto &nt : grammar.production_rule_weights)
  {
    distributions[nt.first] = std::discrete_distribution<int>(nt.second.begin(), nt.second.end());
    message.debug()<<"created distribution for "<<id2string(nt.first)<<messaget::eom;
  }
}

top_down_syntht::resultt top_down_syntht::operator()(std::size_t iteration)
{
  iter = iteration;
  called_LLM_this_iter=false;
  message.status()<<"starting enumeration"<<messaget::eom;
  while(true)
  {
    // maybe put this in a timeout
    top_down_enumerate();
    // check against counterexamples
    if(counterexamples.size()==0)
    {
      message.debug()<<"No counterexamples, returning candidate"<<messaget::eom;
      return CANDIDATE;
    }
    else if(cex_verifier(problem, last_solution, counterexamples)==counterexample_verifyt::resultt::PASS)
    {
      message.debug()<<"counterexample verifier passed "<<messaget::eom;
      return CANDIDATE;
    }
    else
    {
      message.debug()<<"counterexample verifier failed "<<messaget::eom;
    }
  }
  return NO_SOLUTION;
}