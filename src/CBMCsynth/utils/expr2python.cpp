#include "expr2python.h"
#include "expr2sygus.h"
#include <util/mathematical_expr.h>
#include <util/mathematical_types.h>
#include <util/format.h>
#include <util/format_expr.h>
#include <util/range.h>


std::string convert_expr_to_python(const exprt &expr, std::string indent, const std::vector<irep_idt> &placeholder_ids)
{
  std::string result;

  if(expr.id()==ID_symbol || expr.id()==ID_nondet_symbol)
  {
    auto id = to_symbol_expr(expr).get_identifier();
    for(const auto &nt: placeholder_ids)
    {
      if(id == nt)
      {
        result += "??";
        return result;
      }
    }
    result += expr2sygus(expr);
  }
  else if (expr.id()==ID_if)
  {
    const if_exprt &if_expr = to_if_expr(expr);

    result += indent + "if  ";
    result += convert_expr_to_python(if_expr.cond(),"", placeholder_ids);
    result += ": \n";
    result += convert_expr_to_python(if_expr.true_case(),"  "+indent, placeholder_ids) + "\n";
    result += indent + "else:\n";
    result += convert_expr_to_python(if_expr.false_case(), "  "+indent, placeholder_ids);
    result += "\n";
  }
 else if(expr.id()==ID_let)
 {
  const auto &let_expr = to_let_expr(expr);
  const auto &variables = let_expr.variables();
  const auto &values = let_expr.values();
  result +="\n";
  for (auto &binding : make_range(variables).zip(values))
  {
    result += indent + "var " + convert_expr_to_python(binding.first, indent, placeholder_ids) 
            + " = " + convert_expr_to_python(binding.second,indent, placeholder_ids) + "\n";
  }
  result += indent + convert_expr_to_python(let_expr.where(), indent, placeholder_ids) + "\n";
 }
 else if(expr.id()==ID_or || expr.id()==ID_and || expr.id()==ID_xor)
 {
  std::string op;
  result += convert_expr_to_python(expr.operands()[0], indent, placeholder_ids);
  result += " "+id2string(expr.id())+ " ";
  result += convert_expr_to_python(expr.operands()[1], indent, placeholder_ids);
 }
else if(expr.id()==ID_not)
{
  result += "not ";
  result += convert_expr_to_python(expr.operands()[0], indent, placeholder_ids);
 }
 else if(expr.id()==ID_plus)
 {
  result += convert_expr_to_python(expr.operands()[0], indent, placeholder_ids);
  result += " + ";
  result += convert_expr_to_python(expr.operands()[1], indent, placeholder_ids);
 }
else if(expr.id()==ID_minus)
 {
  result += convert_expr_to_python(expr.operands()[0], indent, placeholder_ids);
  result += " - ";
  result += convert_expr_to_python(expr.operands()[1], indent, placeholder_ids);
 }
 else if(expr.id()==ID_mult)
 {
  result += convert_expr_to_python(expr.operands()[0], indent, placeholder_ids);
  result += " * ";
  result += convert_expr_to_python(expr.operands()[1], indent, placeholder_ids);
 }
else if(expr.id()==ID_div)
 {
  result += convert_expr_to_python(expr.operands()[0], indent, placeholder_ids);
  result += " / ";
  result += convert_expr_to_python(expr.operands()[1], indent, placeholder_ids);
 }
 else if(expr.id()==ID_unary_minus)
 {
  result += " - ";
  result += convert_expr_to_python(expr.operands()[0], indent, placeholder_ids);
 }
 else
 {
  std::stringstream ss;
  ss << format(expr);
  result += indent + ss.str();
 }
  return result;
}


std::string synth_fun_to_python(const synth_funt &fun, const exprt &body, const std::vector<irep_idt> &placeholder_ids)
{
  INVARIANT(fun.type.id() == ID_mathematical_function,
            "function symbol must have function type, got " + fun.type.id_string() + " for " + id2string(fun.id));
  std::string result = "def " + clean_id(fun.id) + " (";

  unsigned int count=1;
  for(const auto &p: fun.parameters)
  {
    result += id2string(p.get_identifier());
    if(count < fun.parameters.size())
      result += ", ";
    count++;
  }
  result+= "):\n"; 

  result += convert_expr_to_python(body, "  ", placeholder_ids) + "\n";

  return result;
}



std::string fun_to_python(const symbol_exprt &fun, const exprt &def, const std::vector<irep_idt> &placeholder_ids)
{
  INVARIANT(fun.type().id() == ID_mathematical_function,
            "function symbol must have function type, got " + fun.type().id_string() + " for " + id2string(fun.get_identifier()));
  INVARIANT(def.id() == ID_lambda,
            "function definition must be a lambda expression");
  std::string result = "def " + clean_id(fun.get_identifier()) + " (";
  auto &fun_def = to_lambda_expr(def);
  unsigned int count=1;
  for(const auto &p: fun_def.variables())
  {
    result += id2string(p.get_identifier());
    if(count < fun_def.variables().size())
      result += ", ";
    count++;
  }
  result+= "):\n\n"; 

  result += convert_expr_to_python(fun_def.where(), "  ", placeholder_ids) + "\n";

  return result;
}

std::string expr2python(const exprt &expr)
{
  std::vector<irep_idt> placeholder_ids;
  return convert_expr_to_python(expr, "", placeholder_ids);
}

std::string expr2python(const exprt &expr, const std::vector<irep_idt> &placeholder_ids)
{
  return convert_expr_to_python(expr, "", placeholder_ids);
}

