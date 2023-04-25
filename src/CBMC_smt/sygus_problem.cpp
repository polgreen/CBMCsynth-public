//
// Created by julian on 21.03.23.
//

#include "sygus_problem.h"
#include "problem.h"


#include "parser.h"
#include "printing_utils.h"
#include "problem.h"

#include <fstream>
#include <iostream>
#include <set>

#include <util/cmdline.h>
#include <util/cout_message.h>
#include <util/format_expr.h>
#include <util/namespace.h>
#include <util/replace_symbol.h>
#include <util/simplify_expr.h>
#include <util/symbol_table.h>
#include <util/std_expr.h>
#include <util/arith_tools.h>

#include <solvers/smt2/smt2_dec.h>
#include "smt2_frontend.h"
#include "term_position.h"
#define UNEXPECTEDCASE(S) PRECONDITION_WITH_DIAGNOSTICS(false, S);


symbol_exprt nonterminal(const typet &type)
{
  if(type.id()==ID_unsignedbv || type.id()==ID_integer)
    return symbol_exprt("NTnonbool", type);
  if(type.id()==ID_bool)
    return symbol_exprt("NTbool", type);

  UNEXPECTEDCASE("unsupported type in production rule"); 
}


std::vector<exprt> production_rule(const synth_fun_commandt &f, const typet &type)
{
  symbol_exprt NT = nonterminal(type);
  std::vector<exprt> rules;
  auto func_type = to_mathematical_function_type(f.type);
  for(std::size_t i=0; i< func_type.domain().size(); i++)
  {
    if(func_type.domain()[i]==type) // add relevant parameters
      rules.push_back(symbol_exprt(f.parameters[i],type));
  }

  if(type.id()!=ID_bool) // add constants
  {
    rules.push_back(from_integer(0, type));
    rules.push_back(from_integer(1, type));
  }
  if(type.id()==ID_integer)
  {
    rules.push_back(unary_minus_exprt(NT));
    rules.push_back(plus_exprt(NT,NT));
    rules.push_back(minus_exprt(NT,NT));
    rules.push_back(if_exprt(nonterminal(bool_typet()),NT,NT));
    // rules.push_back(mult_exprt(NT,NT));
    // rules.push_back(div_exprt(NT,NT));
  }
  else if(type.id()==ID_bool)
  {
    rules.push_back(not_exprt(NT));
    rules.push_back(and_exprt(NT,NT));
    rules.push_back(or_exprt(NT,NT));
    rules.push_back(if_exprt(NT,NT,NT));
  }
  else
  {
    UNEXPECTEDCASE("unsupported type in production rule"); 
  }



 return rules;
}


synth_fun_commandt add_grammar(const synth_fun_commandt &f)
{
  synth_fun_commandt result = f;
  syntactic_templatet grammar;
  std::set<typet> types;
  bool integer=false;
  bool bv=false;
  bool hasbool=false;

  auto func = to_mathematical_function_type(f.type);
  for (const auto &t : func.domain())
  {
    if(t.id()==ID_integer)
      integer=true;
    if(t.id()==ID_unsignedbv) 
      bv=true;
    if(t.id()==ID_bool)
      hasbool=true;  
    if(t!=func.codomain())
      types.insert(t);
  }
  INVARIANT(!(bv & integer), "do not support grammars with both bv and int");
  INVARIANT(types.size()<=2, "do not support more than 2 types in a grammar");

  auto codomain_production_rule = production_rule(f, func.codomain());
  grammar.production_rules[nonterminal(func.codomain()).get_identifier()] = codomain_production_rule;
  grammar.nt_ids.push_back(nonterminal(func.codomain()));

  for(const auto &t: types)
  {
    grammar.nt_ids.push_back(nonterminal(t));
    grammar.production_rules[nonterminal(t).get_identifier()] = production_rule(f, t);
  }
  if(!hasbool && func.codomain().id()!=ID_bool)
  {
    grammar.nt_ids.push_back(nonterminal(bool_typet()));
    grammar.production_rules[nonterminal(bool_typet()).get_identifier()] = production_rule(f, bool_typet());
  }
  result.grammar = grammar;
  return result;
}


sygus_problemt create_training_data(problemt &smt_problem, namespacet &ns, messaget &log) {

    decision_proceduret::resultt res = solve_problem(smt_problem, ns, log);

    if (res == decision_proceduret::resultt::D_SATISFIABLE) {
        // replace the free variables in the assertions with the values from the model
        problemt new_valid_problem = substitute_model_into_problem(smt_problem);


    } else if (res == decision_proceduret::resultt::D_UNSATISFIABLE) {
        //problemt new_valid_problem = negate_problem(problem);

    }

    return sygus_problemt();
}


std::vector<sygus_problemt> create_training_data(std::vector<std::string> files) {
    std::vector<sygus_problemt> res;
    for (auto file : files){
        //parsing
        // auto x = create_training_data(problem);
        // res.push_back(x);
    }
    return res;
}
