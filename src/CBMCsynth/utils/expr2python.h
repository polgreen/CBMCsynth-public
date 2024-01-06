#ifndef __EXPR_TO_PYTHON_H_
#define __EXPR_TO_PYTHON_H_

#include <util/expr.h>
#include <util/floatbv_expr.h>
#include <iostream>
#include "../sygus_problem.h"

std::string expr2python(const exprt &expr, const std::vector<irep_idt> &placeholder_ids);
std::string expr2python(const exprt &expr);
std::string synth_fun_to_python(const synth_funt &fun, const exprt &def, const std::vector<irep_idt> &placeholder_ids);
std::string fun_to_python(const symbol_exprt &fun, const exprt &def, const std::vector<irep_idt> &placeholder_ids);


#endif /*__EXPR_TO_PYTHON_H_*/