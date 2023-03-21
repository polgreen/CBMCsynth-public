//
// Created by julian on 21.03.23.
//

#include "sygus_problem.h"
#include "problem.h"


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

#include <solvers/smt2/smt2_dec.h>
#include "smt2_frontend.h"
#include "term_position.h"



sygus_problemt create_training_data(problemt &smt_problem, namespacet &ns, messaget &log) {

    decision_proceduret::resultt res = solve_problem(smt_problem, ns, log);

    if (res == decision_proceduret::resultt::D_SATISFIABLE) {
        // replace the free variables in the assertions with the values from the model
        problemt new_valid_problem = substitute_model_into_problem(smt_problem);


    } else if (res == decision_proceduret::resultt::D_UNSATISFIABLE) {
        //problemt new_valid_problem = negate_problem(problem);

    }

    return sygus_problemt();
}


std::vector<sygus_problemt> create_training_data(std::vector<std::string> files) {
    std::vector<sygus_problemt> res;
    for (auto file : files){
        //parsing
        // auto x = create_training_data(problem);
        // res.push_back(x);
    }
    return res;
}
