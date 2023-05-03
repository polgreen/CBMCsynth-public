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


irep_idt nonterminalID(const typet &type)
{
  if(type.id()==ID_integer)
    return "NTInt";
  else if(type.id()==ID_bool)
    return "NTbool";
  else
    UNEXPECTEDCASE("unsupported type in production rule"); 
}


std::map<irep_idt,std::vector<exprt>> production_rules(const synth_fun_commandt &f)
{
  irep_idt int_id = nonterminalID(integer_typet());
  irep_idt bool_id = nonterminalID(bool_typet());
  symbol_exprt NT_integer = symbol_exprt(int_id, integer_typet());
  symbol_exprt NT_bool = symbol_exprt(bool_id, bool_typet());
  std::vector<exprt> integer_rules;
  std::vector<exprt> bool_rules;
  std::map<irep_idt, std::vector<exprt>> rules;

  auto func_type = to_mathematical_function_type(f.type);
  for(std::size_t i=0; i< func_type.domain().size(); i++)
  {
    if(func_type.domain()[i]==integer_typet()) // add relevant parameters
      integer_rules.push_back(symbol_exprt(f.parameters[i],integer_typet()));
    else if(func_type.domain()[i]==bool_typet())
      bool_rules.push_back(symbol_exprt(f.parameters[i],bool_typet()));
    else {
        UNEXPECTEDCASE("synthesis function parameter of unexpected type");
    }
  }
  // constants
  integer_rules.push_back(from_integer(0, integer_typet()));
  integer_rules.push_back(from_integer(1, integer_typet()));

  integer_rules.push_back(unary_minus_exprt(NT_integer));
  integer_rules.push_back(plus_exprt(NT_integer,NT_integer));
  integer_rules.push_back(minus_exprt(NT_integer,NT_integer));
  integer_rules.push_back(if_exprt(NT_bool,NT_integer,NT_integer));

  bool_rules.push_back(not_exprt(NT_bool));
  bool_rules.push_back(and_exprt(NT_bool,NT_bool));
  bool_rules.push_back(or_exprt(NT_bool,NT_bool));
  bool_rules.push_back(if_exprt(NT_bool,NT_bool,NT_bool));
  bool_rules.push_back(equal_exprt(NT_integer,NT_integer));
  bool_rules.push_back(less_than_exprt(NT_integer,NT_integer));
  bool_rules.push_back(greater_than_exprt(NT_integer,NT_integer));

  rules[int_id]=integer_rules;
  rules[bool_id]=bool_rules;

 return rules;
}


synth_fun_commandt add_grammar(const synth_fun_commandt &f)
{
  synth_fun_commandt result = f;
  syntactic_templatet grammar;

  auto func = to_mathematical_function_type(f.type);
  for (const auto &t : func.domain())
  {
    if(t.id()==ID_unsignedbv || t.id()==ID_signedbv)
      UNEXPECTEDCASE("bitvector synthesis not supported yet");
  }

  grammar.nt_ids.push_back(nonterminalID(func.codomain()));
  grammar.production_rules = production_rules(f);

  for(const auto &r: grammar.production_rules)
  {
    if(r.first==nonterminalID(func.codomain()))
      continue;
    grammar.nt_ids.push_back(r.first);
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
