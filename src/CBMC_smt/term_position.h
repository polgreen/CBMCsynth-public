//
// Created by julian on 15.03.23.
//

#ifndef SRC_TERM_POSITION_H
#define SRC_TERM_POSITION_H

#include <iostream>
#include <utility>
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

struct term_positiont;
std::string to_string(const term_positiont& tp);

struct term_positiont {

    std::deque<int> position;

    // in which assertion
    size_t assertion;

    //term_positiont() = default;
    explicit term_positiont(size_t assertion) : assertion(assertion) {}

    term_positiont(const term_positiont& t) = default;

    [[nodiscard]] bool empty() const {return  position.empty();}

    int front() {return *position.begin();}

    term_positiont pop_front() {
        position.pop_front();
        return *this;
    }

    [[nodiscard]] term_positiont append_node(int i) const {
        term_positiont t(*this);
        t.position.push_back(i);
        return t;
    }
};

class term_pos_not_exist : std::exception {
    std::string msg;
public:
    explicit term_pos_not_exist(std::string msg): msg{std::move(msg)} {}
};

term_positiont operator+(term_positiont& first, const term_positiont& second);

// collect occuring functions and their respective positions
std::multimap<irep_idt, term_positiont> get_function_occurrences(const problemt& problem);

/*
 * Return copy of subterm of term at position pos.
 * */
exprt get_term_copy_at_position(term_positiont pos, const exprt& term);

/*
 * Return copy of subterm of term at position pos.
 * */
exprt get_term_copy_at_position_in_problem(const term_positiont& pos, const problemt& term);


std::vector<term_positiont> get_pos_of_all_occurrences(const exprt& what, const exprt& in, const term_positiont& pos);


#endif //SRC_TERM_POSITION_H
