#ifndef TD_SYNTH_H_
#define TD_SYNTH_H_

#include "synth.h"
#include "../verification/mini_verify.h"
#include "../sygus_problem.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>
#include <random>

// a top down enumerator that randomly enumerates a grammar
// NB: this only handles sygus_problems with a single synthesis function
class top_down_syntht : public syntht
{
public:
  top_down_syntht(message_handlert &_ms, sygus_problemt &_problem, mini_verifyt &_cex_verifier) : 
  message_handler(_ms),
                                                            problem(_problem),
                                                            cex_verifier(_cex_verifier),
                                                            grammar(_problem.get_grammar()){
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
  resultt operator()() override;

  // adds a counterexample to the list
  void add_counterexample(const counterexamplet &cex) override;

protected:
  // used for printing. TODO: make all the printing use the message handlers correctly
  message_handlert &message_handler;
  // the problem to solve
  sygus_problemt &problem;
  // verifies candidates against the counterexamples
  mini_verifyt &cex_verifier;
  // the grammar to enumerate
  // we make a copy of this rather than using the one in the SyGuS problem
  // so we can change it if we need
  // TODO: make this a reference to the grammar in the sygus problem
  syntactic_templatet grammar;
  // counterexamples
  std::vector<counterexamplet> counterexamples;
  // last solution we found
  solutiont last_solution;
 
  // depth we enumerate to
  std::size_t program_size;
  // checks if an exprt contains a nonterminal
  bool contains_nonterminal(const exprt &expr);

  // randomly chooses a production rule to apply whenever it finds
  // a non-terminal in the exprt. If it hits maximum depth, it replaces the non-terminal
  // with the first terminal in the production rules that it finds.
  // it returns when it has found a complete program.
  bool replace_nts(exprt &expr, std::size_t &current_depth);

  // creates the distributions based on non-terminal weights
  void create_distributions();

  // the distributions for each non-terminal
  // irep_idt is the nonterminal name, the distribution has a weight for each
  // production rule for that nonterminal.
  std::map<irep_idt, std::discrete_distribution<int>> distributions;

};

#endif /* TD_SYNTH_H_ */
