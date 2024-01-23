#ifndef __SYNTACTIC_FEEDBACK_H_
#define __SYNTACTIC_FEEDBACK_H_

#include "../sygus_problem.h"
#include <util/message.h>
#include "../parsing/parser.h"

// give syntactic feedback partway through enumeration
class syntactic_feedbackt
{
  public:
    syntactic_feedbackt(sygus_problemt &problem, message_handlert &ms) 
        : problem(problem), message(ms), iter(0)
    {
      last_solution = nil_exprt();
    }

    bool partial_evaluation(const exprt &expr, const counterexamplet &cex);
    std::size_t augment_grammar(const exprt &partial_function, sygus_problemt &problem);
    std::string build_prompt(const exprt &partial_function);
    std::string build_smt_prompt(const exprt &partial_function);
    bool update_grammar;
    bool use_cex_in_prompt;
    bool expand_fun_apps;
    exprt last_solution;
    counterexamplet last_cex;



  private: 
    sygus_problemt &problem;
    messaget message;
    std::size_t iter;
    bool add_to_grammar(const irep_idt &id, const exprt &expr);
    void update_grammar_weights(parsert& parser);
    std::size_t add_functions(const parsert & parser, sygus_problemt& problem);

};



#endif /*__SYNTACTIC_FEEDBACK_H_*/