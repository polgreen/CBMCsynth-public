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

    for (auto& term : terms) {
        //substitution = unification(term, lgg);
        exprt tmp = second_order_var;
        exprt t = term;
        //tmp = substitute(second_order_var, tmp);
        // replace term with tmp in smt_problem
    }

    // create sygus from smt_problem where lgg is solution

    return {};
}


std::vector<sygus_problemt> create_training_data(const std::vector<std::string>& files) {
    return {};
}