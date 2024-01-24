//
// Created by julian on 24.04.23.
//

#ifndef CBMCSMT_UTIL_H
#define CBMCSMT_UTIL_H

#include <utility>
#include <filesystem>

#include <util/cmdline.h>
#include <util/cout_message.h>
#include <util/format_expr.h>
#include <util/namespace.h>
#include <util/replace_symbol.h>
#include <util/simplify_expr.h>
#include <util/symbol_table.h>
#include <util/std_expr.h>
#include <util/expr.h>
#include <util/mathematical_expr.h>

#include "../constants.h"
#include "../sygus_problem.h"


// creates a function application exprt, given the function name, operands, and codomain
function_application_exprt
create_func_app(irep_idt function_name, const std::vector<exprt> &operands, const typet &codomain);

// expands let expressions in an expression
void expand_let_expressions(exprt &expr);

// expands function applications in an expression
void expand_function_applications(exprt &expr, const std::map<symbol_exprt, exprt> &defined_functions);

// // builds a list of functions used in expression that have definitions
void get_defined_functions(
    const exprt &expr, const std::map<symbol_exprt, exprt> &defined_functions,
    std::set<symbol_exprt> &list);

// returns true if expr contains a call to function of f_name
bool contains_function_call(irep_idt f_name, const exprt &expr);

// Replaces all occurences of a string in a string with another string
std::string replace_occurences(std::string str, const std::string &from, const std::string &to);

// replaces nth occurrence of what with by in dest
bool replace_nth_occurrence(const exprt &what, const exprt &by, exprt &dest, const std::size_t &n);

// converts a exprt to NNF form
void nnf(exprt &expr, bool negate);

// not implemented. Will convert function to DNF form
void dnf(exprt &expr);

// does some basic simplification of an expression
void basic_simplify(exprt &expr);

// does some basic simplification of an expression
void basic_simplify(exprt &expr, const syntactic_templatet & grammar);

// counts the number of times any symbol in the ids vector occurs in the expression
std::size_t count_symbol_occurrences(const exprt &expr , const std::vector<irep_idt> &ids);

#endif //CBMCSMT_UTIL_H
