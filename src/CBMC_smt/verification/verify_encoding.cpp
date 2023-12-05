
#include "verify_encoding.h"

#include <util/arith_tools.h>


void verify_encodingt::check_function_bodies(
  const std::map<symbol_exprt, exprt>  &functions)
{
  for(const auto &f : functions)
  {
    if(ID_bool == f.second.type().id())
      continue; // Synth encoding with just literal assignments as solution.

    const auto &signature = to_mathematical_function_type(f.first.type());
    check_function_body(signature, f.second);
    if(f.second.type()!=signature.codomain())
    {
      throw "function body has wrong type";
    }
  }
}


exprt verify_encodingt::operator()(const exprt &expr) const
{
  if(expr.id()==ID_function_application)
  {
    const auto &e=to_function_application_expr(expr);

    DATA_INVARIANT(e.function().id() == ID_symbol,
      "function applied must be symbol");

    const auto &e_identifier =
      to_symbol_expr(e.function()).get_identifier();

    std::map<irep_idt, exprt> f_map;
    for(const auto &f : functions)
      f_map[f.first.get_identifier()]=f.second;

    auto f_it=f_map.find(e_identifier);

    if(f_it==f_map.end())
    {
      exprt tmp=expr;

      for(auto &op : tmp.operands())
        op=(*this)(op);

      exprt instance=instantiate(tmp, e);  
      return tmp;
    }
    else
    {
      exprt result = f_it->second;
      // need to instantiate parameters with arguments
      exprt instance=instantiate(result, e);

      return instance;
    }
  }
  else
  {
    exprt tmp=expr;

    for(auto &op : tmp.operands())
      op=(*this)(op);

    return tmp;
  }
}

exprt verify_encodingt::instantiate(
  const exprt &expr,
  const function_application_exprt &e) const
{
  if(expr.id()==ID_symbol)
  {
    const irep_idt identifier=to_symbol_expr(expr).get_identifier();
    static const std::string parameter_prefix="synth::parameter";

    if(std::string(id2string(identifier), 0, parameter_prefix.size())==parameter_prefix)
    {
      std::string suffix(id2string(identifier), parameter_prefix.size(), std::string::npos);
      std::size_t count=std::stoul(suffix);
      assert(count<e.arguments().size());
      return e.arguments()[count];
    }
    else
      return expr;
  }
  else
  {
    exprt tmp=expr;

    for(auto &op : tmp.operands())
      op=instantiate(op, e);

    return tmp;
  }
}


counterexamplet verify_encodingt::get_counterexample(
  const decision_proceduret &solver) const
{
  counterexamplet result;

  // iterate over nondeterministic symbols, and get their value
  for(const auto &var : free_variables)
  {
    exprt value=solver.get(var);
    result.assignment[var]=value;
    if(value==nil_exprt() && var.id()==ID_nondet_symbol)
    {
      nondet_symbol_exprt tmp_var=to_nondet_symbol_expr(var);
      tmp_var.set_identifier("nondet_"+id2string(to_nondet_symbol_expr(var).get_identifier()));
      value=solver.get(tmp_var);
      result.assignment[var]=value;
    }
    if(value==nil_exprt())
    {
      result.assignment[var] = constant_exprt("0", var.type());
    }
  }

  return result;
}

void verify_encodingt::clear()
{
}
