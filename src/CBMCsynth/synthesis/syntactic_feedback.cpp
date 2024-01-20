
#include "syntactic_feedback.h"
#include "../utils/expr2python.h"
#include "../utils/expr2sygus.h"
#include "../utils/util.h"
#include "../openai/openai.hpp"
#include "../parsing/parser.h"

#include <util/replace_expr.h>
#include <util/mathematical_expr.h>
#include <iostream>
#include <istream>

// define debug to avoid calling openai
// #define DEBUG

// takes in a partial candidate
// returns false if there is no valid solution that can be made from this candidate
bool syntactic_feedbackt::partial_evaluation(const exprt &expr, const counterexamplet &cex)
{
  // check there's only 1 nonterminal left
  // does there exists a solution that can be made from this candidate that satisfies the counterexamples
  return true;
}

std::string syntactic_feedbackt::build_prompt(const exprt &partial_function)
{

  std::string prompt = "You are teaching a student to program. The student must write a function that satisfies the following constraints:\n";
  for (const auto &c : problem.constraints)
  {
    prompt += "assert(" + expr2python(c) + ")\n";
  }

  prompt += "\nSo far, the student has written this code:\n";
  prompt += synth_fun_to_python(problem.synthesis_functions[0], partial_function,
                                problem.synthesis_functions[0].grammar.nt_ids);
  prompt += "\n\n";
  prompt += "Can you suggest some helper functions for the student to use to complete this code?\n";
  prompt += "Print only the code and nothing else\n.";
  return prompt;
}


std::string syntactic_feedbackt::build_smt_prompt(const exprt &partial_function)
{
  std::string prompt = "You are teaching a student to write SMT-LIB. The student must write a function that satisfies the following constraints:\n";
  if(expand_fun_apps)
  {
    for(const auto &c: problem.constraints)
    {
      exprt expanded_c = c;
      expand_function_applications(expanded_c, problem.defined_functions);
      prompt += "(constraint (" + expr2sygus(expanded_c) + ")\n";
    }
  }
  else
  {
  // std::set<symbol_exprt> defined_functions;
  // for(const auto &c: problem.constraints)
  //   get_defined_functions(c, problem.defined_functions, defined_functions);

  // for(const auto & f: defined_functions)
  //   prompt += fun_def(f, problem.defined_functions[f]) + "\n";

  for(const auto &c: problem.constraints)
    prompt += "(constraint (" + expr2sygus(c) + ")\n";
  }

  if (use_cex_in_prompt && last_cex.assignment.size() > 0)
  {
    prompt += "\nThe last solution the student tried was this, but the teacher marked this solution incorrect:\n";

    prompt +=
        fun_def(symbol_exprt(problem.synthesis_functions[0].id, problem.synthesis_functions[0].type),
                last_solution);
    prompt += "\nThis solution was incorrect because it did not work for the following inputs:\n";
    for(const auto & c: last_cex.assignment)
    {
      prompt +=expr2sygus(c.first) + "  =  " + expr2sygus(c.second)+ "\n";
    }

    prompt += "\n\nThe student is trying again.";
  }
  prompt += "\nSo far, the student has written this code:\n";

  prompt +=
      fun_def(symbol_exprt(problem.synthesis_functions[0].id, problem.synthesis_functions[0].type),
              lambda_exprt(problem.synthesis_functions[0].parameters, partial_function));

  prompt += "\n\n";

  prompt += "Can you suggest some helper functions for the student to use to complete this code and replace the ??\n";
  prompt += "\nYou must print only the code and nothing else.\n";
  return prompt;
}

bool syntactic_feedbackt::add_to_grammar(const irep_idt &id, const exprt &expr)
{
  auto &type = expr.type();
  bool added = false;
  for (const auto &rules : problem.get_grammar().production_rules)
  {
    if (rules.first == "Const")
      continue;
    if (rules.second[0].type() == type)
    {
      problem.get_grammar().production_rules[rules.first].push_back(expr);
      auto max = std::max_element(
          problem.get_grammar().production_rule_weights[rules.first].begin(),
          problem.get_grammar().production_rule_weights[rules.first].end());
      // TODO: HEURISTIC this is a heuristic to decide how likely it is that we pick the new production rules
      if (id == problem.synthesis_functions[0].id)
      {
        last_solution = expr;
        problem.get_grammar().production_rule_weights[rules.first].push_back(*max * 5);
        problem.get_grammar().bonus_weights[rules.first].push_back(*max * 4);
      }
      else
      {
        problem.get_grammar().production_rule_weights[rules.first].push_back(*max);
        problem.get_grammar().bonus_weights[rules.first].push_back(0);
      }
      added = true;
    }
  }
  return added;
}

