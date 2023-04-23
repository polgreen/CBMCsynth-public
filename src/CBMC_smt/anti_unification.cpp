//
// Created by julian on 21.03.23.
//

#include "anti_unification.h"
#include <util/replace_expr.h>


#include "uuid_v4.h"
UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
UUIDv4::UUID uuid = uuidGenerator.getUUID();


// Plotkin Anti-Unification
exprt compute_lgg(const std::vector<exprt>& terms) {
    if (terms.empty()) {
        throw std::exception(); // TODO logging
    } // if terms.size() == 1 ?

    auto operand = terms[0].get(ID_identifier);

    // check if all the terms have the same root symbol
    for (const exprt& t : terms) {
        if (t.get(ID_identifier) != operand) { // Not the same root symbol
            std::string bytes = uuid.bytes();
            return symbol_exprt("var_" + bytes, t.type()); // return fresh variable
        }
    }

    replace_mapt substs;

    size_t n_op_args = terms[0].operands().size();
    exprt new_sub_lgg{terms[0]}; // create lgg with the same root symbol as other terms

    // solve all lgg sub problems (recurse into term)
    for (size_t i = 0; i < n_op_args; ++i) {
        std::vector<exprt> sub_problem;
        for (const auto& term: terms) {
            sub_problem.emplace_back(term.operands()[i]); // Segmentation fault
        }
        exprt op = new_sub_lgg.operands()[i];
        substs.insert({op, compute_lgg(sub_problem)});
    }

    auto flag = replace_expr(substs, new_sub_lgg);
    if (not flag) {
        throw std::exception(); // TODO logging
    }
    return new_sub_lgg;
}