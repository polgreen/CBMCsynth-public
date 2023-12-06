/*******************************************************************\
 Module: CBMC SMT stuff
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

/// File ending of SMT2 files. Used to determine the language frontend that
/// shall be used.

/// File ending of Sygus files. Used to determine the language frontend that
/// shall be used.
#define SYGUS_FILE_ENDING ".sl"
#define SMT2_FILE_ENDING ".smt2"



void help(std::ostream &out)
{
  out << "\n"
         "* *      A library of SyGuS and SMT manipulating utils         * *\n";

  out << "* *                                                            * *\n"
         "\n"
         "Usage:                           Purpose:\n"
         "\n"
         " CBMC_smt [-?] [-h] [--help]       show help\n"
         " CBMC_smt file.smt2 ...              source file names\n"
         "\n"
         "\n"
         "Command line options\n"
         " --verbosity N                   increase verbosity (10 gives maximum verbosity)\n"
         "\n"
         "The following options are applicable to SyGuS problems:\n"
         "--cegis                          attempt to do CEGIS with the basic random top-down enumerator\n"
         "--add-default-grammar            replace all synthesis function grammars with default\n"
         "--dump-problem                   print problem in SyGuS-IF format\n"
         "--dump-problem-as-smt            use quantifiers to print a sygus problem in SMT-LIB\n"
         "--solve-with-cvc5                call CVC5 to solve the SyGuS problem\n"
         "\n"
         "\n"
         "The following options are applicable to SMT problems:\n"
         "--solve-smt                      solve the SMT problem with Z3\n"
         "--dump-problem                   print problem in SMT-lib format\n"
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
