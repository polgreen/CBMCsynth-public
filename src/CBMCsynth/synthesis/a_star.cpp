#include "a_star.h"
#include "../utils/util.h"
#include <util/std_expr.h>
#include <util/replace_expr.h>
#include <queue>


bool is_nonterminal(const exprt &expr, const syntactic_templatet &grammar)
{
  if(expr.id()==ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if(grammar.production_rules.find(symbol.get_identifier())!=grammar.production_rules.end())
      return true;
  }
  return false;
}


void a_star_syntht::calculate_h_scores()
{
  bool change = false;
  while (!change)
  {
    message.debug()<<"Calculating h scores\n"<<messaget::eom;
    for (std::size_t i = 0; i < grammar.nt_ids.size(); i++)
    {
      // calculate h for each rule
      auto &rules = grammar.production_rules[grammar.nt_ids[i]];
      auto &weights = probabilities[grammar.nt_ids[i]];
      double max_score = 0;
      for (std::size_t j = 0; j < rules.size(); j++)
      {
        // calculate h for each nonterminal in the rule
        std::vector<exprt> nonterminals;
        std::vector<double> possible_scores;
        double rule_score = weights[j];
        if (rules[j].id() == ID_symbol)
        {
          // rule is nonterminal -> nonterminal
          if (is_nonterminal(rules[j], grammar))
            rule_score *= h_scores[to_symbol_expr(rules[j]).get_identifier()];
        }
        else
        {
          std::vector<symbol_exprt> nts = list_nonterminals(rules[j], grammar);
          for (const auto &nt : nts)
            rule_score *= h_scores[nt.get_identifier()];
        }
        if (rule_score > max_score)
          max_score = rule_score;
        // score is max of possible scores
      }
      if (max_score != h_scores[grammar.nt_ids[i]])
      {
        change = true;
        h_scores[grammar.nt_ids[i]] = max_score;
      }
    }
  }
}

double a_star_syntht::g(const exprt &partial_func)
{
  std::vector<symbol_exprt> nts = list_nonterminals(partial_func, grammar);
  if(nts.size()==0)
    return 0;
  double score = 0;
  for(const auto &nt: nts)
  {
    score += std::log2(h_scores[nt.get_identifier()]);
  }
  return -score;  
}

void a_star_syntht::set_up_probabilities()
{
  // if grammar has no weights add them
  if(grammar.production_rule_weights.size()==0)
  {
    message.debug()<<"adding default weights"<< messaget::eom;
    add_grammar_weights(grammar);
  }

  for (auto &nt : grammar.production_rule_weights)
  {
    double total_weight = 0;
    for(auto &w: nt.second)
      total_weight += w;
    
    std::vector<double> weights;
    for(auto &w: nt.second)
      weights.push_back(w/total_weight);
    probabilities[nt.first] = weights;
  }
}

a_star_syntht::resultt a_star_syntht::operator()(std::size_t iteration)
{
  // calculate h scores
  set_up_probabilities();
  calculate_h_scores();
  message.status()<<"starting enumeration"<<messaget::eom;
  // tuple of cf+cg, cg, expr
  std::priority_queue<std::tuple<double, double, exprt>> Q;
  std::tuple<double, double, exprt> start_node = std::make_tuple(h_scores[grammar.start], h_scores[grammar.start], symbol_exprt(grammar.start, grammar.start_type));
  Q.push(start_node);
  while(!Q.empty())
  {
    const auto &node = Q.top();
    const double cf = std::get<0>(node) - std::get<1>(node);
    const auto &cg = std::get<1>(node);
    const auto &partial_func = std::get<2>(node);
    if(!contains_nonterminal(partial_func, grammar))
    {
      last_solution.functions[symbol_exprt(problem.synthesis_functions[0].id, 
          problem.synthesis_functions[0].type)] = 
          lambda_exprt(problem.synthesis_functions[0].parameters, partial_func);
      if(cex_verifier(problem, last_solution, counterexamples) == counterexample_verifyt::resultt::PASS)//verify against cex
        return CANDIDATE;
    }
    for(const auto &nt : list_nonterminals(partial_func, grammar))
    {
      exprt copy = partial_func;
      const auto & rules = grammar.production_rules[nt.get_identifier()];
      const auto & weights = grammar.production_rule_weights[nt.get_identifier()];
      for(unsigned i=0; i<rules.size(); i++)
      {
        replace_expr(nt, rules[i], copy);
        basic_simplify(copy);
        // don't bother adding if its already in the queue
        // NB: this might mean we don't add a higher score for the same program
        if(queue_of_progs.find(copy)!=queue_of_progs.end())
          continue;
        double score = cf + weights[i];
        double new_cg = g(copy);
        Q.push(std::make_tuple(score+new_cg, new_cg, copy));
      }
    }
  } 
  return NO_SOLUTION;
}


void a_star_syntht::set_program_size(std::size_t size)
{
  program_size = size;
}

void a_star_syntht::add_counterexample(const counterexamplet &cex)
{
  counterexamples.push_back(cex);
}

a_star_syntht::resultt a_star_syntht::operator()(std::size_t max_depth)
{

}