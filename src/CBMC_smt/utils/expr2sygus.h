#ifndef EMU_EXPR2SYGUS
#define EMU_EXPR2SYGUS

#include <util/expr.h>
#include <util/floatbv_expr.h>
#include <iostream>
#include "../sygus_problem.h"

// converts exprts to SyGuS strings

enum class wheret {
    BEGIN, END
};
#define UNEXPECTEDCASE(S) PRECONDITION_WITH_DIAGNOSTICS(false, S);

// General todos
#define SMT2_TODO(S) PRECONDITION_WITH_DIAGNOSTICS(false, "TODO: " S)

std::string clean_id(const irep_idt &id);

void clean_symbols(exprt &expr);

std::string type2sygus(const typet &type);

std::string expr2sygus(const exprt &expr);

std::string synth_fun_dec(const synth_funt &definition);

std::string fun_def(const symbol_exprt &function, const exprt &body);
std::string fun_dec(const symbol_exprt &fun);

std::string var_dec(const symbol_exprt &symbol);


std::string convert_typecast(const typecast_exprt &expr);

std::string convert_floatbv_typecast(const floatbv_typecast_exprt &expr);

std::string convert_constant(const constant_exprt &expr);

std::string convert_relation(const binary_relation_exprt &);

std::string convert_plus(const plus_exprt &expr);

std::string convert_minus(const minus_exprt &expr);

std::string convert_div(const div_exprt &expr);

std::string convert_mult(const mult_exprt &expr);

std::string convert_rounding_mode_FPA(const exprt &expr);

std::string convert_floatbv_plus(const ieee_float_op_exprt &expr);

std::string convert_floatbv_minus(const ieee_float_op_exprt &expr);

std::string convert_floatbv_div(const ieee_float_op_exprt &expr);

std::string convert_floatbv_mult(const ieee_float_op_exprt &expr);

std::string convert_mod(const mod_exprt &expr);

std::string convert_index(const index_exprt &expr);

std::string convert_member(const member_exprt &expr);

std::string convert_with(const with_exprt &expr);

std::string convert_expr(const exprt &);

std::string convert_type(const typet &);

std::string flatten2bv(const exprt &expr);

/*
 * Printer for SyGuS problems
 * */
std::string build_sygus_query(const sygus_problemt &problem);

std::string build_sygus_query(const sygus_problemt &problem, bool add_default_grammar);


#endif /* EMU_EXPR2SYGUS*/