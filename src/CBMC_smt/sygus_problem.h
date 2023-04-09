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

struct syntactic_templatet
{
    std::vector<irep_idt> nt_ids;
    std::map<irep_idt, std::vector<exprt>> production_rules;
};

class synth_fun_commandt {

public:
    irep_idt id;
    typet type;
    std::vector<irep_idt> parameters;
    syntactic_templatet grammar;

    // synth_fun_commandt(
    //   const irep_idt &_id,  
    //   const syntactic_templatet &_grammar,
    //   const typet &_type,
    //   const std::vector<irep_idt> &_parameters)
    //   : id(_id), type(_type), parameters(_parameters),grammar(_grammar)
    // {
    //   PRECONDITION(
    //     (_type.id() == ID_mathematical_function &&
    //      to_mathematical_function_type(_type).domain().size() ==
    //        _parameters.size()) ||
    //     (_type.id() != ID_mathematical_function && _parameters.empty()));
    // }
};

class sygus_problemt {

public:

    std::string comments;

    synth_fun_commandt synth_fun;

    std::string filename;
    std::vector<exprt> assertions;
    std::map<symbol_exprt, exprt> defined_functions;
    std::vector<symbol_exprt> free_var;

    std::string logic;

};


#endif //SRC_SYGUS_PROBLEM_H
