#ifndef __PRINTING_UTILS_H__
#define __PRINTING_UTILS_H__

#include "problem.h"
#include <fstream>

void print_problem(problemt &problem, std::ostream &out);
void print_model(problemt &problem, std::ostream &out);
void traverse_expression(exprt &expr, std::ostream &out);
void print_problem_tree(problemt &problem, std::ostream &out);

#endif