//
// Created by julian on 15.03.23.
//

#include "term_position.h"
#include "parser.h"
#include "printing_utils.h"
#include "problem.h"

#include <fstream>
#include <iostream>

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


term_position operator+(term_position& first, const term_position& second) {
    first.position.insert(first.position.end(), second.position.begin(), second.position.end());
    return first;
}


std::multimap<irep_idt, term_position>
get_function_occurrences_aux(const exprt& expr, const term_position& root) {
    std::multimap<irep_idt, term_position> result_map;

    if (expr.id() == ID_function_application) {

        function_application_exprt app = to_function_application_expr(expr);
        result_map.insert({app.id(), root});
        for (size_t i = 0; i < app.operands().size(); ++i) {
            result_map.merge(get_function_occurrences_aux(app.operands()[i], root.append_node(i)));
        }
    }
    return result_map;
}

std::multimap<irep_idt, term_position>
    get_function_occurrences(const problemt& problem) {
    std::multimap<irep_idt, term_position> result_map;
    for (auto x : problem.assertions) {
        result_map.merge(get_function_occurrences_aux(x, term_position()));
    }

    return result_map;
}

exprt get_sub_term_at_position(term_position pos, exprt& term) {
    if (pos.empty()) {
        return term;
    }

    std::vector<exprt>::size_type first = pos.front();
    if (first >= term.operands().size()) {
        throw term_pos_not_exist("Position does not exist in term.");
    }
    return get_sub_term_at_position(pos.pop_front(), term.operands()[first]);
}

std::string to_string(const term_position& tp) {
    std::string s = "[";
    for (size_t i = 0; i < tp.position.size(); ++i) {
        s +=i;
        if (i < tp.position.size()-1) {
            s += "|";
        }
    }
    s+="]";
    return s;
}