#ifndef CVC5_SYNTH_H
#define CVC5_SYNTH_H

#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>
#include "sygus_problem.h"

class cvc5_syntht
{
public:
  cvc5_syntht(message_handlert &_ms) : message_handler(_ms){};

  // build a candidate solution, in the form of a model
  // that can be queried using model()

  decision_proceduret::resultt operator()(const sygus_problemt &);

  // get full solution map
  std::map<symbol_exprt, exprt> get_solution() const;
  // get model for specific expression
  exprt model(exprt expr) const;

protected:
  decision_proceduret::resultt read_result(std::istream &in, const sygus_problemt &p);
  decision_proceduret::resultt solve(const sygus_problemt &problem);
  std::map<symbol_exprt, exprt> last_solution;
  message_handlert &message_handler;
};


#endif // CVC5_SYNTH_H