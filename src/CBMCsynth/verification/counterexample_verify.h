#ifndef MINI_VERIFY_H
#define MINI_VERIFY_H

#include "../sygus_problem.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>

// Class for verifying whether a candidate solution works for a set of counterexamples
class counterexample_verifyt
{
 public:
  using resultt = enum { PASS, FAIL };
  counterexample_verifyt(namespacet &_namespace, 
                message_handlert &_ms) :
                ns(_namespace),
                message(_ms){};

  resultt operator()( sygus_problemt &problem,
    const solutiont &solution, const std::vector<counterexamplet> &cex);
  
  counterexamplet get_failed_cex();

  
  protected:
  namespacet ns;
  messaget message;
  counterexamplet failed_cex;
  
  void add_problem(const sygus_problemt &problem, const solutiont &solution, const counterexamplet &cex, decision_proceduret &solver );

};

#endif /* MINI_VERIFY_H */
