//
// Created by julian on 21.03.23.
//

#ifndef SRC_CREATE_TRAINING_DATA_H
#define SRC_CREATE_TRAINING_DATA_H


#include "sygus_problem.h"
#include "cmdline.h"
#include <string>
#include <vector>



sygus_problemt create_training_data(const std::string& file);

std::vector<sygus_problemt> create_training_data(const std::vector<std::string>& files);


int create_synthesis_problems(const cmdlinet &cmdline);

#endif //SRC_CREATE_TRAINING_DATA_H
