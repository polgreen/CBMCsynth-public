#ifndef SYNTH_H_
#define SYNTH_H_

#include "../sygus_problem.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>

class syntht
{

public:
  using resultt = enum { CANDIDATE, NO_SOLUTION };
  syntht(message_handlert &_ms, sygus_problemt &_problem): 
    message_handler(_ms),
    problem(_problem){};

  resultt operator()();
  exprt model(exprt) const;

  solutiont get_solution() const;
  void set_program_size(std::size_t size);
  
protected:
  std::vector<counterexamplet> counterexamples;
  solutiont last_solution;
  message_handlert &message_handler;
  sygus_problemt &problem;
  // set of previous solutions
};

#endif /* SYNTH_H_ */
