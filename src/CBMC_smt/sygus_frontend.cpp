#include "sygus_frontend.h"
#include "sygus_problem.h"
#include "cvc5_synth.h"
#include "util.h"
#include "parser.h"
#include "printing_utils.h"
#include <util/format_expr.h>
#include <util/std_expr.h>
#include <util/cout_message.h>

int test_cvc5(const cmdlinet &cmdline)
{
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
  message_handler.set_verbosity(v);

  // construct problem
  sygus_problemt problem;
  problem.logic = "LIA";
  problem.free_var.push_back(symbol_exprt("x", integer_typet()));
  problem.free_var.push_back(symbol_exprt("y", integer_typet()));
  // construct synth fun command
  synth_fun_commandt synth_fun;
  synth_fun.id = "f";
  std::vector<typet> domain;
  domain.push_back(integer_typet());
  domain.push_back(integer_typet());
  synth_fun.type = mathematical_function_typet(domain, integer_typet());
  std::vector<irep_idt> params;
  params.push_back("a");
  params.push_back("b");
  synth_fun.parameters = params;
  problem.synthesis_functions.push_back(synth_fun);
  // add constraints
  std::vector<exprt> inputs;
  inputs.push_back(symbol_exprt("x", integer_typet()));
  inputs.push_back(symbol_exprt("y", integer_typet()));
  problem.constraints.push_back(
      equal_exprt(create_func_app("f", inputs, integer_typet()),
                  plus_exprt(inputs[0], inputs[1])));
  cvc5_syntht cvc5_synth(message.get_message_handler());

  if (cvc5_synth(problem) == decision_proceduret::resultt::D_SATISFIABLE)
  {
    message.status() << "Synthesis succeeded" << messaget::eom;
    for (const auto &f : cvc5_synth.get_solution())
    {
      message.status() << format(f.first) << " = " << format(f.second) << messaget::eom;
    }
    return 0;
  }
  else
  {
    message.status() << "Synthesis failed" << messaget::eom;
    return 1;
  }
  return 0;
}

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
  // parse the problem
  try
  {
    parser.parse();
  }
  catch (const parsert::smt2_errort &e)
  {
    message.error() << e.get_line_no() << ": "
                    << e.what() << messaget::eom;
    return 20;
  }

  auto problem = parser.get_sygus_problem();

  // pre-processing
  if (cmdline.isset("add-default-grammar"))
  {
    for (auto &f : problem.synthesis_functions)
    {
      add_grammar(f);
    }
    // replace all grammars in the sygus problem with default
  }

  // do stuff
  if (cmdline.isset("dump-problem-as-smt"))
  {
    // dump the sygus problem as an smt problem
    print_sygus_as_smt(problem, message.status());
  }
  if (cmdline.isset("dump-problem"))
  {
    print_sygus_problem(problem, message.status());
    // just print the problem
  }
  if (cmdline.isset("solve-sygus"))
  {
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

  return 0;
}