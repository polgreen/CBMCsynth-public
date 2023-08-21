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

class synth_funt {

public:
    irep_idt id;
    typet type;
    std::vector<irep_idt> parameters;
    syntactic_templatet grammar;
};

class sygus_problemt {

public:

    std::vector<std::string> comments;

    std::vector<synth_funt> synthesis_functions;

    std::string filename;
    std::vector<exprt> constraints;
    std::vector<exprt> assumptions;
    std::map<symbol_exprt, exprt> defined_functions;
    std::vector<symbol_exprt> free_var;

    std::string logic;

};

synth_funt copy_fun_add_grammar(const synth_funt &f);
void add_grammar(synth_funt &f);


#endif //SRC_SYGUS_PROBLEM_H
