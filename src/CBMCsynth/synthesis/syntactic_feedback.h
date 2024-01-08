#ifndef __SYNTACTIC_FEEDBACK_H_
#define __SYNTACTIC_FEEDBACK_H_

#include "../sygus_problem.h"
#include <util/message.h>

// give syntactic feedback partway through enumeration
class syntactic_feedbackt
{
  public:
    syntactic_feedbackt(sygus_problemt &problem, const syntactic_templatet &grammar, message_handlert &ms) 
        : problem(problem), grammar(grammar), message(ms)
    {
    // TODO Auto-generated constructor stub
    }

    bool partial_evaluation(const exprt &expr, const counterexamplet &cex);
    std::size_t augment_grammar(const exprt &partial_function, sygus_problemt &problem);
    std::string build_prompt(const exprt &partial_function);
    std::string build_smt_prompt(const exprt &partial_function);


  private: 
    sygus_problemt problem;
    syntactic_templatet grammar;
    messaget message;

};



#endif /*__SYNTACTIC_FEEDBACK_H_*/