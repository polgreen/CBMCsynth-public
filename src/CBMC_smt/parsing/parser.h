#ifndef PARSER_CPP
#define PARSER_CPP

#include <solvers/smt2/smt2_parser.h>
#include "../sygus_problem.h"
#include "../smt_problem.h"

#include <util/mathematical_expr.h>
#include <util/mathematical_types.h>

class parsert : public smt2_parsert {
public:
    explicit parsert(std::istream &_in) : smt2_parsert(_in) {
        setup_commands();
    }

    exprt::operandst assertions;
    std::string logic;
    exprt::operandst constraints;
    exprt::operandst assumptions;

    // store synth funcs
    std::map<irep_idt, synth_funt> synthesis_functions;
    // might contain information from where the problem was originally
    std::vector<std::string> set_info_cmds;

    void print_problem();
    void parse_model();

    using smt2_errort = smt2_tokenizert::smt2_errort;

    void expand_function_applications(exprt &expr);

    smt_problemt get_smt_problem();
    sygus_problemt get_sygus_problem();

protected:
    void setup_commands();
    void add_synth_fun_id(irep_idt id, 
    const smt2_parsert::signature_with_parameter_idst &sig, const syntactic_templatet& grammar);
    void build_smt_problem();
    void build_sygus_problem();
    syntactic_templatet parse_grammar();
    symbol_exprt NTDef();
    std::vector<exprt> GTerm_seq(const symbol_exprt &nonterminal);
    smt_problemt smt_problem;
    sygus_problemt sygus_problem;
};

#endif