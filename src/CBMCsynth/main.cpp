/*******************************************************************\
 Module: Basic synthesizer, and some utils for manipulating SyGUs and SMT files
 Author: Elizabeth Polgreen, epolgreen@gmail.com.
\*******************************************************************/

#include <util/cmdline.h>
#include <util/suffix.h>

#include "smt2_frontend.h"
#include "sygus_frontend.h"
#include "constants.h"
#include <random>

#include <iostream>

#define OPTIONS         \
  "(verbosity): "       \
  "(add-default-grammar)" \
  "(dump-problem)" \
  "(dump-problem-as-smt)" \
  "(solve-with-cvc5)" \
  "(cegis)" \
  "(cegis-bu)" \
  "(cegis-prob-bu)" \
  "(probs): " \
  "(top-down-cegis) " \
  "(use-bonus-weights)" \
  "(update-grammar)" \
  "(use-cex-in-prompt)" \
  "(do-not-expand-fun-apps)" \
  "(program-depth): " \
  "(frequency-of-LLM-calls): " \
  "(astar)"                 \
  "(use-LLM)" \
  "(program-depth):" \
  "(temperature):" \



/// File ending of SMT2 files. Used to determine the language frontend that
/// shall be used.

/// File ending of Sygus files. Used to determine the language frontend that
/// shall be used.
#define SYGUS_FILE_ENDING ".sl"
#define SMT2_FILE_ENDING ".smt2"



void help(std::ostream &out)
{
  out << "\n"
         "* *               A basic synthesiser and a                * *\n"
        "* *     library of SyGuS and SMT manipulating utils         * *\n";

  out << "* *                                                            * *\n"
         "\n"
         "Usage:                           Purpose:\n"
         "\n"
         " CBMCsynth [-?] [-h] [--help]       show help\n"
         " CBMCsynth file.smt2 ...              source file names\n"
         "\n"
         "\n"
         "Command line options\n"
         " --verbosity N                   increase verbosity (10 gives maximum verbosity)\n"
         "\n"
         "The following options select the basic algorithm used for solving a CEGIS problem:\n"
         "--cegis                          CEGIS with the most basic random top-down enumerator\n"
         "                                 If no weights are given for the grammar, equal weights are used.\n"
         "--astar                          CEGIS with the a* enumerator. If the --use-LLM option is\n" 
         "                                 also specified, this CEGIS supports integrated calls to LLM\n"
         "                                 If no weights are given for the grammar, equal weights are used.\n"
         "--top-down-cegis                      CEGIS with the top-down random enumerator. If the --use-LLM option is\n" 
         "                                 also specified, this CEGIS supports integrated calls to LLM\n"
         
         "\nThe following options are configurations for the LLM calls\n"
         "--use-cex-in-prompt              use the most recent solution and counterexample in the prompt\n"
         "--do-not-expand-fun-apps         do not expand function applications in the prompt\n"
         "--temperature                    set the temperature for openai calls\n"

         "\nThe following options are configurations for the CEGIS algorithm\n"    
         "--update-grammar                 uses LLM feedback to update the grammar weights\n"
         "--add-default-grammar            replace all synthesis function grammars with default\n"
         "--program-depth                  limit the maximum depth we enumerate in top-down CEGIS (default 5) \n"
         "--use-LLM                        make calls to the LLM during synthesis\n"
         "--probs F                        give a file with weights for each production rule. The weights must be integers,\n"
         "                                 one integer per line, in the order that the NTs and rules appear in the grammar\n"
         "\n"
         "\n"

        "The following options are useful printing and debugging options\n"
         "--dump-problem                   print problem in SyGuS-IF format\n"
         "--dump-problem-as-smt            use quantifiers to print a sygus problem in SMT-LIB\n"
         "--solve-with-cvc5                call CVC5 to solve the SyGuS problem\n"
         
        //  "The following options are debugging optoin to SMT problems:\n"
        //  "--solve-smt                      solve the SMT problem with Z3\n"
        //  "--dump-problem                   print the SMT problem in SMT-lib format\n"
         "\n"
         "\n";
}

int main(int argc, const char *argv[])
{
  cmdlinet cmdline;

  if (cmdline.parse(argc, argv, OPTIONS))
  {
    std::cerr << "Usage error\n";
    help(std::cerr);
    return 1;
  }

  if (cmdline.isset("help") || cmdline.isset("h") || cmdline.isset("?") || cmdline.args.empty())
  {
    help(std::cout);
    return 1;
  }
  try
  {
    // all other options need an input file
    if (has_suffix(cmdline.args.back(), SYGUS_FILE_ENDING))
    {
      // handle all sygus options
      return sygus_frontend(cmdline);
    }
    else if (has_suffix(cmdline.args.back(), SMT2_FILE_ENDING))
    {
      // handle all other smt2 options
      return smt2_frontend(cmdline);
    }
    else
    {
      std::cerr << "Unknown file type \n";
    }
  }
  catch (const char *s)
  {
    std::cerr << "Error: " << s << '\n';
  }
  catch (const std::string &s)
  {
    std::cerr << "Error: " << s << '\n';
  }
}
