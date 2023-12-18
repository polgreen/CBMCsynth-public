#ifndef BU_SYNTH_H_
#define BU_SYNTH_H_

#include "synth.h"
#include "../verification/mini_verify.h"
#include "../sygus_problem.h"
#include <solvers/decision_procedure.h>
#include <util/namespace.h>
#include <util/message.h>
#include <random>
#include <set>

// a top down enumerator that randomly enumerates a grammar
// NB: this only handles sygus_problems with a single synthesis function
class bottom_up_syntht : public syntht
{
public:
  bottom_up_syntht(message_handlert &_ms, sygus_problemt &_problem, mini_verifyt &_cex_verifier) : 
  message_handler(_ms),
                                                            problem(_problem),
                                                            cex_verifier(_cex_verifier),
                                                            grammar(_problem.get_grammar()){
                                                              create_distributions();
                                                              current_pool = &program_pool1;
                                                              prev_pool = &program_pool2;
                                                            };

  solutiont get_solution() const override;

  // initialises the top down enumeration wtih the start symbol of the grammar
  // then calls replace_nts to enumerate through the grammar
  void bottom_up_enumerate();

  // filters the current pool of programs based on observational equivalence 
  // and simplification
  void filter_pool();

  // set maximum depth of enumeration
  void set_program_size(std::size_t size) override;

  // calls the enumerator and then checks the result against the counterexamples
  // returns when it has found a candidate that satisfies the counterexamples
  resultt operator()() override;

  // adds a counterexample to the list
  void add_counterexample(const counterexamplet &cex) override;

protected:
  // used for printing. TODO: make all the printing use the message handlers correctly
  message_handlert &message_handler;
  // the problem to solve
  sygus_problemt &problem;
  // verifies candidates against the counterexamples
  mini_verifyt &cex_verifier;
  // the grammar to enumerate
  // we make a copy of this rather than using the one in the SyGuS problem
  // so we can change it if we need
  // TODO: make this a reference to the grammar in the sygus problem
  syntactic_templatet grammar;
  // counterexamples
  std::vector<counterexamplet> counterexamples;
  // for each nonterminal, the map contains a set of vectors of integers.
  // each vector of integers is a list of outputs obtained
  // by executing a candidate program on the vector of counterexamples.
  // we use this to check if we are adding candidates taht are observationally
  // equivalent to the current pool.
  std::map<irep_idt,std::set<std::vector<int>>> counterexample_results;
  // last solution we found
  solutiont last_solution;
  // namespacet ns;

  bool get_next_programs();
  bool initialise_program_pool();
  void empty_current_pool();
  std::set<exprt> replace_one_nt(const exprt &expr, const syntactic_templatet &grammar, std::set<exprt> &new_exprs);

  
  // we have two program pools, 
  // we use program_pool1 as the current pool on odd iterations (e.g., 1, 3)
  // and program_pool2 as the current pool on even iterations
  // the other pool is always the pool from the previous iteration
  std::map<irep_idt, std::set<exprt>> program_pool1;
  std::map<irep_idt, std::set<exprt>> program_pool2;
  // we keep the pool of new potential solutions here so we can check
  // them, without checking every program in the pool
  std::set<exprt> latest_solution_pool;

  std::map<irep_idt, std::set<exprt>> *current_pool;
  std::map<irep_idt, std::set<exprt>> *prev_pool;
  std::size_t iteration;
 
  // depth we enumerate to
  std::size_t program_size;

  bool is_observationally_unique(const exprt &expr, const irep_idt &nt);
  // program inputs and outputs
  std::set<std::pair<std::vector<constant_exprt>, constant_exprt>> program_io;

  bool verify_against_counterexamples(const exprt & expr);

  // creates the distributions based on non-terminal weights
  void create_distributions();

  // the distributions for each non-terminal
  // irep_idt is the nonterminal name, the distribution has a weight for each
  // production rule for that nonterminal.
  std::map<irep_idt, std::discrete_distribution<int>> distributions;

};

#endif /* BU_SYNTH_H_ */
