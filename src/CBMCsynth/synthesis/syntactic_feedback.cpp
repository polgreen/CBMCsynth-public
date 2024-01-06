
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
//#define DEBUG

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
  for(const auto &c: problem.constraints)
  {
    prompt += "assert(" + expr2python(c) + ")\n";
  }

  prompt+= "\nSo far, the student has written this code:\n";
  prompt+= synth_fun_to_python(problem.synthesis_functions[0], partial_function, 
  problem.synthesis_functions[0].grammar.nt_ids);
  prompt+= "\n\n";
  prompt+="Can you suggest some helper functions for the student to use to complete this code?\n";
  prompt+="Print only the code and nothing else\n.";
  return prompt;
}

std::string syntactic_feedbackt::build_smt_prompt(const exprt &partial_function)
{
  std::string prompt = "You are teaching a student to write SMT-LIB. The student must write a function that satisfies the following constraints:\n";
  for(const auto &c: problem.constraints)
  {
    prompt += "(constraint (" + expr2sygus(c) + ")\n";
  }

  prompt+= "\nSo far, the student has written this code:\n";

  prompt+=
  fun_def(symbol_exprt(problem.synthesis_functions[0].id, problem.synthesis_functions[0].type),
  lambda_exprt(problem.synthesis_functions[0].parameters, partial_function));

  prompt+= "\n\n";

// add section saying what the student previously did, and what it failed on
  //prompt += "The student has tried the following functions, but they did not satisfy the constraints:\n";


  prompt+="Can you suggest some helper functions for the student to use to complete this code and replace the ??\n";
  prompt+="Print only the code and nothing else\n.";
  return prompt;
}


bool syntactic_feedbackt::augment_grammar(const exprt &partial_function, 
  sygus_problemt &problem)
{
  // debugs without calling openai
#ifdef DEBUG  
  std::string response = "(define-fun max ((x Int) (y Int)) Int (ite (> x y) x y))";
#else
  // generate openAI query
    openai::start(); // Will use the api key provided by `OPENAI_API_KEY` environment variable
    std::string prompt = build_smt_prompt(partial_function);

    openai::Json messages;
    messages["role"] = "user";
    messages["content"] = prompt;
    openai::Json j;
    j["model"] = "gpt-3.5-turbo";
    j["messages"] = openai::Json::array({messages});
    j["temperature"] = 0;

    auto completion = openai::chat().create(j);
    // std::cout << "completion text " << completion["choices"][0]["message"]["content"] << std::endl;

    // The following ugly code converts the json into a string, removes
    // extra stuff from the string and then pipes the string to an istream so the
    // parser can parse it. 
    std::ostringstream oss;
    oss << completion["choices"][0]["message"]["content"] ;

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
#endif
    std::cout<<response<<std::endl;
    std::istringstream str(response);
    parsert parser(str);

    try
    {
      parser.parse();
    }
    catch (const parsert::smt2_errort &e)
    {
      std::cerr << e.get_line_no() << ": "
               << e.what() << std::endl;
    }

 bool change=false;
  // add the new functions to the problem
  for(auto &id: parser.id_map)
  {
    if(id.second.definition.is_not_nil())
    {
      std::cout<<"adding function "<<id.first<<" with definition " << id.second.definition.pretty()<<std::endl;
      problem.defined_functions[symbol_exprt(id.first, id.second.type)] = id.second.definition;
 
      if(id.second.definition.id() == ID_lambda)
      {
        auto lambda = to_lambda_expr(id.second.definition);
        // insert into grammar (as a complete terminal expression)
        auto codomain = lambda.type().codomain();
        for(const auto &rules: problem.get_grammar().production_rules)
        {
          if(rules.second[0].type() == codomain)
          {
            std::cout<<"adding to rule for "<<id2string(rules.first)<<std::endl;
            problem.get_grammar().production_rules[rules.first].push_back(lambda.where());
            auto max = std::max_element(
              problem.get_grammar().production_rule_weights[rules.first].begin(),
            problem.get_grammar().production_rule_weights[rules.first].end());
            problem.get_grammar().production_rule_weights[rules.first].
            push_back(*max * 10);
            change=true;
          }
          }
      }
    }
  }
  return change;
}