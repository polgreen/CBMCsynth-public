#ifndef A_STAR_H_
#define A_STAR_H_

#include "synth.h"
#include "../verification/counterexample_verify.h"
#include "../sygus_problem.h"
#include "syntactic_feedback.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>
#include <set>
#include <unordered_set>
#include <queue>

struct q_entry
{
  double cf;
  double cg;
  exprt expr;
  bool operator<(const q_entry &o) const
  {
    return (cf + cg) > (o.cf + o.cg);
  }
  q_entry(double cf, double cg, exprt expr) : cf(cf), cg(cg), expr(expr) {}
};

class a_star_syntht : public syntht
{
public:
  a_star_syntht(message_handlert &_ms, sygus_problemt &_problem, counterexample_verifyt &_cex_verifier) : message(_ms),
                                                                                                          problem(_problem),
                                                                                                          cex_verifier(_cex_verifier),
                                                                                                          grammar(_problem.get_grammar()),
                                                                                                          feedback(_problem, _ms),
                                                                                                          LLM_calls(0),
                                                                                                          maxLLM_calls(10){
                                                                                                              // create probabilities
                                                                                                          };

  solutiont get_solution() const override;

  // set maximum depth of enumeration
  void set_program_size(std::size_t size) override;

  // calls the enumerator and then checks the result against the counterexamples
  // returns when it has found a candidate that satisfies the counterexamples
  resultt operator()(std::size_t) override;

  // adds a counterexample to the list
  void add_counterexample(const counterexamplet &cex) override;
  bool use_syntactic_feedback;
  void set_feedback_parameters(bool _use_syntactic_feedback,
                               bool _update_grammar,
                               bool _use_cex_in_prompt,
                               bool _expand_fun_apps = true)
  {
    use_syntactic_feedback = _use_syntactic_feedback;
    feedback.update_grammar = _update_grammar;
    feedback.use_cex_in_prompt = _use_cex_in_prompt;
    feedback.expand_fun_apps = _expand_fun_apps;
  }

  void set_temperature(double _temperature)
  {
    feedback.temperature = _temperature;
  }

protected:
  // used for printing. TODO: make all the printing use the message handlers correctly
  messaget message;
  // the problem to solve
  sygus_problemt &problem;
  // verifies candidates against the counterexamples
  counterexample_verifyt &cex_verifier;
  // the grammar to enumerate
  // we make a copy of this rather than using the one in the SyGuS problem
  // so we can change it if we need
  // TODO: make this a reference to the grammar in the sygus problem
  syntactic_templatet &grammar;
  syntactic_feedbackt feedback;
  std::size_t LLM_calls;
  std::size_t maxLLM_calls;
  // counterexamples
  std::vector<counterexamplet> counterexamples;
  std::map<irep_idt, double> h_scores;
  // last solution we found
  solutiont last_solution;
  // depth we enumerate to
  std::size_t program_size; // HEURISTIC
  void calculate_h_scores();
  void set_up_probabilities();
  void replace_nonterminal(const exprt &expr, const double &cf);
  bool get_LLM_feedback(const exprt &expr);
  std::priority_queue<q_entry> Q;

  std::unordered_map<exprt, double, irep_hash> queue_of_progs;
  double g(const exprt &partial_func);
  std::map<irep_idt, std::vector<double>> probabilities;
  std::map<irep_idt, std::vector<double>> weights;
};

#endif /* A_STAR_H_ */