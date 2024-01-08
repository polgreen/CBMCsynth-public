#ifndef CEGIS_H_
#define CEGIS_H_

#include "synthesis/synth.h"
#include "verification/verify.h"
#include "sygus_problem.h"
#include <util/namespace.h>

// class for doing CEGIS. Can receive any syntht synthesizer and a verifyt verifier.

class cegist
{
public:
  cegist(
    syntht &_synthesizer,
    verifyt &_verify,
    sygus_problemt &_problem,
    namespacet &_ns,
    message_handlert &_ms) : synthesizer(_synthesizer),
                            verify(_verify),
                            problem(_problem),
                            ns(_ns),
                            message(_ms)
                            {};

  using resultt = decision_proceduret::resultt;
  resultt doit();

protected:
  syntht &synthesizer;
  verifyt &verify;
  sygus_problemt &problem;
  namespacet ns;
  solutiont solution;
  messaget message;

};
#endif /* CEGIS_H_ */