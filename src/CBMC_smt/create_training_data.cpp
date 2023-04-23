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

#include <util/replace_symbol.h>
#include <util/replace_expr.h>


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

#include "term_position.h"
#include "problem.h"

#include "smt2_frontend.h"

/**
 * This method is most important for getting "good" training data. It is worth investigating which
 * hyper-parameters impact the data in what way.
 * suggestions : sub-tree similarity
 **/
std::vector<term_position> get_term_positions(const problemt& problem) {
    term_position p1;
    p1.assertion = 0;
    term_position p2;
    p2.assertion = 1;
    return {p1,p2};
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



int create_synthesis_problems(const cmdlinet &cmdline) {
    // parse input file
    assert(cmdline.args.size() == 1);
    std::ifstream in(cmdline.args.front());

    if (!in) {
        std::cerr << "Failed to open input file" << std::endl;
        return 10;
    }

    console_message_handlert message_handler;
    messaget message(message_handler);

    // this is our default verbosity
    unsigned int v = messaget::M_STATISTICS;

    if (cmdline.isset("verbosity")) {
        v = std::stol(
                cmdline.get_value("verbosity"));;
        if (v > 10) {
            v = 10;
        }
    }

    symbol_tablet symbol_table;
    namespacet ns(symbol_table);

    message_handler.set_verbosity(v);
    parsert parser(in);
    // parse the problem
    try {
        parser.parse();
    } catch (const parsert::smt2_errort &e) {
        message.error() << e.get_line_no() << ": "
                        << e.what() << messaget::eom;
        return 20;
    }

    problemt problem = build_problem(parser);
    decision_proceduret::resultt res = solve_problem(problem, ns, message);
    // print problem and model
    message.debug() << "Solving with SMT solver:" << messaget::eom;
    print_model(problem, message.status());
    message.status() << messaget::eom;

    if (res == decision_proceduret::resultt::D_SATISFIABLE) {
        // replace the free variables in the assertions with the values from the model
        problemt new_valid_problem = substitute_model_into_problem(problem);
        // print the new problem
        message.status() << "NEW PROBLEM" << messaget::eom;
        print_problem(new_valid_problem, message.status());
        message.status() << messaget::eom;

    } else if (res == decision_proceduret::resultt::D_UNSATISFIABLE) {
        //problemt new_valid_problem = negate_problem(problem);

    }

    message.status()<<"\n\nRunning traverse_expression"<< messaget::eom;
    traverse_expression(problem.assertions[0], message.status());


    message.status()<<"\n\nPrinting Function Positions"<< messaget::eom;
    std::multimap<irep_idt, term_position> positions = get_function_occurrences(problem);
    for (auto x : positions) {
        message.status() << x.first << ": " << to_string(x.second) << messaget::eom;
    }
    message.status() << messaget::eom; // flush

    return 1;
}