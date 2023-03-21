//
// Created by julian on 21.03.23.
//

#ifndef SRC_ANTI_UNIFICATION_H
#define SRC_ANTI_UNIFICATION_H

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


/**
 * We compute the least general generalization (lgg) of a set of terms.
 * */
exprt compute_lgg(const std::vector<exprt>& terms);



#endif //SRC_ANTI_UNIFICATION_H