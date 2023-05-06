//
// Created by julian on 06.05.23.
//

#ifndef CBMCSMT_SUBTERM_HEURISTICS_H
#define CBMCSMT_SUBTERM_HEURISTICS_H


#include <vector>
#include "term_position.h"
#include "problem.h"


/**
 * This method is most important for getting "good" training data. It is worth investigating which
 * hyper-parameters impact the data in what way.
 * suggestions : sub-tree similarity
 **/
std::vector<term_positiont> get_term_positions(const problemt &problem);


#endif //CBMCSMT_SUBTERM_HEURISTICS_H
