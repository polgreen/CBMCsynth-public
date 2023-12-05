#ifndef CEGIS_H_
#define CEGIS_H_

#include "synthesis/synth.h"
#include "verification/verify.h"
#include "sygus_problem.h"
#include <util/namespace.h>

class cegist
{
public:
  cegist(
    syntht &,
    verifyt &,
    sygus_problemt &,
    namespacet &);

  using resultt = decision_proceduret::resultt;
  resultt doit();

protected:
  syntht &synthesizer;
  verifyt &verify;
  sygus_problemt &problem;
  namespacet ns;
  solutiont solution;

};
#endif /* CEGIS_H_ */