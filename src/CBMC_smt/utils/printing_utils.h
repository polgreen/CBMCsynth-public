#ifndef __PRINTING_UTILS_H__
#define __PRINTING_UTILS_H__

#include "../smt_problem.h"
#include "../sygus_problem.h"
#include <fstream>

void print_smt_problem(const smt_problemt &problem, std::ostream &out);

void print_model(smt_problemt &problem, std::ostream &out);

void traverse_expression(exprt &expr, std::ostream &out);

void print_problem_tree(const smt_problemt &problem, std::ostream &out);

void print_sygus_problem(const sygus_problemt &problem, std::ostream &out);
void print_sygus_as_smt(const sygus_problemt &problem, std::ostream &out);


#endif