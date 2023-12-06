//
// Created by julian on 04.04.23.
//

#include "smt_problem.h"

#include <string>

#include "parsing/parser.h"
// #include "printing_utils.h"

#include <fstream>
#include <iostream>

#include <util/cmdline.h>
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


smt_problemt parse_problem(const std::string &filename) {
    // parse input file
    std::ifstream in(filename);

    if (!in) {
        std::cerr << "Failed to open input file" << std::endl;
        throw std::exception();
    }

    console_message_handlert message_handler;
    messaget message(message_handler);

    // this is our default verbosity
    unsigned int v = messaget::M_STATISTICS;

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
        throw std::exception();
    }

    smt_problemt result;
    result.logic = parser.logic;
    result.filename = filename;
    for (const auto &a: parser.assertions) {
        result.assertions.push_back(a);
    }

    // identify the free variables and defined functions in the problem
    for (const auto &id: parser.id_map) {
        if (id.second.definition.is_nil() && id.second.kind == smt2_parsert::idt::VARIABLE) {
            result.free_var[symbol_exprt(id.first, id.second.type)] = nil_exprt();
        } else {
            result.defined_functions[symbol_exprt(id.first, id.second.type)] = id.second.definition;
        }
    }
    return result;
}

smt_problemt negate_problem(const smt_problemt &problem) {
    smt_problemt new_problem;

    new_problem.logic = problem.logic;
    new_problem.defined_functions = problem.defined_functions;
    new_problem.filename = problem.filename;
    new_problem.free_var = problem.free_var;

    new_problem.assertions.push_back(not_exprt(conjunction(problem.assertions)));

    return new_problem;
}