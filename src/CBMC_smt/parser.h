#ifndef PARSER_CPP
#define PARSER_CPP

#include <solvers/smt2/smt2_parser.h>

#include <util/mathematical_expr.h>
#include <util/mathematical_types.h>

class parsert: public smt2_parsert
{
  public: 
    explicit parsert(std::istream &_in): smt2_parsert(_in) 
    {
        setup_commands();
    }

  exprt::operandst assertions;
  std::string logic;
  void print_problem();
  using smt2_errort = smt2_tokenizert::smt2_errort;
  void expand_function_applications(exprt &expr);

  protected:
    void setup_commands();
};

#endif