#ifndef _SMT2_FRONTEND_H_
#define _SMT2_FRONTEND_H_

#include <util/cmdline.h>
#include <util/cmdline.h>
#include <util/cout_message.h>
#include <util/format_expr.h>
#include <util/namespace.h>
#include <util/replace_symbol.h>
#include <util/simplify_expr.h>
#include <util/symbol_table.h>
#include <util/std_expr.h>
#include <util/expr.h>

#include <solvers/smt2/smt2_dec.h>

#include "problem.h"
#include "parser.h"


int smt2_frontend(const cmdlinet &);

int smt2_frontend(const cmdlinet &, std::istream &in);

decision_proceduret::resultt solve_problem(problemt &problem, namespacet &ns, messaget &message);

problemt substitute_model_into_problem(const problemt &problem);

problemt build_problem(parsert &parser);

#endif /*_SMT2_FRONTEND_H_*/