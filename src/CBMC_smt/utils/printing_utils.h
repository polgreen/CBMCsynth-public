#ifndef __PRINTING_UTILS_H__
#define __PRINTING_UTILS_H__

#include "../smt_problem.h"
#include "../sygus_problem.h"
#include <util/namespace.h>
#include <fstream>

// General printing utils

// Prints a SyGuS problem using CBMC pretty printer. Good for debugging
void print_problem(const sygus_problemt &problem, std::ostream &out);

// Prints SMT problem in SMT format
void print_smt_problem(const smt_problemt &problem, std::ostream &out);

// Prints any model for an SMT problem
// the model is stored in the free_var assignments in smt_problemt
void print_model(smt_problemt &problem, std::ostream &out);

// Traverses an exprt and pretty prints each level to a stream
// only use for debugging
void traverse_expression(exprt &expr, std::ostream &out);

// prints a sygus problemt in sygus format
void print_sygus_problem(const sygus_problemt &problem, std::ostream &out);

// converts a sygus problemt to an SMT problem, representing 
// the synth-fun as an uninterpreted function, and prints the SMT-lib to a stream
void print_sygus_as_smt(const sygus_problemt &problem, std::ostream &out);

// Not finished. Start of a function to print a SyGuS problem as python
void print_sygus_as_python(const sygus_problemt &problem, std::ostream &out, namespacet &ns);

#endif