#ifndef VERIFY_H
#define VERIFY_H

#include "verify_encoding.h"
#include "../sygus_problem.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>


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
  namespacet ns;
  messaget log;
  /// Encoding for the verification decision procedure call.
  verify_encodingt verify_encoding;
  counterexamplet counterexample;

  void replace_synth_fun_parameters(const sygus_problemt &problem, std::map <symbol_exprt, exprt> &solution_functions); 
  void add_problem(const sygus_problemt &problem, const solutiont &solution, decision_proceduret &solver );

};

#endif /* VERIFY_H */
