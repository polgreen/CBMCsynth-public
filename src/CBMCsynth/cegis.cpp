#include "cegis.h"
#include "utils/expr2sygus.h"
#include <iostream>

// this function executes a basic counterexample guided inductive synthesis loop
cegist::resultt cegist::doit()
{
  std::size_t iteration = 0;
  solutiont solution;
  while (true)
  {
    switch (synthesizer(iteration))
    {
    case syntht::CANDIDATE:
      // we got a candidate.
      break;
    case syntht::NO_SOLUTION:
      // synthesis didn't find a solution, return, we can't solve this problem
      return decision_proceduret::resultt::D_UNSATISFIABLE;
    }
    iteration++;

    solutiont solution = synthesizer.get_solution();
    message.status() << "Candidate: ";
    for (const auto &f : solution.functions)
    {
      message.status() << fun_def(f.first, f.second) << messaget::eom;
    }

    switch (verify(problem, solution))
    {
    case verifyt::PASS:
      message.status() << "Verification passed" << messaget::eom;
      // TODO: pretty print the solution
      return decision_proceduret::resultt::D_SATISFIABLE;
    case verifyt::FAIL:
      message.status() << "Verification failed" << messaget::eom;
      synthesizer.add_counterexample(verify.get_counterexample());
      break;
    }
  }
  return decision_proceduret::resultt::D_UNSATISFIABLE;
}
