#include "counterexample_verify.h"
#include "../utils/util.h"

#include <solvers/smt2/smt2_dec.h>
#include <langapi/language_util.h>

#include <util/format_expr.h>
#include <util/replace_expr.h>
#include <util/replace_symbol.h>
#include <util/run.h>

#include "../utils/expr2sygus.h"
#include <iostream>


counterexamplet counterexample_verifyt::get_failed_cex(){
  return failed_cex;
}

// adds the constraints to the solver
void counterexample_verifyt::add_problem(const sygus_problemt &problem, const solutiont &solution, const counterexamplet &cex, decision_proceduret &solver)
{
  // expand function applications, and add to solver.
  if (problem.assumptions.size() > 0)
  {
    exprt assumptions = conjunction(problem.assumptions);
    expand_function_applications(assumptions, solution.functions);
    expand_function_applications(assumptions, problem.defined_functions);
    expand_function_applications(assumptions, solution.functions);
    solver.set_to_true(assumptions);
  }

  exprt constraints = conjunction(problem.constraints);
  expand_function_applications(constraints, solution.functions);
  expand_function_applications(constraints, problem.defined_functions);
  // expand again, incase the body of any defined functions contained the synth functions
  expand_function_applications(constraints, solution.functions);
  solver.set_to_false(constraints);
  // add counterexample to solver
  for (const auto &c : cex.assignment)
  {
    solver.set_to_true(equal_exprt(c.first, c.second));
  }
}

// verifies whether a candidate solution works for a set of counterexamples
// returns PASS if it works for all counterexamples, FAIL otherwise
counterexample_verifyt::resultt counterexample_verifyt::operator()(sygus_problemt &problem,
    const solutiont &solution,const std::vector<counterexamplet> &cex)
  {
    for (const auto &c : cex)
    {
      // get SMT solver
      // we get a new SMT solver for every counterexample because it was
      // easier than resetting the solver. It would be better to do this
      // using solving under assumptions.
      // TODO: implement this using solving under assumptions
      smt2_dect solver(
      ns, "cex_verify", "generated by counterexample verifier",
      "ALL", smt2_dect::solvert::Z3, message.get_message_handler());

      add_problem(problem, solution, c, solver);
      // get the result
      decision_proceduret::resultt result = solver();
      switch (result)
      {
      case decision_proceduret::resultt::D_SATISFIABLE:
      {
        failed_cex = c;  
        // failed to satisfy a counterexample, return FAIL
        return counterexample_verifyt::resultt::FAIL;
      }
      case decision_proceduret::resultt::D_ERROR:
      {
        message.status() << "ERROR in counterexample verifier \n" << messaget::eom;
        return counterexample_verifyt::resultt::FAIL;
      }
      case decision_proceduret::resultt::D_UNSATISFIABLE:
      default:
      {
        // do nothing
      }
      }
    }
    // we only get here if we satisfied all counterexamples
    return counterexample_verifyt::resultt::PASS;
  }

