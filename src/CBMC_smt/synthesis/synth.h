#ifndef SYNTH_H_
#define SYNTH_H_

#include "../sygus_problem.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>
#include <random>

class syntht
{
public:
  using resultt = enum { CANDIDATE,
                         NO_SOLUTION };
  syntht(message_handlert &_ms, sygus_problemt &_problem) : message_handler(_ms),
                                                            problem(_problem),
                                                            grammar(_problem.get_grammar()){
                                                              create_distributions();
                                                            };

  solutiont get_solution() const;
  void top_down_enumerate();
  void set_program_size(std::size_t size);

protected:
  std::vector<counterexamplet> counterexamples;
  solutiont last_solution;
  message_handlert &message_handler;
  sygus_problemt &problem;
  syntactic_templatet grammar;
  std::size_t program_size;
  bool contains_nonterminal(const exprt &expr);
  bool replace_nts(exprt &expr, std::size_t &current_depth);
  void create_distributions();
  std::map<irep_idt, std::discrete_distribution<int>> distributions;
  // set of previous solutions
  
};

#endif /* SYNTH_H_ */
