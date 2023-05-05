//
// Created by julian on 24.04.23.
//

#include "util.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <random>
#include <filesystem>

#include "constants.h"
#include <util/expr_iterator.h>
#include <util/cmdline.h>
#include <util/suffix.h>


/*
 * Check if the root symbols in expr a and b are the same
 * */
bool root_equality(const exprt& a, const exprt& b)
{
    if (a.id() == b.id()) {
        if (a.is_constant() and b.is_constant()) {
            const constant_exprt a_cnst = to_constant_expr(a);
            const constant_exprt b_cnst = to_constant_expr(b);

            if (a_cnst.get_value() == b_cnst.get_value()) {
                return true;
            } else {
                return false;
            }
        } else if (a.id() == ID_symbol and b.id() == ID_symbol) {
            const symbol_exprt a_expr = to_symbol_expr(a);
            const symbol_exprt b_expr = to_symbol_expr(b);
            if (a_expr.get_identifier() == b_expr.get_identifier()) {
                return true;
            } else {
                return false;
            }
        } else {
            if (a.operands().size() != b.operands().size()) {
                return false;
            }
            for( int i = 0; i < a.operands().size(); ++i) { // for functions that are polymorphic
                if (a.operands()[i].type() != b.operands()[i].type()) {
                    return false;
                }
            }


            return true;
        }
    }
    return false;
}

function_application_exprt create_func_app(irep_idt function_name, const std::vector<exprt>& operands, const typet& codomain)
{
    // create function type
    std::vector<typet> domain;
    for(const auto &op: operands) {
        domain.push_back(op.type());
    }
    mathematical_function_typet function_type(domain, codomain);
    return function_application_exprt{symbol_exprt(function_name, function_type), operands};
}

std::size_t expr_height(const exprt& expr) {
    if (! expr.has_operands()) {
        return 1;
    } else {
        size_t max = 0;
        for (const auto& op : expr.operands()) {
            size_t op_depth = expr_height(op);
            max = op_depth > max ? op_depth : max;
        }
        return max + 1;
    }
}

bool is_binder(const exprt& expr) {
    if (expr.id()  == ID_let_binding) {
        return true;
    }
    if (expr.id() == ID_exists) {
        return true;
    }
    if (expr. id() == ID_forall) {
        return true;
    }
    return false;
}

std::size_t is_binder_free(const exprt& expr) {
    for(auto it = expr.depth_begin() , itend = expr.depth_end(); it != itend; ++it) {
        if (is_binder(*it)) {
            return false;
        }
    }
    return true;
}


std::vector<std::string> files_with_suffix_in_dirs(const std::vector<std::string>& dirs, const std::string& suffix) {
    std::vector<std::string> res;

    for (auto& dir : dirs) {
        if (std::filesystem::is_regular_file(dir)) {
            if (has_suffix(dir, suffix)) {
                res.push_back(dir);
            }
            continue;
        }
        for (const auto &entry: std::filesystem::directory_iterator(dir)) {
            if (has_suffix(entry.path().string(), suffix)) {
                res.push_back(entry.path().string());
            }
        }
    }

    return res;
}
