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
#include "util.h"

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

#include "expr2sygus.h"
#include <util/expr_iterator.h>

/**
 * This method is most important for getting "good" training data. It is worth investigating which
 * hyper-parameters impact the data in what way.
 * suggestions : sub-tree similarity
 **/
std::vector<term_positiont> get_term_positions(const problemt& problem) {

    int lgg_max_height = 100;

    std::vector<exprt> all_subterms;
    for (const exprt& assertion : problem.assertions) {
        for(auto it = assertion.depth_begin() , itend = assertion.depth_end(); it != itend; ++it) {
            all_subterms.push_back(*it);
        }
    }

    std::size_t height = 0;
    exprt fst;
    exprt snd;

    for(int i = 0; i < all_subterms.size(); ++i) {
        const auto& x = all_subterms[i];
        for (int j = i; j < all_subterms.size(); ++j ){
            const auto& y = all_subterms[j];

            if (x.type() != y.type()) {
                continue;
            }

            if (x.type().id()!=ID_integer) {
                continue;
            }

            if (is_subterm(x, y) or is_subterm(y, x)) {
                continue;
            }

            auto lgg = compute_lgg({{x,y}});
            if (!is_binder_free(lgg.first)) {
                continue;
            }
            auto lgg_height = expr_height(lgg.first);
            if (lgg_height > height and x != y and lgg_height<= lgg_max_height) {
                fst = x;
                snd = y;
                height = expr_height(lgg.first);
            }
        }
    }

    std::vector<exprt> terms{fst, snd};
    for (const auto& term : all_subterms) {
        if (term.type() != fst.type()) {
            continue;
        }
        if (!is_binder_free(term)) {
            continue;
        }

        terms.push_back(term);
        auto lgg = compute_lgg(terms);
        if (expr_height(lgg.first) < height) {
            terms.pop_back();
        }
    }


    std::vector<term_positiont> res;

    for (auto i = 0 ; i < problem.assertions.size(); ++i) {
        for (const auto& x : terms){
            concat(res,get_pos_of_all_occurrences(x, problem.assertions[i], term_positiont(i)));
        }
    }


    return res;
}


std::optional<sygus_problemt> create_training_data(const problemt &smt_problem, const namespacet& namespacet) {

    std::vector<term_positiont> positions = get_term_positions(smt_problem);
    if (positions.empty()) {
        return std::nullopt;
    }
    std::vector<exprt> terms;
    terms.reserve(positions.size());
    for (auto& pos : positions) {
        terms.push_back(get_term_copy_at_position_in_problem(pos, smt_problem));
    }

    std::pair<exprt, std::vector<symbol_exprt>> anti_uni = compute_lgg(terms);

    exprt lgg = anti_uni.first;
    std::vector<exprt> new_vars;
    for (const auto& x : anti_uni.second) {
        new_vars.emplace_back(x);
    }

    // this is a term like :    Foo(t1,...,tn) where n >= number of variables gathered from lgg;
   // exprt second_order_var("synthTarget", lgg.type(), std::vector<exprt>(new_vars));
    function_application_exprt synth_fun_app = 
        create_func_app("synthTarget",std::vector<exprt>(new_vars), lgg.type());

    // generate the replace map
    replace_mapt replace_map;
    for (auto& term : terms) {
        std::vector<std::pair<exprt, exprt>> to_unify;
        to_unify.emplace_back(lgg, term);// The order of arguments here is important due to the ordering in the if-else-if branches in unify. Note that this should be solved at some point with variable ordering. But until then let's keep it this way.
        replace_symbolt substitution =  unify(to_unify).value();
        exprt tmp = synth_fun_app;
        substitution(tmp);
        // replace term with tmp in smt_problem
        replace_map.insert({term, tmp});
    }

    sygus_problemt sygus_problem;

    sygus_problem.comments = smt_problem.comments;

    std::stringstream ss;
    ss << "Solution: " << format(lgg);
    sygus_problem.comments.push_back(ss.str());

    synth_fun_commandt synth_fun;
    synth_fun.id = to_symbol_expr(synth_fun_app.function()).get_identifier();
    synth_fun.type = synth_fun_app.function_type();
    // synth_fun.parameters = irep_args;
    for (const auto& x: anti_uni.second){
        synth_fun.parameters.push_back(x.get_identifier());
    }

    sygus_problem.synth_fun = synth_fun;

    sygus_problem.logic = smt_problem.logic;
    sygus_problem.defined_functions = smt_problem.defined_functions;
    sygus_problem.filename = smt_problem.filename;

    for (const auto& x : smt_problem.free_var) {
        sygus_problem.free_var.push_back(x.first);
    }

    // replace in assertions
    for (auto assertion : smt_problem.assertions) {
        replace_expr(replace_map, assertion);
        //if (simplify_expr(assertion,namespacet).is_true()) {
        //    continue;
        //}
        sygus_problem.assertions.push_back(assertion);
    }

    return sygus_problem;
}


