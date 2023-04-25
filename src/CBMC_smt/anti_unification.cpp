//
// Created by julian on 21.03.23.
//

#include "anti_unification.h"
#include <util/replace_expr.h>

#include "util.h"

#include "uuid_v4.h"
UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;



// Plotkin Anti-Unification
std::pair<exprt, std::vector<symbol_exprt>>  compute_lgg(const std::vector<exprt>& terms) {
    if (terms.empty()) {
        throw std::exception(); // TODO logging
    } // if terms.size() == 1 ?

    auto operand = terms[0].id();

    // check if all the terms have the same root symbol
    for (const exprt& t : terms) {
        if (!root_equality(terms[0], t)) { // Not the same root symbol
            std::string bytes = std::to_string(uuidGenerator.getUUID().hash());
            auto new_var = symbol_exprt("var_" + bytes, t.type()); // return fresh variable
            return {new_var, {new_var}};
        }
    }

    replace_mapt substs;
    size_t n_op_args = terms[0].operands().size();
    exprt new_sub_lgg{terms[0]}; // create lgg with the same root symbol as other terms
    std::vector<symbol_exprt> new_vars;
    // solve all lgg sub problems (recurse into term)
    for (size_t i = 0; i < n_op_args; ++i) {
        std::vector<exprt> sub_problem;
        for (const auto& term: terms) {
            sub_problem.emplace_back(term.operands()[i]); // Segmentation fault
        }
        exprt op = new_sub_lgg.operands()[i];
        std::pair<exprt, std::vector<symbol_exprt>> res = compute_lgg(sub_problem);
        concat(new_vars, res.second);
        substs.insert({op, res.first});
    }

    replace_expr(substs, new_sub_lgg);

    return {new_sub_lgg, new_vars};
}