//
// Created by julian on 15.03.23.
//

#ifndef SRC_TERM_POSITION_H
#define SRC_TERM_POSITION_H

#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <string>
#include "problem.h"
#include <deque>
#include <exception>

#include <util/cmdline.h>
#include <util/cout_message.h>
#include <util/format_expr.h>
#include <util/namespace.h>
#include <util/replace_symbol.h>
#include <util/simplify_expr.h>
#include <util/symbol_table.h>
#include <util/std_expr.h>
#include <util/expr.h>

#include <sstream>

struct term_position;
std::string to_string(const term_position& tp);

struct term_position {

    std::deque<int> position;

    // in which assertion
    size_t assertion;

    term_position() = default;
    term_position(size_t assertion) : assertion(assertion) {}

    term_position(const term_position& t):
        position{t.position},
        assertion{t.assertion}
        { }

    bool empty() {return  position.size() == 0;}

    int front() {return *position.begin();}

    term_position pop_front() {
        position.pop_front();
        return *this;
    }

    term_position append_node(int i) const {
        term_position t(*this);
        t.position.push_back(i);
        return t;
    }
};

class term_pos_not_exist : std::exception {
    std::string msg;
public:
    term_pos_not_exist(std::string msg): msg{msg} {}
};

term_position operator+(term_position& first, const term_position& second);

// collect occuring functions and their respective positions
std::multimap<irep_idt, term_position> get_function_occurrences(const problemt& problem);

/*
 * Return copy of subterm of term at position pos.
 * */
exprt get_term_copy_at_position(term_position pos, const exprt& term);

/*
 * Return copy of subterm of term at position pos.
 * */
exprt get_term_copy_at_position_in_problem(term_position pos, problemt& term);


#endif //SRC_TERM_POSITION_H
