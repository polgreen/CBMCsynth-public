//
// Created by julian on 21.03.23.
//

#ifndef SRC_CREATE_TRAINING_DATA_H
#define SRC_CREATE_TRAINING_DATA_H


#include "sygus_problem.h"
#include <string>
#include <vector>

std::vector<sygus_problemt> create_training_data(const std::vector<std::string>& files);

#endif //SRC_CREATE_TRAINING_DATA_H
