#ifndef MINI_VERIFY_H
#define MINI_VERIFY_H

#include "../sygus_problem.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>

// Class for verifying whether a candidate solution works for a set of counterexamples
class mini_verifyt
{
 public:
  using resultt = enum { PASS, FAIL };
  mini_verifyt(namespacet &_namespace, 
                message_handlert &_ms) :
                ns(_namespace),
                log(_ms){};

  resultt operator()( sygus_problemt &problem,
    const solutiont &solution, const std::vector<counterexamplet> &cex);
  
  protected:
  namespacet ns;
  messaget log;
  void add_problem(const sygus_problemt &problem, const solutiont &solution, const counterexamplet &cex, decision_proceduret &solver );

};

#endif /* MINI_VERIFY_H */
