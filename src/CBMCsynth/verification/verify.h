#ifndef VERIFY_H
#define VERIFY_H

#include "../sygus_problem.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>

// Class for verifying a solution to a sygus problem.
class verifyt
{
 public:
  using resultt = enum { PASS, FAIL };
  verifyt(namespacet &_namespace, 
                message_handlert &_ms) :
                ns(_namespace),
                log(_ms){};

  resultt operator()( sygus_problemt &problem,
    const solutiont &solution);


  resultt operator()(sygus_problemt &problem,
    const solutiont &solution,
    decision_proceduret &solver);
  
  counterexamplet get_counterexample();

  protected:
  counterexamplet get_counterexample(
  const decision_proceduret &solver, const sygus_problemt &problem) const;
  namespacet ns;
  messaget log;
  counterexamplet counterexample;
  void add_problem(const sygus_problemt &problem, const solutiont &solution, decision_proceduret &solver );

};

#endif /* VERIFY_H */