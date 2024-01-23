
#include "syntactic_feedback.h"
#include "../utils/expr2python.h"
#include "../utils/expr2sygus.h"
#include "../utils/util.h"
#include "../openai/openai.hpp"

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

// iter = 0: don't expand functions,don't give defs, no cex
//  iter = 1: expanded functions, no cex
//  iter = 2: expanded functions, cex
// iter = 3: don't expand functions, do give defs, no cex
// iter = 4: don't expand functions, don't give defs, cex
// iter = 5: don't expand functions, do give defs, cex

std::string syntactic_feedbackt::build_smt_prompt(const exprt &partial_function)
{
  message.debug() << "iter is " << iter << messaget::eom;
  std::string prompt = "You are teaching a student to write SMT-LIB. ";
  if (iter == 1 || iter == 2)
  {
    prompt += "The student must write a function that satisfies the following constraints:\n";
    for (const auto &c : problem.constraints)
    {
      exprt expanded_c = c;
      expand_function_applications(expanded_c, problem.defined_functions);
      prompt += "(constraint (" + expr2sygus(expanded_c) + ")\n";
    }
  }
  else
  {
    if (iter == 3 || iter == 5)
    {
      prompt += "The student may find the following functions useful:\n";
      std::set<symbol_exprt> defined_functions;
      for (const auto &c : problem.constraints)
        get_defined_functions(c, problem.defined_functions, defined_functions);

      for (const auto &f : defined_functions)
        prompt += fun_def(f, problem.defined_functions[f]) + "\n";
    }

    prompt += "The student must write a function that satisfies the following constraints:\n";
    for (const auto &c : problem.constraints)
      prompt += "(constraint (" + expr2sygus(c) + ")\n";
  }

  if ((iter == 2 || iter == 4 || iter == 5) && last_cex.assignment.size() > 0)
  {
    prompt += "\nThe last solution the student tried was this, but the teacher marked this solution incorrect:\n";

    prompt +=
        fun_def(symbol_exprt(problem.synthesis_functions[0].id, problem.synthesis_functions[0].type),
                last_solution);
    prompt += "\nThis solution was incorrect because it did not work for the following inputs:\n";
    for (const auto &c : last_cex.assignment)
    {
      prompt += expr2sygus(c.first) + "  =  " + expr2sygus(c.second) + "\n";
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
  iter++;
  if (iter == 6)
    iter = 0;
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
      // auto max = std::max_element(
      //     problem.get_grammar().production_rule_weights[rules.first].begin(),
      //     problem.get_grammar().production_rule_weights[rules.first].end());
      double avg = 0;
      for (const auto &w : problem.get_grammar().production_rule_weights[rules.first])
        avg += w;
      avg = avg / problem.get_grammar().production_rule_weights[rules.first].size();
      // TODO: HEURISTIC this is a heuristic to decide how likely it is that we pick the new production rules
      if (id == problem.synthesis_functions[0].id)
      {
        last_solution = expr;
        problem.get_grammar().production_rule_weights[rules.first].push_back(avg);
        problem.get_grammar().bonus_weights[rules.first].push_back(avg - 1);
      }
      else
      {
        problem.get_grammar().production_rule_weights[rules.first].push_back(avg);
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
  std::string response = " (define-fun in-range ((v Int) (lower Int) (upper Int)) Bool  (and (> v lower) (< v upper)))(define-fun inv-f ((x Int) (z Int)) Bool  (=> (and (in-range x (- 0 100) 200) (in-range z 100 200))      (and (in-range x (- 0 100) 200) (in-range z 100 200))))(define-fun inv-f-extended ((x Int) (z Int) (x! Int) (z! Int)) Bool  (=> (and (in-range x (- 0 100) 200) (in-range z 100 200))      (or (and (in-range x (- 0 100) 100) (> z 100))          (or (>= x 100) (<= z 100)))))The completed code would look like this:(define-fun in-range ((v Int) (lower Int) (upper Int)) Bool  (and (> v lower) (< v upper)))(define-fun inv-f ((x Int) (z Int)) Bool  (=> (and (in-range x (- 0 100) 200) (in-range z 100 200))      (and (in-range x (- 0 100) 200) (in-range z 100 200))))(define-fun inv-f-extended ((x Int) (z Int) (x! Int) (z! Int)) Bool  (=> (and (in-range x (- 0 100) 200) (in-range z 100 200))      (or (and (in-range x (- 0 100) 100) (> z 100))          (or (>= x 100) (<= z 100)))))";
  //   "(define-fun bvashr4 ((x (_ BitVec 4)) (y (_ BitVec 4))) (_ BitVec 4) (bvashr x y))2. Helper function for bvuge:   (define-fun bvuge4 ((x (_ BitVec 4)) (y (_ BitVec 4))) Bool     (bvuge x y))Using these helper functions, the student can replace the ?? in their code with the following:(define-fun fn0 ((vr0 (_ BitVec 4))(vr1 (_ BitVec 4))) Bool (not (or (bvuge4 (bvashr4 (_ bv0 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv1 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv2 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv3 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv4 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv5 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv6 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv7 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv8 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv9 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv10 4) vr0) vr1)          (bvuge4 (bvashr4 (_ bv11 4) vr0) vr1)         ";
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
  std::size_t new_functions = 0;
  parsert parser(str);
  try
  {
    parser.parse_generously = true;
    parser.add_defined_functions(problem.defined_functions);
    parser.add_symbols(problem.free_var);

    last_solution = nil_exprt();

    if (response.find("define-fun") == std::string::npos)
    {
      last_solution = parser.parse_expression();
      add_to_grammar(problem.synthesis_functions[0].id, last_solution);
      new_functions++;
    }
    else
      parser.parse();

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
      update_grammar_weights(parser);
    }
  }
  catch (const parsert::smt2_errort &e)
  {
    message.debug() << "Error parsing LLM response: " << e.get_line_no() << ": "
                    << e.what() << messaget::eom;
    update_grammar_weights(parser);
  }

  return new_functions;
}

void syntactic_feedbackt::update_grammar_weights(parsert & parser)
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