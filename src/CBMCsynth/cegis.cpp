#include "cegis.h"
#include "utils/util.h"
#include "utils/expr2sygus.h"
#include <iostream>

void cegist::print_solution()
{
  message.status() << "Solution :" << messaget::eom;
  std::set<symbol_exprt> dependencies;
  for (auto &f : solution.functions)
    get_defined_functions(f.second, problem.defined_functions, dependencies);

  for (const auto &f : dependencies)
    std::cout << fun_def(f, problem.defined_functions[f]) << std::endl;

  for (const auto &f : solution.functions)
    std::cout << fun_def(f.first, f.second) << std::endl;
}

// this function executes a basic counterexample guided inductive synthesis loop
cegist::resultt cegist::doit()
{
  std::size_t iteration = 0;
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

    solution = synthesizer.get_solution();
    message.status() << "Candidate: ";
    for (const auto &f : solution.functions)
    {
      message.status() << fun_def(f.first, f.second) << messaget::eom;
    }

    switch (verify(problem, solution))
    {
    case verifyt::PASS:
      message.status() << "Verification passed" << messaget::eom;
      print_solution();
      return decision_proceduret::resultt::D_SATISFIABLE;
    case verifyt::FAIL:
      message.status() << "Verification failed" << messaget::eom;
      synthesizer.add_counterexample(verify.get_counterexample());
      break;
    }
  }
  return decision_proceduret::resultt::D_UNSATISFIABLE;
}
