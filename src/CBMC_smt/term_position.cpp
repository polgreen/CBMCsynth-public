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

void map_to_string(std::multimap<irep_idt, term_position> mp, std::ostream& message) {
    for (auto x : mp) {
        message << x.first << ": " << to_string(x.second) << std::endl;
    }
}

std::multimap<irep_idt, term_position>
get_function_occurrences_aux(const exprt& expr, const term_position& root) {
    std::multimap<irep_idt, term_position> result_map;

    if (!expr.operands().empty()) {
        result_map.insert({expr.id(), root});
        for (size_t i = 0; i < expr.operands().size(); ++i) {
            result_map.merge(get_function_occurrences_aux(expr.operands()[i], root.append_node(i)));
        }
    }
    return result_map;
}

std::multimap<irep_idt, term_position>
    get_function_occurrences(const problemt& problem) {
    std::multimap<irep_idt, term_position> result_map;

    for (size_t i = 0; i < problem.assertions.size(); ++i) {
        result_map.merge(get_function_occurrences_aux(problem.assertions[i], term_position(i)));
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
    s = s + std::to_string(tp.assertion);
    s = s + ">";
    for (size_t i = 0; i < tp.position.size(); ++i) {
        s = s + std::to_string(tp.position[i]);
        if (i < tp.position.size()-1) {
            s = s + "|";
        }
    }
    s = s + "]";
    return s;
}