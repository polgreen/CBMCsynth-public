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

class a_star_syntht : public syntht
{
  public:
    a_star_syntht(message_handlert &_ms, sygus_problemt &_problem, counterexample_verifyt &_cex_verifier) : message(_ms),
                                                                                                           problem(_problem),
                                                                                                           cex_verifier(_cex_verifier),
                                                                                                           grammar(_problem.get_grammar()),
                                                                                                           feedback(_problem, _ms),
    {
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
  std::map<irep_idt, double> h_scores;
  // last solution we found
  solutiont last_solution;
  // depth we enumerate to
  std::size_t program_size; // HEURISTIC
  void calculate_h_scores();
  void set_up_probabilities();

  std::unordered_set<exprt> queue_of_progs;
  double g(const exprt &partial_func);
  std::map<irep_idt, std::vector<double>> probabilities;
};




#endif /* A_STAR_H_ */