std::size_t syntactic_feedbackt::augment_grammar(const exprt &partial_function,
                                                 sygus_problemt &problem)
{
  // debugs without calling openai
#ifdef DEBUG
  std::string response = "(define-fun max ((x Int) (y Int)) Int (ite (> x y) x y))";
#else
  // generate openAI query
  openai::start(); // Will use the api key provided by `OPENAI_API_KEY` environment variable
  replace_mapt replace_map;
  for (const auto &rule : problem.get_grammar().production_rules)
  {
    replace_map[symbol_exprt(rule.first, rule.second[0].type())] = symbol_exprt("??", rule.second[0].type());
  }
  auto partial_function_copy = partial_function;
  replace_expr(replace_map, partial_function_copy);

  std::string prompt = build_smt_prompt(partial_function_copy);
  message.debug() << "prompt is " << prompt << messaget::eom;

  openai::Json messages;
  messages["role"] = "user";
  messages["content"] = prompt;
  openai::Json j;
  j["model"] = "gpt-3.5-turbo";
  j["messages"] = openai::Json::array({messages});
  // j["temperature"] = 0.5; // heuristic

  auto completion = openai::chat().create(j);

  // The following ugly code converts the json into a string, removes
  // extra stuff from the string and then pipes the string to an istream so the
  // parser can parse it.
  std::ostringstream oss;
  oss << completion["choices"][0]["message"]["content"];

  std::string response = oss.str();
  response.erase(std::remove(response.begin(), response.end(), '\"'),
                 response.end());

  // remove all occurences of the substring "\\n" in the string
  // these appear because the json library uses raw string literals
  std::string substr = "\\n";
  std::string::size_type n = substr.length();
  for (std::string::size_type i = response.find(substr);
       i != std::string::npos;
       i = response.find(substr))
    response.erase(i, n);
  
  // erase all characters before the first "("
  response.erase(0, response.find("("));
#endif
  message.debug() << "LLM response: " << response << messaget::eom;
  std::istringstream str(response);
  parsert parser(str);
  parser.add_defined_functions(problem.defined_functions);

  last_solution = nil_exprt();
  std::size_t new_functions = 0;
  try
  {
    if (response.find("define-fun") == std::string::npos)
    {
      parser.add_symbols(problem.free_var);
      last_solution = parser.parse_expression();
      add_to_grammar(problem.synthesis_functions[0].id, last_solution);
      new_functions++;
    }
    else
      parser.parse();
  }
  catch (const parsert::smt2_errort &e)
  {
    message.debug() << "Error parsing LLM response: " << e.get_line_no() << ": "
                    << e.what() << messaget::eom;
  }
  // add the new functions to the problem
  for (auto &id : parser.id_map)
  {
    if (id.second.definition.is_not_nil()) 
    //&&   problem.defined_functions.find(symbol_exprt(id.first, id.second.type)) == problem.defined_functions.end())
    {
      problem.defined_functions[symbol_exprt(id.first, id.second.type)] = id.second.definition;

      if (id.second.definition.id() == ID_lambda)
      {
        auto lambda = to_lambda_expr(id.second.definition);
        // insert into grammar (as a complete terminal expression)
        if (add_to_grammar(id.first, lambda.where()))
        {
          new_functions++;
        }
      }
    }
  }

  if (update_grammar)
  {
    // update grammar weights
    message.debug() << "Updating grammar weights with new functions" << messaget::eom;
    for (const auto &rules : problem.get_grammar().production_rules)
    {
      message.debug() << id2string(rules.first) << " : " << messaget::eom;
      auto &weights = problem.get_grammar().production_rule_weights[rules.first];
      for (unsigned i = 0; i < rules.second.size(); i++)
      {
        message.debug() << " " << expr2sygus(rules.second[i]) << ":";
        if (parser.operator_counts.find(rules.second[i].id()) != parser.operator_counts.end())
        {
          weights[i] += parser.operator_counts[rules.second[i].id()];
        }
        message.debug() << weights[i] << "\n";
      }
      message.debug() << messaget::eom;
    }
  }

  return new_functions;
}