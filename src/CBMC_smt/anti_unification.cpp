//
// Created by julian on 21.03.23.
//

#include "anti_unification.h"
#include <util/replace_expr.h>

exprt compute_lgg(const std::vector<exprt>& terms) {
    if (terms.empty()) {
        throw std::exception(); // TODO logging
    } // if terms.size() == 1 ?

    auto operand = terms[0].get(ID_identifier);
    for (const exprt& t : terms) {
        if (t.get(ID_identifier) != operand) {
            return {}; // fresh variable
        }
    }
    // all terms have the same operand

    replace_mapt substs;

    size_t n_op_args = terms[0].operands().size();
    exprt new_term{terms[0]};

    for (size_t i = 0; i < n_op_args; ++i) {
        std::vector<exprt> sub_problem;
        for (const auto& term: terms) {
            sub_problem.push_back(term.operands()[i]);
        }
        substs.insert({new_term.operands()[i], compute_lgg(sub_problem)});
    }

    auto flag = replace_expr(substs, new_term);
    if (not flag) {
        throw std::exception(); // TODO logging
    }
    return new_term;
}