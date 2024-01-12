#ifndef PROB_BU_SYNTH_H_
#define PROB_BU_SYNTH_H_

#include "synth_bu.h"

// a top down enumerator that randomly enumerates a grammar
// NB: this only handles sygus_problems with a single synthesis function
class prob_bu_syntht : public bottom_up_syntht
{
public:
  prob_bu_syntht(message_handlert &_ms, sygus_problemt &_problem, counterexample_verifyt &_cex_verifier) : 
    bottom_up_syntht(_ms, _problem, _cex_verifier){};


  // calls the enumerator and then checks the result against the counterexamples
  // returns when it has found a candidate that satisfies the counterexamples
  resultt operator()(std::size_t iteration) override;

protected:
  std::map<exprt,double> program_score;

  
  std::multimap<double, exprt, std::greater<>> sorted_solutions_to_check;

  std::map<irep_idt, std::vector<double>> normalised_rule_weights;
  void setup_rule_weights();
  void get_next_programs();
  void initialise_program_pool();
  double calculate_program_score(const double &rule_score, const std::vector<exprt> &operands, const exprt &new_expr);


};

#endif /* PROB_BU_SYNTH_H_ */
