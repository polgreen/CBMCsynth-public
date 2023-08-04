//
// Created by julian on 21.03.23.
//

#include "create_training_data.h"

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <util/replace_symbol.h>
#include <util/replace_expr.h>
#include <util/cmdline.h>
#include <util/cout_message.h>
#include <util/format_expr.h>
#include <util/namespace.h>
#include <util/symbol_table.h>
#include <util/std_expr.h>
#include <util/expr.h>
#include <util/expr_iterator.h>
#include <solvers/smt2/smt2_dec.h>


#include "smt_problem.h"
#include "term_position.h"
#include "anti_unification.h"
#include "unification.h"
#include "constants.h"
#include "progressbar.hpp"
#include "sygus_problem.h"
#include "smt2_frontend.h"
#include "parser.h"
#include "util.h"
#include "subterm_heuristics.h"
#include "expr2sygus.h"


std::optional<sygus_problemt> create_training_data(const smt_problemt &smt_problem, const namespacet &namespacet) {

    std::vector<term_positiont> positions = get_term_positions(smt_problem);
    if (positions.empty()) {
        return std::nullopt;
    }
    std::vector<exprt> terms;
    terms.reserve(positions.size());
    for (auto &pos: positions) {
        terms.push_back(get_term_copy_at_position_in_problem(pos, smt_problem));
    }

    std::pair<exprt, std::vector<symbol_exprt>> anti_uni = compute_lgg(terms);

    exprt lgg = anti_uni.first;
    std::vector<exprt> new_vars;
    new_vars.reserve(anti_uni.second.size());
    for (const auto &x: anti_uni.second) {
        new_vars.emplace_back(x);
    }

    // this is a term like :    Foo(t1,...,tn) where n >= number of variables gathered from lgg;
    // exprt second_order_var("synthTarget", lgg.type(), std::vector<exprt>(new_vars));
    function_application_exprt synth_fun_app =
            create_func_app("synthTarget", std::vector<exprt>(new_vars), lgg.type());

    // generate the replace map
    replace_mapt replace_map;
    for (auto &term: terms) {
        std::vector<std::pair<exprt, exprt>> to_unify;
        to_unify.emplace_back(lgg, term);// The order of arguments here is important due to the ordering in the if-else-if branches in unify. Note that this should be solved at some point with variable ordering. But until then let's keep it this way.
        replace_symbolt substitution = unify(to_unify).value();
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
    for (const auto &x: anti_uni.second) {
        synth_fun.parameters.push_back(x.get_identifier());
    }

    sygus_problem.synthesis_functions.push_back(synth_fun);

    if (smt_problem.logic.rfind("QF_",0) == 0) { // check if it starts with
        sygus_problem.logic = smt_problem.logic.substr(3);
    } else {
        sygus_problem.logic = smt_problem.logic;
    }
    sygus_problem.defined_functions = smt_problem.defined_functions;
    sygus_problem.filename = smt_problem.filename;

    for (const auto &x: smt_problem.free_var) {
        sygus_problem.free_var.push_back(x.first);
    }

    // replace in assertions
    for (auto assertion: smt_problem.assertions) {
        replace_expr(replace_map, assertion);

        if (simplify_expr(assertion,namespacet).is_true()) {
            continue;
        }
        sygus_problem.constraints.push_back(assertion);
    }

    return sygus_problem;
}


std::optional<sygus_problemt> create_synthesis_problem(const std::string &file, messaget& message) {
    // parse input file

    std::ifstream in(file);

    if (!in) {
        std::cerr << "Failed to open input file" << std::endl;
        throw default_exception("Failed to open file " + file);
    }

    in.ignore( std::numeric_limits<std::streamsize>::max() );
    std::streamsize length = in.gcount();
    in.clear();   //  Since ignore will have set eof.
    in.seekg( 0, std::ios_base::beg );
    if (length > MAX_FILE_SIZE) { // dirty hack to discard files that are too large and would lead to seg faults etc.
        return std::nullopt;
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
        std::cout << "Could not parse: " << file << std::endl;
        return std::nullopt;
        //throw default_exception("Could not parse.");
    }

    smt_problemt smt_problem = parser.get_smt_problem();
    smt_problem.filename = file;
    for (auto &x: smt_problem.assertions) {
        expand_let_expressions(x);

    }

    decision_proceduret::resultt res = solve_problem(smt_problem, ns, message);

    smt_problemt new_valid_problem;
    if (res == decision_proceduret::resultt::D_SATISFIABLE) {

        // check for validity
        smt_problemt neg_problem = negate_problem(smt_problem);
        decision_proceduret::resultt new_res = solve_problem(neg_problem, ns, message);

        if (new_res == decision_proceduret::resultt::D_UNSATISFIABLE) { // original problem is valid
            new_valid_problem = smt_problem;
        } else if (new_res == decision_proceduret::resultt::D_ERROR) {
            throw solver_timeout("SMT solver error.");
        } else {
            // replace the free variables in the assertions with the values from the model
            new_valid_problem = substitute_model_into_problem(smt_problem);
            // print the new smt_problem
            message.debug() << "Problem is satisfiable, working with substitution" << messaget::eom;
        }
    } else if (res == decision_proceduret::resultt::D_UNSATISFIABLE) {
        message.debug() << "Problem is UNSAT, working with negation." << messaget::eom;
        new_valid_problem = negate_problem(smt_problem);
    } else if (res == decision_proceduret::resultt::D_ERROR) {
        throw solver_timeout("SMT solver error.");
    }

    auto sygus_problem_opt = create_training_data(new_valid_problem, ns);

    return sygus_problem_opt;
}

void create_and_write_problem(const std::string& file, const std::string& output, messaget& message) {
    try {
        auto s_prob = create_synthesis_problem(file, message);
        if (!s_prob) {
            message.debug() << "couldn't handle " + file << messaget::eom;
        } else {

            std::filesystem::path file_path(output);
            std::filesystem::create_directories(file_path.parent_path()); // create intermediate dirs.
            std::cout << "Writing result to: " << file_path << std::endl;

            std::ofstream myfile(file_path);
            myfile << build_sygus_query(s_prob.value());
            myfile.close();
        }
    } catch (solver_timeout& e) {
        return;
    }
}

int create_synthesis_problem(const cmdlinet &cmdline) {

    std::string input = cmdline.args[0];
    std::string output_file = cmdline.args[1];

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


    create_and_write_problem(input, output_file, message);


    return 0;
}