//
// Created by julian on 05.04.23.
//

#ifndef CBMCSMT_UNIFICATION_H
#define CBMCSMT_UNIFICATION_H


#include <util/replace_symbol.h>
#include <optional>

/**
 * Syntactic unificaiton of two terms
 **/
std::optional<replace_symbolt> unify(std::vector<std::pair<exprt, exprt>> &problem);


#endif //CBMCSMT_UNIFICATION_H
