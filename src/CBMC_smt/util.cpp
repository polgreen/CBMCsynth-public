//
// Created by julian on 24.04.23.
//

#include "util.h"
#include <iostream>
#include <format.h>
#include <mathematical_expr.h>

/*
 * Check if the root symbols in expr a and b are the same
 * */
bool root_equality(const exprt& a, const exprt& b)
{
    if (a.id() != b.id()) {
        return false;
    } else if (a.is_constant()) {
        if (!b.is_constant()) {
            return false;
        }
        const constant_exprt a_cnst = to_constant_expr(a);
        const constant_exprt b_cnst = to_constant_expr(b);

        if (a_cnst.get_value() != b_cnst.get_value()) {
            return false;
        } else {
            return true;
        }
    } else if (b.is_constant()) {
        if (! a.is_constant()) {
            return false;
        }
        const constant_exprt a_cnst = to_constant_expr(a);
        const constant_exprt b_cnst = to_constant_expr(b);
        if (a_cnst.get_value() != b_cnst.get_value()) {
            return false;
        } else {
            return true;
        }
    } else if (a.id() == ID_symbol) {
        if (b.id() != ID_symbol) {
            return false;
        }
        const symbol_exprt a_expr = to_symbol_expr(a);
        const symbol_exprt b_expr = to_symbol_expr(b);
        if (a_expr.get_identifier() != b_expr.get_identifier()) {
            return false;
        } else {
            return true;
        }
    } else if (b.id() == ID_symbol) {
        if (a.id() != ID_symbol) {
            return false;
        }
        const symbol_exprt a_expr = to_symbol_expr(a);
        const symbol_exprt b_expr = to_symbol_expr(b);
        if (a_expr.get_identifier() != b_expr.get_identifier()) {
            return false;
        } else {
            return true;
        }
    }

    return true;
}