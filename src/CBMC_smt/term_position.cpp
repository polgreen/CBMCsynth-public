//
// Created by julian on 15.03.23.
//

#include "term_position.h"
#include "problem.h"

#include <iostream>

#include <util/cmdline.h>
#include <util/namespace.h>
#include <util/symbol_table.h>
#include <util/expr.h>
#include "util.h"


term_positiont operator+(term_positiont &first, const term_positiont &second) {
    first.position.insert(first.position.end(), second.position.begin(), second.position.end());
    return first;
}


void map_to_string(const std::multimap<irep_idt, term_positiont> &mp, std::ostream &message) {
    for (const auto &x: mp) {
        message << x.first << ": " << to_string(x.second) << std::endl;
    }
}


std::multimap<irep_idt, term_positiont>
get_function_occurrences_aux(const exprt &expr, const term_positiont &root) {
    std::multimap<irep_idt, term_positiont> result_map;

    if (!expr.operands().empty()) {
        result_map.insert({expr.id(), root});
        for (size_t i = 0; i < expr.operands().size(); ++i) {
            result_map.merge(get_function_occurrences_aux(expr.operands()[i], root.append_node(i)));
        }
    }
    return result_map;
}


std::multimap<irep_idt, term_positiont>
get_function_occurrences(const problemt &problem) {
    std::multimap<irep_idt, term_positiont> result_map;

    for (size_t i = 0; i < problem.assertions.size(); ++i) {
        result_map.merge(get_function_occurrences_aux(problem.assertions[i], term_positiont(i)));
    }

    return result_map;
}


exprt get_term_copy_at_position(term_positiont pos, const exprt &term) {
    if (pos.empty()) {
        return term;
    }

    std::vector<exprt>::size_type first = pos.front();
    if (first >= term.operands().size()) {
        throw term_pos_not_exist("Position does not exist in term.");
    }
    return get_term_copy_at_position(pos.pop_front(), term.operands()[first]);
}


exprt get_term_copy_at_position_in_problem(const term_positiont &pos, const problemt &prob) {
    return get_term_copy_at_position(pos, prob.assertions[pos.assertion]);
}


std::string to_string(const term_positiont &tp) {
    std::string s = "[";
    s = s + std::to_string(tp.assertion);
    s = s + ">";
    for (size_t i = 0; i < tp.position.size(); ++i) {
        s += std::to_string(tp.position[i]);
        if (i < tp.position.size() - 1) {
            s += "|";
        }
    }
    s = s + "]";
    return s;
}

bool is_overlapping(const term_positiont &pos1, const term_positiont &pos2) {
    if (pos1.assertion != pos2.assertion) {
        return false;
    }
    size_t least_length = std::min(pos1.position.size(), pos2.position.size());

    for (int i = 0; i < least_length; ++i) {
        if (pos1.position[i] != pos2.position[i]) {
            return false;
        }
    }
    return true;

}


std::vector<term_positiont> get_pos_of_all_occurrences(const exprt &what, const exprt &in, const term_positiont &pos) {
    if (what == in) {
        return {pos};
    } else if (!in.has_operands()) {
        return {};
    } else {
        std::vector<term_positiont> res;
        for (auto i = 0; i < in.operands().size(); ++i) {
            auto recurr =
                    get_pos_of_all_occurrences(what, in.operands()[i], pos.append_node(i));
            concat(res, recurr);
        }
        return res;
    }
}