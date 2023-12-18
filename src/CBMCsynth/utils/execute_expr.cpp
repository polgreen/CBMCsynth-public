#include "execute_expr.h"
#include <string>

// unsound execution for integer exprs

bool execute_boolean_expr(const exprt &expr)
{
  if(expr.type().id()!=ID_bool)
  {
    throw "execute_boolean_expr: expr is not of boolean type";
  }

  if(expr.id()==ID_constant)
  {
    return to_constant_expr(expr).is_true();
  }
  if(expr.id()==ID_and)
  {
    return execute_boolean_expr(expr.operands()[0]) && execute_boolean_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_or)
  {
    return execute_boolean_expr(expr.operands()[0]) || execute_boolean_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_not)
  {
    return !execute_boolean_expr(expr.operands()[0]);
  }
  if(expr.id()==ID_implies)
  {
    return !execute_boolean_expr(expr.operands()[0]) || execute_boolean_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_if)
  {
    const auto &if_expr = to_if_expr(expr);
    if(execute_boolean_expr(if_expr.cond()))
    {
      return execute_boolean_expr(if_expr.true_case());
    }
    else
    {
      return execute_boolean_expr(if_expr.false_case());
    }
  }
  if(expr.id()==ID_equal)
  {
    return execute_integer_expr(expr.operands()[0]) == execute_integer_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_notequal)
  {
    return execute_integer_expr(expr.operands()[0]) != execute_integer_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_le)
  {
    return execute_integer_expr(expr.operands()[0]) <= execute_integer_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_lt)
  {
    return execute_integer_expr(expr.operands()[0]) < execute_integer_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_ge)
  {
    return execute_integer_expr(expr.operands()[0]) >= execute_integer_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_gt)
  {
    return execute_integer_expr(expr.operands()[0]) > execute_integer_expr(expr.operands()[1]);
  }
  
  throw "execute_boolean_expr: unsupported expr" + expr.pretty();
}

int execute_integer_expr(const exprt &expr)
{
  if(expr.type().id()!=ID_integer)
    throw "execute_integer_expr: expr is not of integer type";

  if(expr.id()==ID_constant)
  {
    return std::stoi(id2string(to_constant_expr(expr).get_value()));
  }
  if(expr.id()==ID_plus)
  {
    return execute_integer_expr(expr.operands()[0]) + execute_integer_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_minus)
  {
    return execute_integer_expr(expr.operands()[0]) - execute_integer_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_mult)
  {
    return execute_integer_expr(expr.operands()[0]) * execute_integer_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_div)
  {
    return execute_integer_expr(expr.operands()[0]) / execute_integer_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_mod)
  {
    return execute_integer_expr(expr.operands()[0]) % execute_integer_expr(expr.operands()[1]);
  }
  if(expr.id()==ID_unary_minus)
  {
    return -execute_integer_expr(expr.operands()[0]);
  }
  if(expr.id()==ID_if)
  {
    const auto &if_expr = to_if_expr(expr);
    if(execute_boolean_expr(if_expr.cond()))
    {
      return execute_integer_expr(if_expr.true_case());
    }
    else
    {
      return execute_integer_expr(if_expr.false_case());
    }
  }
  throw "execute_integer_expr: unsupported expr";
}