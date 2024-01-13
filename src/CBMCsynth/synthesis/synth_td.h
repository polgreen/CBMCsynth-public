#ifndef TD_SYNTH_H_
#define TD_SYNTH_H_

#include "synth.h"
#include "../verification/counterexample_verify.h"
#include "../sygus_problem.h"
#include "syntactic_feedback.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>
#include <random>
#include <set>

// a top down enumerator that randomly enumerates a grammar
// NB: this only handles sygus_problems with a single synthesis function
class top_down_syntht : public syntht
{
public:
  top_down_syntht(message_handlert &_ms, sygus_problemt &_problem, counterexample_verifyt &_cex_verifier) : 
  message(_ms),
                                                            problem(_problem),
                                                            cex_verifier(_cex_verifier),
                                                            grammar(_problem.get_grammar()),
                                                            feedback(_problem, _ms),
                                                            enumerations_since_LLM(0),
                                                            num_LLM_calls(0)
                                                            {
                                                              create_distributions();
                                                            };

  solutiont get_solution() const override;

  // initialises the top down enumeration wtih the start symbol of the grammar
  // then calls replace_nts to enumerate through the grammar
  void top_down_enumerate();

  // set maximum depth of enumeration
  void set_program_size(std::size_t size) override;

  // calls the enumerator and then checks the result against the counterexamples
  // returns when it has found a candidate that satisfies the counterexamples
  resultt operator()(std::size_t) override;

  // adds a counterexample to the list
  void add_counterexample(const counterexamplet &cex) override;

  using enum_resultt = enum { CHANGED, NO_CHANGE, ABORT};

  bool use_syntactic_feedback;
  bool use_bonus_weights;

  void set_feedback_parameters(bool _use_syntactic_feedback, 
    bool _update_grammar, 
    bool _use_bonus_weights, 
    bool _use_cex_in_prompt,
    bool _expand_fun_apps=true)
  {
    use_syntactic_feedback = _use_syntactic_feedback;
    feedback.update_grammar = _update_grammar;
    use_bonus_weights = _use_bonus_weights;
    feedback.use_cex_in_prompt = _use_cex_in_prompt;
    feedback.expand_fun_apps = _expand_fun_apps;
  }
  std::size_t frequency_of_LLM_calls = 10; // HEURISTIC

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
  // counterexamples
  std::vector<counterexamplet> counterexamples;
  // index of failed cex
  std::size_t failed_cex;
  // last solution we found
  solutiont last_solution;
  std::set<std::vector<unsigned>> prev_solutions;
 
  // depth we enumerate to
  std::size_t program_size; // HEURISTIC
  // iteration we are on
  std::size_t iter;

  // number of enumerations since the last LLM call
  std::size_t enumerations_since_LLM; // HEURISTIC
  // minimum number of enumerations between LLM calls

  std::size_t max_LLM_calls=10; // HEURISTIC
  std::size_t num_LLM_calls; // HEURISTIC
  bool called_LLM_this_iter; // says if we have called the LLM this iteration
  // randomly chooses a production rule to apply whenever it finds
  // a non-terminal in the exprt. If it hits maximum depth, it replaces the non-terminal
  // with the first terminal in the production rules that it finds.
  // it returns when it has found a complete program.
  enum_resultt replace_nts(exprt &expr, std::size_t &current_depth, std::vector<unsigned> &sequence);

  // creates the distributions based on non-terminal weights
  void create_distributions();

  // the distributions for each non-terminal
  // irep_idt is the nonterminal name, the distribution has a weight for each
  // production rule for that nonterminal.
  std::map<irep_idt, std::discrete_distribution<int>> distributions;

};

#endif /* TD_SYNTH_H_ */
