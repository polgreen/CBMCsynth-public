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

// this function executes a basic counterexample guided inductive synthesis loop
cegist::resultt cegist::doit()
{
solutiont solution;
while(true)
{
   switch(synthesizer())
    {
    case syntht::CANDIDATE:
    // we got a candidate.
      break;
    case syntht::NO_SOLUTION:
    // synthesis didn't find a solution, return, we can't solve this problem
      return decision_proceduret::resultt::D_UNSATISFIABLE;
    }

    solutiont solution = synthesizer.get_solution();
    switch(verify(problem, solution))
    {
    case verifyt::PASS:
      std::cout<<"Verification passed" <<std::endl;
      // TODO: pretty print the solution
      return decision_proceduret::resultt::D_SATISFIABLE;
    case verifyt::FAIL:
      std::cout<<"Verification failed" <<std::endl;
      synthesizer.add_counterexample(verify.get_counterexample());
      break;
    }
  }
  return decision_proceduret::resultt::D_UNSATISFIABLE;
} 

