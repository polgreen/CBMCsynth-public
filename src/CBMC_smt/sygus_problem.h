//
// Created by julian on 21.03.23.
//

#ifndef SRC_SYGUS_PROBLEM_H
#define SRC_SYGUS_PROBLEM_H

#include <string>
#include <vector>
#include <util/expr.h>
#include <util/std_expr.h>
#include <util/symbol.h>
#include <util/mathematical_types.h>
#include <map>

struct syntactic_templatet {
    irep_idt start;
    typet start_type;
    std::vector<irep_idt> nt_ids;
    std::map<irep_idt, std::vector<exprt>> production_rules;
    std::map<irep_idt, std::vector<unsigned>> production_rule_weights;
};

class synth_funt {

public:
    irep_idt id;
    typet type;
    std::vector<irep_idt> parameters;
    syntactic_templatet grammar;
};

class sygus_problemt {

public:

    std::vector<std::string> comments;
    // assume we only have one synthesis function for enumeration, 
    // but keep this as a vector so we can parse multiple
    std::vector<synth_funt> synthesis_functions;

    std::string filename;
    std::vector<exprt> constraints;
    std::vector<exprt> assumptions;
    std::map<symbol_exprt, exprt> defined_functions;
    std::vector<symbol_exprt> free_var;

    // only used for invariant problems
    irep_idt inv_id, pre_id, trans_id, post_id;
    std::string logic;
    std::vector<exprt> nnf_constraints() const;
    syntactic_templatet get_grammar() const;

};

synth_funt copy_fun_add_grammar(const synth_funt &f);
void add_grammar(synth_funt &f);
void add_grammar_weights(syntactic_templatet &g);

class solutiont
{
  public:
  std::map<symbol_exprt, exprt> functions;
};

class counterexamplet
{
public:
  std::unordered_map<exprt, exprt, irep_hash> assignment;
  void clear() { assignment.clear(); }
};


#endif //SRC_SYGUS_PROBLEM_H
