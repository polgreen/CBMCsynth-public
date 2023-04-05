//
// Created by julian on 21.03.23.
//

#include "create_training_data.h"


#include "sygus_problem.h"
#include <string>
#include <vector>

#include "problem.h"
#include "term_position.h"
#include "anti_unification.h"
#include "unification.h"

#include <replace_symbol.h>
#include <replace_expr.h>
/**
 * This method is most important for getting "good" training data. It is worth investigating which
 * hyper-parameters impact the data in what way.
 * suggestions : sub-tree similarity
 **/
std::vector<term_position> get_term_positions(const problemt& problem) {
    return {};
}

sygus_problemt create_training_data(const std::string& file) {

    problemt smt_problem = parse_problem(file);
    std::vector<term_position> positions = get_term_positions(smt_problem);

    std::vector<exprt> terms;
    terms.reserve(positions.size());
    for (auto& pos : positions) {
        terms.push_back(get_term_copy_at_position_in_problem(pos, smt_problem));
    }

    exprt lgg = compute_lgg(terms);
    exprt second_order_var;  // this is a term like :    Foo(t1,...,tn) where n >= number of variables gathered from lgg;

    replace_mapt replace_map;

    for (auto& term : terms) {
        std::vector<std::pair<exprt, exprt>> to_unify;
        to_unify.emplace_back(term, lgg);
        auto x = unify(to_unify);
        if (!x) {
            throw std::exception(); // This should not occur, since they should always be unifiable (by construction of lgg)
        }
        replace_symbolt substitution = x.value();
        exprt tmp = second_order_var;
        substitution(tmp);
        // replace term with tmp in smt_problem
        replace_map.insert({term, tmp});
    }


    sygus_problemt sygus_problem;
    sygus_problem.logic = smt_problem.logic;
    sygus_problem.defined_functions = smt_problem.defined_functions;

    for (auto assertion : smt_problem.assertions) {
        replace_expr(replace_map, assertion);
        sygus_problem.assertions.push_back(assertion);
    }

    // create sygus from smt_problem where lgg is solution

    return sygus_problem;
}


std::vector<sygus_problemt> create_training_data(const std::vector<std::string>& files) {
    std::vector<sygus_problemt> res;
    for (auto& x : files) {
        res.push_back(create_training_data(x));
    }

    return res;
}