#include "printing_utils.h"
#include "expr2sygus.h"

#include <util/format_expr.h>
#include <util/std_expr.h>
#include <util/expr.h>

// print the expression tree for each assertion
void print_smt_problem(const smt_problemt &problem, std::ostream &out)
{
  out << "(set-logic " << problem.logic << ")" << std::endl;
  int count = 0;

  for (const auto &f : problem.defined_functions)
  {
    out << fun_def(f.first, f.second) << "\n";
  }

  for (const auto &e : problem.free_var)
  {
    out << fun_dec(e.first) << std::endl;
  }

  for (const auto &a : problem.assertions)
  {
    out << " ; Assertion " << count << "\n";
    out << "(assert ";
    count++;
    out << expr2sygus(a) << ")" << std::endl;
  }
  out << "(check-sat)\n";
}

void print_sygus_as_smt(const sygus_problemt &problem, std::ostream &out)
{
  out << "; printing sygus problem as smt problem " << std::endl;
  out << "(set-logic " << problem.logic << ")" << std::endl;

  for (const auto &f : problem.defined_functions)
    out << fun_def(f.first, f.second) << "\n";

  // print the synthesis functions as UFs
  for(const auto &f: problem.synthesis_functions)
    out << fun_dec(symbol_exprt(f.id, f.type)) << "\n";
  // we are looking for a model such that all constraints
  // hold for all free variables
  out <<"(assert (forall (";

  for (const auto &e : problem.free_var)
    out << "("<< e.get_identifier() << " "<< type2sygus(e.type())<<" )";
  out << ")";
  out <<"(and ";
  for (const auto &c: problem.constraints)
    out << expr2sygus(c) << " \n";
  out <<")\n";

  out <<"(check-sat)\n";
}

void print_sygus_problem(const sygus_problemt &problem, std::ostream &out)
{
  out << "; printing sygus problem  " << std::endl;
  out << build_sygus_query(problem) << std::endl;
}

void print_model(smt_problemt &problem, std::ostream &out)
{
  out << "Model:" << std::endl;
  for (const auto &e : problem.free_var)
  {
    out << "Free var : " << e.first.get_identifier();
    if (!e.second.is_nil())
      out << " = " << expr2sygus(e.second) << std::endl;
    else
      out << "[no assignment]" << std::endl;
  }
}

void traverse_expression(exprt &expr, std::ostream &out)
{
  out << expr.id() << std::endl;
  for (auto op : expr.operands())
  {
    out << "looking at expression: " << op.pretty() << std::endl;
    if (op.id() == ID_symbol)
    {
      out << "Found symbol: " << op.get(ID_identifier) << std::endl;
    }
    traverse_expression(op, out);
  }
}

void print_problem_tree(smt_problemt &problem, std::ostream &out)
{
  out << "Problem tree:" << std::endl;
  int count = 0;
  for (const auto &a : problem.assertions)
  {
    out << "Assertion " << count << std::endl;
    count++;
    out << a.pretty() << std::endl;
  }
}