std::optional<sygus_problemt> create_synthesis_problem(const std::string& file, messaget message) {
    // parse input file

    std::ifstream in(file);

    if (!in) {
        std::cerr << "Failed to open input file" << std::endl;
        throw default_exception("Failed to open file " + file);
    }

    symbol_tablet symbol_table;
    namespacet ns(symbol_table);

    parsert parser(in);
    // parse the smt_problem
    try {
        parser.parse();
    } catch (const parsert::smt2_errort &e) {
        message.error() << e.get_line_no() << ": "
                        << e.what() << messaget::eom;
        throw default_exception("Could not parse.");
    }

    problemt smt_problem = build_problem(parser);
    smt_problem.filename = file;
    for (auto& x : smt_problem.assertions) {
        expand_let_expressions(x);
    }


    decision_proceduret::resultt res = solve_problem(smt_problem, ns, message);
    // print smt_problem and model
    //message.debug() << "Solving with SMT solver:" << messaget::eom;
    //print_model(smt_problem, message.status());
    //message.status() << messaget::eom;

    problemt new_valid_problem;
    if (res == decision_proceduret::resultt::D_SATISFIABLE) {
        // replace the free variables in the assertions with the values from the model
        new_valid_problem = substitute_model_into_problem(smt_problem);
        // print the new smt_problem
        message.status() << "Problem is satisfiable, working with substitution" << messaget::eom;
        //print_problem(new_valid_problem, message.status());
        //message.status() << messaget::eom;

    } else if (res == decision_proceduret::resultt::D_UNSATISFIABLE) {
        message.status() << "Problem is UNSAT, working with negation." << messaget::eom;
        new_valid_problem = negate_problem(smt_problem);
    } else if (res == decision_proceduret::resultt::D_ERROR) {
        throw default_exception("SMT solver error.");
    }

    auto sygus_problem_opt = create_training_data(new_valid_problem, ns);

    //message.status() << "--------------------------------------------------\n" << messaget::eom;
    //message.status() << build_sygus_query(sygus_problem) << messaget::eom;
    //message.status() << "--------------------------------------------------\n" << messaget::eom;


    //message.status() << messaget::eom; // flush

    return sygus_problem_opt;
}

int create_synthesis_problem(const cmdlinet &cmdline) {

    std::string dir = cmdline.args[0];
    std::vector<std::string> all_files = files_with_suffix_in_dirs({dir}, SMT2_FILE_ENDING);


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
    message_handler.set_verbosity(v);

    for (auto& file : all_files) {
        auto s_prob = create_synthesis_problem(file, message);

        if (s_prob) {
            std::cout << (*s_prob).filename << std::endl;
        } else {
            std::cout << "couldn't handle " + file << std::endl;
        }
    }
    //build_sygus_query(s_prob)

    return 0;
}