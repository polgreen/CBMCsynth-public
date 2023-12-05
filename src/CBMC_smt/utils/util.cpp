//
// Created by julian on 24.04.23.
//

#include "util.h"
#include <iostream>
#include <vector>
#include <filesystem>

#include <util/expr_iterator.h>
#include <util/suffix.h>
#include <util/arith_tools.h>
#include <util/expr_util.h>

#include <iostream>

void dnf(exprt &expr)
{
  // nnf(expr, false);
  // for (auto &op: expr.operands())
  //   dnf(op);

  // if(expr.id()==ID_and)
  // {
  //   std::vector<exprt> disjuncts;
  //   std::vector<exprt> conjuncts;
  //   for(auto &op: expr.operands())
  //   {
  //     // if(op.id()==ID_or)
  //     // {

  //     // }
  //     // elif (op.id()==ID_and)
  //     // {
  //     //   conjuncts.push_back(op);
  //     // }
  //   }
  // }
  
}


void nnf(exprt &expr, bool negate)
{
  if(!expr.is_boolean())
    return;

  if(expr.is_true())
  {
    if(negate)
      expr=false_exprt();
  }
  else if(expr.is_false())
  {
    if(negate)
      expr=true_exprt();
  }
  else if(expr.id()==ID_not)
  {
    nnf(to_not_expr(expr).op(), !negate);
    exprt tmp;
    tmp.swap(to_not_expr(expr).op());
    expr.swap(tmp);
  }
  else if(expr.id()==ID_and)
  {
    if(negate)
      expr.id(ID_or);

    Forall_operands(it, expr)
      nnf(*it, negate);
  }
  else if(expr.id()==ID_or)
  {
    if(negate)
      expr.id(ID_and);

    Forall_operands(it, expr)
      nnf(*it, negate);
  }
  else if(expr.id()==ID_typecast)
  {
    const auto &typecast_expr = to_typecast_expr(expr);

    if(
      typecast_expr.op().type().id() == ID_unsignedbv ||
      typecast_expr.op().type().id() == ID_signedbv)
    {
      equal_exprt tmp(
        typecast_expr.op(), from_integer(0, typecast_expr.op().type()));
      nnf(tmp, !negate);
      expr.swap(tmp);
    }
    else if(negate)
    {
      expr = boolean_negate(expr);
    }
  }
  else if(expr.id()==ID_le)
  {
    if(negate)
    {
      // !a<=b <-> !b=>a <-> b<a
      expr.id(ID_lt);
      auto &rel = to_binary_relation_expr(expr);
      std::swap(rel.lhs(), rel.rhs());
    }
  }
  else if(expr.id()==ID_lt)
  {
    if(negate)
    {
      // !a<b <-> !b>a <-> b<=a
      expr.id(ID_le);
      auto &rel = to_binary_relation_expr(expr);
      std::swap(rel.lhs(), rel.rhs());
    }
  }
  else if(expr.id()==ID_ge)
  {
    if(negate)
      expr.id(ID_lt);
    else
    {
      expr.id(ID_le);
      auto &rel = to_binary_relation_expr(expr);
      std::swap(rel.lhs(), rel.rhs());
    }
  }
  else if(expr.id()==ID_gt)
  {
    if(negate)
      expr.id(ID_le);
    else
    {
      expr.id(ID_lt);
      auto &rel = to_binary_relation_expr(expr);
      std::swap(rel.lhs(), rel.rhs());
    }
  }
  else if(expr.id()==ID_equal)
  {
    if(negate)
      expr.id(ID_notequal);
  }
  else if(expr.id()==ID_notequal)
  {
    if(negate)
      expr.id(ID_equal);
  }
  else if(expr.id()==ID_implies)
  {
    if(negate)
    {
      and_exprt tmp(to_implies_expr(expr).op0(), boolean_negate(to_implies_expr(expr).op1()));
      expr.swap(tmp);
    }
    else
    {
    or_exprt tmp(boolean_negate(to_implies_expr(expr).op0()), to_implies_expr(expr).op1());
    expr.swap(tmp);
    }
    nnf(expr, negate);
  }
  else if(negate)
  {
    expr = boolean_negate(expr);
  }
  else
  {
    // no change
    std::cout<<"no change"<<std::endl;
  }
}





function_application_exprt
create_func_app(irep_idt function_name, const std::vector<exprt> &operands, const typet &codomain) {
    // create function type
    std::vector<typet> domain;
    for (const auto &op: operands) {
        domain.push_back(op.type());
    }
    mathematical_function_typet function_type(domain, codomain);
    return function_application_exprt{symbol_exprt(function_name, function_type), operands};
}



void replace_local_var(exprt &expr, const irep_idt &target, exprt &replacement) {
    if (expr.id() == ID_symbol) {
        if (to_symbol_expr(expr).get_identifier() == target)
            expr = replacement;
    }
    if(expr.id()==ID_let)
    {
       replace_local_var(to_let_expr(expr).where(), target, replacement);
       for(auto &val: to_let_expr(expr).values())
           replace_local_var(val, target, replacement);
    }
    else{
        for (auto &op: expr.operands())
            replace_local_var(op, target, replacement);
    }
}


void expand_let_expressions(exprt &expr) {
    if (expr.id() == ID_let) {
        auto &let_expr = to_let_expr(expr);
        for (unsigned int i = 0; i < let_expr.variables().size(); i++) {
            INVARIANT(let_expr.variables()[i].id() == ID_symbol,
                      "Let expression should have list of symbols, not " + let_expr.variables()[i].pretty());
            replace_local_var(let_expr.where(), to_symbol_expr(let_expr.variables()[i]).get_identifier(),
                              let_expr.values()[i]);
        }
        expr = let_expr.where();
        expand_let_expressions(expr);
    }
    for (auto &op: expr.operands())
        expand_let_expressions(op);
}


std::string replace_occurences(std::string str, const std::string &from, const std::string &to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}