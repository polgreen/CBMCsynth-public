#include "printing_utils.h"
#include "problem.h"
#include "expr2sygus.h"

#include <util/format_expr.h>
#include <util/std_expr.h>
#include <util/expr.h>


// print the expression tree for each assertion
void print_problem(problemt &problem, std::ostream &out) {
    out << "Logic: " << problem.logic << std::endl;
    out << "Problem:" << std::endl;
    int count = 0;

    for (const auto &a: problem.defined_functions) {
        out << "Function: " << a.first.get_identifier();
        out << "  " << format(a.second) << std::endl;
    }

    for (const auto &a: problem.assertions) {
        out << "Assertion " << count << ": ";
        count++;
        out << format(a) << std::endl;
    }
    out << "Free variables: " << std::endl;
    for (const auto &e: problem.free_var) {
        out << e.first.get_identifier() << std::endl;
    }
}

void test_sygus_printing(problemt &problem, std::ostream &out)
{
    out << "Testing SyGuS printing: " << std::endl;

    for (const auto &e: problem.free_var) {
        out << var_dec(e.first)<<std::endl;
    }

    for (const auto &a: problem.defined_functions) {
        out << fun_def(a.first, a.second);
    }

    for (const auto &a: problem.assertions) {
        out << "(assert ";
        out << expr2sygus(a) << std::endl;
        out <<")\n";
    }


}

void print_model(problemt &problem, std::ostream &out) {
    out << "Model:" << std::endl;
    for (const auto &e: problem.free_var) {
        out << "Free var : " << e.first.get_identifier();
        if (!e.second.is_nil())
            out << " = " << format(e.second) << std::endl;
        else
            out << "[no assignment]" << std::endl;
    }
}

void traverse_expression(exprt &expr, std::ostream &out) {
    out << expr.id() << std::endl;
    for (auto op: expr.operands()) {
        out << "looking at expression: " << op.pretty() << std::endl;
        if (op.id() == ID_symbol) {
            out << "Found symbol: " << op.get(ID_identifier) << std::endl;
        }
        traverse_expression(op, out);
    }
}

void print_problem_tree(problemt &problem, std::ostream &out) {
    out << "Problem tree:" << std::endl;
    int count = 0;
    for (const auto &a: problem.assertions) {
        out << "Assertion " << count << std::endl;
        count++;
        out << a.pretty() << std::endl;
    }
}