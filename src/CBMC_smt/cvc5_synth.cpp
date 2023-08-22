#include "cvc5_synth.h"
#include "utils/expr2sygus.h"
#include "parsing/parser.h"

#include <fstream>
#include <solvers/decision_procedure.h>
#include <util/tempfile.h>
#include <util/run.h>


decision_proceduret::resultt cvc5_syntht::operator()(const sygus_problemt &problem)
{
  return solve(problem);
}


decision_proceduret::resultt cvc5_syntht::read_result(std::istream &in, const sygus_problemt &problem)
{
  if (!in)
  {
    std::cout << "Failed to open input file";
    return decision_proceduret::resultt::D_ERROR;
  }

  parsert result_parser(in);

  try
  {
    result_parser.parse_model();
  }
  catch (const parsert::smt2_errort &e)
  {
    std::cout << e.get_line_no() << ": "
              << e.what() << std::endl;
    return decision_proceduret::resultt::D_ERROR;
  }
  if (result_parser.id_map.size() == 0)
    return decision_proceduret::resultt::D_ERROR;

  for (auto &id : result_parser.id_map)
  {
    for(const auto &synth_fun: problem.synthesis_functions)
    {
    if(id.first==synth_fun.id)
    {
      if (id.second.type.id() == ID_mathematical_function)
      {
        symbol_exprt symbol = symbol_exprt(id.second.type);
        symbol.set_identifier(id.first);
        clean_symbols(id.second.definition);
        last_solution[symbol] = id.second.definition;
      }
      else
      {
        symbol_exprt symbol = symbol_exprt(mathematical_function_typet({}, id.second.type));
        symbol.set_identifier(id.first);
        clean_symbols(id.second.definition);
        last_solution[symbol] = id.second.definition;
      }
    }
    }
  }
  return decision_proceduret::resultt::D_SATISFIABLE;
}

decision_proceduret::resultt cvc5_syntht::solve(const sygus_problemt &problem)
{
  const std::string query = build_sygus_query(problem);
  temporary_filet
      temp_file_problem("sygus_problem_", ""),
      temp_file_stdout("sygus_stdout_", ""),
      temp_file_stderr("sygus_stderr_", "");
  {
    std::ofstream problem_out(
        temp_file_problem(), std::ios_base::out | std::ios_base::trunc);
    problem_out << query;
  }

  std::vector<std::string> argv;
  std::string stdin_filename;
  argv = {"cvc5", "--lang", "sygus2", temp_file_problem()};

  int res =
      run(argv[0], argv, stdin_filename, temp_file_stdout(), temp_file_stderr());
  if (res < 0)
    return decision_proceduret::resultt::D_ERROR;
  else
  {
    
    std::ifstream in(temp_file_stdout());
    return read_result(in, problem);
  }
}

exprt cvc5_syntht::model(exprt expr) const
{
  assert(expr.id()==ID_symbol);
  auto iter = last_solution.find(to_symbol_expr(expr));
  assert(iter!=last_solution.end());
  return iter->second;
}

std::map<symbol_exprt, exprt> cvc5_syntht::get_solution() const
{
  return last_solution;
}

