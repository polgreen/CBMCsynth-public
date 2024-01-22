#include "a_star.h"
#include "synth.h"
#include "../utils/util.h"
#include "../utils/expr2sygus.h"
#include <util/std_expr.h>
#include <util/replace_expr.h>
#include <queue>
#include <iostream>


bool is_nonterminal(const exprt &expr, const syntactic_templatet &grammar)
{
  if (expr.id() == ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if (grammar.production_rules.find(symbol.get_identifier()) != grammar.production_rules.end())
      return true;
  }
  return false;
}

void a_star_syntht::calculate_h_scores()
{
  while (1)
  {
    bool change = false;
    message.debug() << "Calculating h scores\n"
                    << messaget::eom;
    for (std::size_t i = 0; i < grammar.nt_ids.size(); i++)
    {
      // calculate h for each rule
      auto &rules = grammar.production_rules[grammar.nt_ids[i]];
      auto &probs = probabilities[grammar.nt_ids[i]];
      double max_score = 0;
      for (std::size_t j = 0; j < rules.size(); j++)
      {
        // calculate h for each nonterminal in the rule
        std::vector<exprt> nonterminals;
        std::vector<double> possible_scores;
        double rule_score = probs[j];
        std::vector<symbol_exprt> nts = list_nonterminals(rules[j], grammar);
        for (const auto &nt : nts)
          rule_score *= h_scores[nt.get_identifier()];

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
    if (change == false)
      break;
  }
  message.debug() << "h scores: " << messaget::eom;
  for (const auto &nt : grammar.nt_ids)
    message.debug() << nt << " : " << h_scores[nt] << messaget::eom;
}

double a_star_syntht::g(const exprt &partial_func)
{
  std::vector<symbol_exprt> nts = list_nonterminals(partial_func, grammar);
  if (nts.size() == 0)
    return 0;
  double score = 0;
  for (const auto &nt : nts)
  {
    double hscore = h_scores[nt.get_identifier()];
    if (hscore != 0)
    {
      score += std::log2(hscore);
    }
  }
  return -score;
}

void a_star_syntht::set_up_probabilities()
{
  // if grammar has no weights add them
  if (grammar.production_rule_weights.size() == 0)
  {
    add_grammar_weights(grammar);
  }

  for (auto &nt : grammar.production_rule_weights)
  {
    double total_weight = 0;
    for (auto &w : nt.second)
      total_weight += w;

    std::vector<double> wts;
    std::vector<double> probs;
    // w = -log2(p), or infinity if p=0
    for (auto &w : nt.second)
    {
      probs.push_back(w / total_weight);
      if (w == 0)
        wts.push_back(std::numeric_limits<double>::infinity());
      else
        wts.push_back(-std::log2(w / total_weight));
    }
    probabilities[nt.first] = probs;
    weights[nt.first] = wts;
  }
}

void print_queue(std::priority_queue<q_entry> Q)
{
  std::cout << "Queue: (size " << Q.size() << ")\n";
  while (!Q.empty())
  {
    std::cout << expr2sygus(Q.top().expr) << " : " << Q.top().cf << " : " << Q.top().cg << std::endl;
    Q.pop();
  }
}

bool a_star_syntht::get_LLM_feedback(const exprt &expr)
{
  LLM_calls++;
  std::size_t new_funcs = feedback.augment_grammar(expr, problem);
  message.debug()<<"LLM added "<<new_funcs<<" new functions\n";
  if (feedback.last_solution.id() != ID_nil)
  {
    message.debug() << "LLM gave us a candidate solution: " << format(feedback.last_solution) << messaget::eom;
    last_solution.functions[symbol_exprt(problem.synthesis_functions[0].id,
                                         problem.synthesis_functions[0].type)] =
        lambda_exprt(problem.synthesis_functions[0].parameters, feedback.last_solution);

    try{
      if(cex_verifier(problem, last_solution, counterexamples) == counterexample_verifyt::resultt::PASS)\
        return true;
    }
    catch(const std::exception &e)
    {
      message.debug()<<"Exception in cex verifier: "<<e.what()<<messaget::eom;
    }
  }
  // update weights and h scores
  set_up_probabilities();
  calculate_h_scores();
  return false;
}

a_star_syntht::resultt a_star_syntht::operator()(std::size_t iteration)
{
  // calculate h scores
  set_up_probabilities();
  calculate_h_scores();
  std::size_t iter = 0;
  message.status() << "starting enumeration" << messaget::eom;
  // tuple of cf, cg, expr
  if(Q.empty())
    Q.push(q_entry(0.0, h_scores[grammar.start], symbol_exprt(grammar.start, grammar.start_type)));
  bool call_LLM=false;
  while (!Q.empty())
  {
    message.debug()<<"iteration "<<iter<<", queue size "<<Q.size()<<messaget::eom;
    if((iter==0 || Q.size()%50==0) && use_syntactic_feedback && LLM_calls < maxLLM_calls)
    {
      message.debug()<<"calling LLM\n";
      call_LLM=true;
    }
    if(call_LLM && count_nonterminals(Q.top().expr, grammar)!=0)
    {
      call_LLM=false;
      if(get_LLM_feedback(Q.top().expr))
        return CANDIDATE;
    }
    iter++;

    const double cf = Q.top().cf;
    const exprt partial_func = Q.top().expr;
    message.debug() << "top of queue " << expr2sygus(partial_func) << messaget::eom;
    if (!contains_nonterminal(partial_func, grammar))
    {
      message.debug() << "possible candidate" << messaget::eom;
      last_solution.functions[symbol_exprt(problem.synthesis_functions[0].id,
                                           problem.synthesis_functions[0].type)] =
          lambda_exprt(problem.synthesis_functions[0].parameters, partial_func);
      if(cex_verifier(problem, last_solution, counterexamples) == counterexample_verifyt::resultt::PASS) // verify against cex
      {
        message.debug() << "counterexample verifier passed " << messaget::eom;
        return CANDIDATE;
      }
      else
        message.debug() << "failed cex verification" << messaget::eom;
    }
    Q.pop();
    const auto &nt_counts = get_nonterminal_counts(partial_func, grammar);
    for (const auto &nt : grammar.nt_ids)
    {
      if (nt_counts.find(nt) == nt_counts.end())
        continue;
      const auto &rules = grammar.production_rules[nt];
      const auto &w = weights[nt];
      const auto &nt_symbol = symbol_exprt(nt, grammar.production_rules[nt][0].type());
      for (unsigned i = 0; i < rules.size(); i++)
      {
    //    for (std::size_t j = 1; j <= nt_counts.at(nt); j++)
        {
          exprt copy = partial_func;
          replace_nth_occurrence(nt_symbol, rules[i], copy, 1);
          double new_cf = cf + w[i];
          double new_cg = g(copy);

          if (queue_of_progs.find(copy) != queue_of_progs.end())
          {
            if(queue_of_progs[copy] > new_cf+new_cg)
              {
                queue_of_progs[copy] = new_cf+new_cg;
                message.debug() << "updating " << expr2sygus(copy) << " in queue with score " << new_cf << ", " << new_cg << messaget::eom;
                Q.push(q_entry(new_cf, new_cg, copy));
              }
          }
          else
          {
            queue_of_progs[copy] = new_cf+new_cg;
            message.debug() << "adding " << expr2sygus(copy) << " to queue with score " << new_cf << ", " << new_cg << messaget::eom;
            Q.push(q_entry(new_cf, new_cg, copy));
          }
        }
      }
    }
  }
  std::cout<<"queue empty\n";
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

solutiont a_star_syntht::get_solution() const
{
  return last_solution;
}
