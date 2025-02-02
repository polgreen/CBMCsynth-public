#ifndef _SMT_PROBLEM_H_
#define _SMT_PROBLEM_H_

// we store anything we might need to pass around here
#include <util/std_expr.h>
#include <map>

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


class smt_problemt {
public:
    std::string filename;

    std::vector<exprt> assertions;
    std::map<symbol_exprt, exprt> free_var;
    std::map<symbol_exprt, exprt> defined_functions;
    std::string logic;
    std::vector<std::string> comments;

};

smt_problemt parse_problem(const std::string &filename);

/*
 * creates conjunction of assertions and negates the term.
 * Turns an unsatisfiable problem into a valid one
 * */
smt_problemt negate_problem(const smt_problemt &problem);

#endif /*_SMT_PROBLEM_H_*/