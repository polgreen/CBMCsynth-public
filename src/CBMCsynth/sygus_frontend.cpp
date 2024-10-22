#include "sygus_frontend.h"
#include "sygus_problem.h"
#include "utils/cvc5_synth.h"
#include "utils/util.h"
#include "parsing/parser.h"
#include "utils/printing_utils.h"
#include <util/format_expr.h>
#include <util/std_expr.h>
#include <util/cout_message.h>

#include "synthesis/a_star.h"
#include "synthesis/synth.h"
#include "synthesis/synth_td.h"
#include "synthesis/synth_LLM.h"
#include "synthesis/synth_bu.h"
#include "synthesis/synth_prob_bu.h"
#include "verification/verify.h"
#include "verification/counterexample_verify.h"
#include "cegis.h"

int sygus_frontend(const cmdlinet &cmdline)
{
  // parse problem and get sygus problem
  console_message_handlert message_handler;
  messaget message(message_handler);

  // this is our default verbosity
  unsigned int v = messaget::M_STATISTICS;

  if (cmdline.isset("verbosity"))
  {
    v = std::stol(
        cmdline.get_value("verbosity"));
    ;
    if (v > 10)
    {
      v = 10;
    }
  }

  // parse input file
  assert(cmdline.args.size() == 1);
  std::ifstream in(cmdline.args.front());

  if (!in)
  {
    message.error() << "Failed to open input file" << messaget::eom;
    return 10;
  }

  symbol_tablet symbol_table;
  namespacet ns(symbol_table);

  message_handler.set_verbosity(v);
  parsert parser(in);
  sygus_problemt problem;
  // parse the problem
  try
  {
    parser.parse();
    problem = parser.get_sygus_problem();
  }
  catch (const parsert::smt2_errort &e)
  {
    message.error() << e.get_line_no() << ": "
                    << e.what() << messaget::eom;
    return 20;
  }

  if (cmdline.isset("probs"))
  {
    try
    {
      message.status() << "Parsing probabilities file" << messaget::eom;
      std::string filename = cmdline.get_value("probs");
      parse_probs(filename, problem);

    }
    catch (const std::exception &e)
    {
      message.error() << "Failed to parse probabilities file" << messaget::eom;
      return 30;
    }
  }

  // pre-processing
  if (cmdline.isset("add-default-grammar"))
  {
    for (auto &f : problem.synthesis_functions)
    {
      message.debug() << "adding default grammars to sygus problems" << messaget::eom;
      add_grammar(f);
    }
    // replace all grammars in the sygus problem with default
  }
  // do stuff
  if (cmdline.isset("dump-problem-as-smt"))
  {
    // dump the sygus problem as an smt problem
    message.debug() << "Dumping problem as smt\n *************************\n"
                    << messaget::eom;
    print_sygus_as_smt(problem, message.status());
    message.status() << messaget::eom; // flush
  }
  if (cmdline.isset("dump-problem"))
  {
    message.debug() << "Dumping problem\n *************************\n " << messaget::eom;
    print_sygus_problem(problem, message.status());
    message.status() << messaget::eom; // flush
    // just print the problem
  }
  if (cmdline.isset("solve-with-cvc5"))
  {
    message.status() << "Solving sygus problem with CVC5" << messaget::eom;
    // we solve with cvc5
    cvc5_syntht cvc5_synth(message.get_message_handler());
    decision_proceduret::resultt res = cvc5_synth(problem);
    // print problem and model
    if (res == decision_proceduret::resultt::D_SATISFIABLE)
    {
      message.status() << "Synthesis succeeded" << messaget::eom;
      for (const auto &f : cvc5_synth.get_solution())
      {
        message.status() << format(f.first) << " = " << format(f.second) << messaget::eom;
      }
      message.status() << messaget::eom; // flush
      return 0;
    }
    else
    {
      message.status() << "Synthesis failed" << messaget::eom;
      message.status() << messaget::eom; // flush
      return 1;
    }
  }
  if (cmdline.isset("cegis"))
  {
    message.status() << "top down CEGIS" << messaget::eom;
    counterexample_verifyt cex_verify(ns, message_handler);
    top_down_syntht synth(message_handler, problem, cex_verify);
    if (cmdline.isset("program-depth"))
      synth.set_program_size(std::stoi(cmdline.get_value("program-depth")));
    else
      synth.set_program_size(5);
    synth.use_syntactic_feedback = false;
    verifyt verify(ns, message_handler);
    cegist cegis(synth, verify, problem, ns, message_handler);
    cegis.doit();
    return 0;
  }
  if (cmdline.isset("top-down-cegis"))
  {
    message.status() << "top down CEGIS with an LLM providing syntactic feedback" << messaget::eom;
    counterexample_verifyt cex_verify(ns, message_handler);
    llm_syntht synth(message_handler, problem, cex_verify);
    if (cmdline.isset("program-depth"))
      synth.set_program_size(std::stoi(cmdline.get_value("program-depth")));
    else
      synth.set_program_size(5);

    if (cmdline.isset("temperature"))
      synth.set_temperature(std::stod(cmdline.get_value("temperature")));
    else
      synth.set_temperature(0.5);

    synth.set_feedback_parameters(cmdline.isset("use-LLM"),
                                  cmdline.isset("update-grammar"),
                                  cmdline.isset("use-bonus-weights"),
                                  cmdline.isset("use-cex-in-prompt"),
                                  !cmdline.isset("do-not-expand-fun-apps"));
    if (cmdline.isset("frequency-of-LLM-calls"))
      synth.frequency_of_LLM_calls = std::stoi(cmdline.get_value("frequency-of-LLM-calls"));
    verifyt verify(ns, message_handler);
    cegist cegis(synth, verify, problem, ns, message_handler);
    cegis.doit();
    return 0;
  }
  else if (cmdline.isset("cegis-bu"))
  {
    message.status() << "Basic bottom up CEGIS" << messaget::eom;
    counterexample_verifyt cex_verify(ns, message_handler);
    bottom_up_syntht synth(message_handler, problem, cex_verify);
    synth.set_program_size(5);
    verifyt verify(ns, message_handler);
    cegist cegis(synth, verify, problem, ns, message_handler);
    cegis.doit();
  }
  else if (cmdline.isset("cegis-prob-bu"))
  {
    message.status() << "probabilistic bottom up CEGIS" << messaget::eom;
    counterexample_verifyt cex_verify(ns, message_handler);
    prob_bu_syntht synth(message_handler, problem, cex_verify);
    synth.set_program_size(5);
    verifyt verify(ns, message_handler);
    cegist cegis(synth, verify, problem, ns, message_handler);
    cegis.doit();
  }
  else if(cmdline.isset("astar"))
  {
    message.status() << "A star search" << messaget::eom;
    counterexample_verifyt cex_verify(ns, message_handler);
    a_star_syntht synth(message_handler, problem, cex_verify);
    synth.set_program_size(5);
    synth.set_feedback_parameters(cmdline.isset("use-LLM"),
                                  cmdline.isset("update-grammar"),
                                  cmdline.isset("use-cex-in-prompt"),
                                  !cmdline.isset("do-not-expand-fun-apps"));
                                
    if (cmdline.isset("temperature"))
      synth.set_temperature(std::stod(cmdline.get_value("temperature")));
    else
      synth.set_temperature(0.5);
    verifyt verify(ns, message_handler);
    cegist cegis(synth, verify, problem, ns, message_handler);
    cegis.doit();
  }

  return 0;
}