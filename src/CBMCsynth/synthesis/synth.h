#ifndef SYNTH_H_
#define SYNTH_H_

#include "../sygus_problem.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>
#include <random>


// base class for synthesis phase of CEGIS.
//  any synthesiser must implement these functions
class syntht
{
public:
  using resultt = enum { CANDIDATE,
                         NO_SOLUTION };

  virtual solutiont get_solution() const = 0;
  virtual void set_program_size(std::size_t) = 0;
  virtual resultt operator()() = 0;
  virtual void add_counterexample(const counterexamplet &cex) = 0;
  
};

#endif /* SYNTH_H_ */
