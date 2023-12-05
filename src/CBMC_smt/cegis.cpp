#include "cegis.h"
#include <iostream>

cegist::cegist(
  syntht &__synthesizer,
  verifyt &__verify,
  sygus_problemt &__problem, 
  namespacet &_ns) :
  synthesizer(__synthesizer),
  verify(__verify),
  problem(__problem),
  ns(_ns)
{

}

cegist::resultt cegist::doit()
{
solutiont solution;
while(true)
{
   switch(synthesizer())
    {
    case syntht::CANDIDATE:
    // we chekc this later
      break;
    case syntht::NO_SOLUTION:
        return decision_proceduret::resultt::D_UNSATISFIABLE;
    }

    solutiont solution = synthesizer.get_solution();
    switch(verify(problem, solution))
    {
    case verifyt::PASS:
      std::cout<<"Verification passed" <<std::endl;
      return decision_proceduret::resultt::D_SATISFIABLE;
    case verifyt::FAIL:
      break;
    }
  }
  return decision_proceduret::resultt::D_UNSATISFIABLE;
} 

