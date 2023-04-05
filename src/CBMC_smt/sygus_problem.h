//
// Created by julian on 21.03.23.
//

#ifndef SRC_SYGUS_PROBLEM_H
#define SRC_SYGUS_PROBLEM_H

#include <string>
#include <vector>
#include <expr.h>
#include <symbol.h>
#include <map>

class synth_fun_command {

};

class sygus_problemt {

public:

    std::string comments;

    synth_fun_command synth_fun;

    std::string filename;
    std::vector<exprt> assertions;
    std::map<symbol_exprt, exprt> defined_functions;
    std::string logic;

};


#endif //SRC_SYGUS_PROBLEM_H
