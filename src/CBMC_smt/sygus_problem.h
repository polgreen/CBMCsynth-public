//
// Created by julian on 21.03.23.
//

#ifndef SRC_SYGUS_PROBLEM_H
#define SRC_SYGUS_PROBLEM_H

#include <string>
#include <vector>
#include <util/expr.h>
#include <util/symbol.h>
#include <util/mathematical_types.h>
#include <map>

struct syntactic_templatet {
    std::vector<irep_idt> nt_ids;
    std::map<irep_idt, std::vector<exprt>> production_rules;
};

class synth_fun_commandt {

public:
    irep_idt id;
    typet type;
    std::vector<irep_idt> parameters;
    syntactic_templatet grammar;
};

class sygus_problemt {

public:

    std::vector<std::string> comments;

    synth_fun_commandt synth_fun;

    std::string filename;
    std::vector<exprt> assertions;
    std::map<symbol_exprt, exprt> defined_functions;
    std::vector<symbol_exprt> free_var;

    std::string logic;

};

synth_fun_commandt add_grammar(const synth_fun_commandt &f);


#endif //SRC_SYGUS_PROBLEM_H
