//
// Created by julian on 05.04.23.
//

#include "unification.h"

#include "find_symbols.h"
#include "std_expr.h"

bool occurs_check(const exprt& symbol, const exprt& term) {
    if (symbol.id() != ID_symbol) { // make sure symbol really is a symbol
        throw std::exception();
    }
    return has_symbol_expr(term, symbol.id(), false);
}

std::optional<replace_symbolt> unify(std::vector<std::pair<exprt, exprt>>& problem)
{
    if (problem.empty()) {
        return std::make_optional<replace_symbolt>({});
    }
    std::pair<exprt, exprt> back = problem.back();
    problem.pop_back();
    exprt& fst = back.first;
    exprt& snd = back.second;

    if (fst == snd) {   // delete rule
        return unify(problem);
    }
    else if (fst.id() == snd.id()) { // decompose rule
        for (int i = 0 ; i < fst.operands().size(); ++i) {
            problem.emplace_back(fst.operands()[i], snd.operands()[i]);
        }
        return unify(problem);
    } else if (fst.id_string() != snd.id_string()) { // conflict   TODO make sure that this is checking root function symbol
        return std::nullopt;
    } else if (fst.id() == ID_symbol) { // eliminate  TODO make sure that this is checking that it's a variable
        if (occurs_check(fst, snd)) { // failed occurs check
            return std::nullopt;
        }
        replace_symbolt substitution;
        substitution.insert(to_symbol_expr(fst), snd);
        for (auto& term : problem) {
            substitution(term.first);
            substitution(term.second);
        }
        auto sol = unify(problem);
        if (sol) {
            sol->insert(to_symbol_expr(fst), snd);
        }
        return sol;
    } else if (snd.id() == ID_symbol) { // eliminate  TODO make sure that this is checking that it's a variable
        if (occurs_check(snd, fst)) {
            return std::nullopt;
        }
        replace_symbolt substitution;
        substitution.insert(to_symbol_expr(snd), fst);
        for (auto& term : problem) {
            substitution(term.first);
            substitution(term.second);
        }
        auto sol = unify(problem);
        if (sol) {
            sol->insert(to_symbol_expr(snd), fst);
        }
        return sol;
    } else {
        return std::nullopt;
    }
}