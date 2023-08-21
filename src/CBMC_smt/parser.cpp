#include "parser.h"
#include <util/replace_symbol.h>
#include <iostream>

std::vector<exprt> parsert::GTerm_seq(const symbol_exprt &nonterminal)
{
  irep_idt id;
  std::vector<exprt> rules;

  if(smt2_tokenizer.next_token()!=smt2_tokenizert::OPEN)
    throw error("Grammar production rule must start with '('");

  if(smt2_tokenizer.next_token()!=smt2_tokenizert::SYMBOL)
    throw error("Grammar production rule must start with non terminal name");
  
  id = smt2_tokenizer.get_buffer();
  typet nt_sort = sort(); 
  if(id!=nonterminal.get_identifier() || nt_sort != nonterminal.type())
    throw error("Grouped rule listing does not match the name in order) from the predeclaration");

  if(smt2_tokenizer.next_token()!=smt2_tokenizert::OPEN)
    throw error("Grammar production rule must start with '('");

  while(smt2_tokenizer.peek()!=smt2_tokenizert::CLOSE)
  {
    auto rule = expression();
    if(rule.type()!=nt_sort)
      throw error("rule does not match sort");
    rules.push_back(rule);
  }
  smt2_tokenizer.next_token(); // eat the close
  if(smt2_tokenizer.next_token()!=smt2_tokenizert::CLOSE)
    throw error("Grammar production rule must end with ')'");

  return rules;
}

symbol_exprt parsert::NTDef()
{
  // (Symbol Sort)
  if(smt2_tokenizer.next_token()!=smt2_tokenizert::OPEN)
    throw error("NTDef must begin with '('");

  if(smt2_tokenizer.peek()==smt2_tokenizert::OPEN)
    smt2_tokenizer.next_token(); // symbol might be in another set of parenthesis

  if(smt2_tokenizer.next_token()!=smt2_tokenizert::SYMBOL)
    throw error("NTDef must have a symbol");

  irep_idt id = smt2_tokenizer.get_buffer();  
  typet nt_sort = sort();
  add_unique_id(id, exprt(ID_nil, nt_sort));

  if(smt2_tokenizer.next_token()!=smt2_tokenizert::CLOSE)
    throw error("NTDef must end with ')'");
  
  return symbol_exprt(id, nt_sort);
}

syntactic_templatet parsert::parse_grammar()
{
  syntactic_templatet result;
  std::vector<symbol_exprt> non_terminals;
  std::map<irep_idt, std::vector<exprt>> production_rules;

  // it is not necessary to give a syntactic template
  if(smt2_tokenizer.peek()==smt2_tokenizert::CLOSE)
    return result;
  
  if(smt2_tokenizer.next_token()!=smt2_tokenizert::OPEN)
    throw error("Nonterminal list must begin with '('");

  // parse list of nonterminals
  while(smt2_tokenizer.peek()!=smt2_tokenizert::CLOSE)
  {
    auto next_nonterminal = NTDef();
    non_terminals.push_back(next_nonterminal);
    result.nt_ids.push_back(next_nonterminal.get_identifier());
  }
    // eat the close
  smt2_tokenizer.next_token();
  // eat the open
  smt2_tokenizer.next_token();

  for(const auto &nt: non_terminals)
  {
    std::vector<exprt> production_rule = GTerm_seq(nt);
    result.production_rules[nt.get_identifier()] = production_rule;
  }
  smt2_tokenizer.next_token(); // eat the close

  return result;
}

void parsert::add_synth_fun_id(irep_idt id, 
const smt2_parsert::signature_with_parameter_idst &sig, const syntactic_templatet& grammar)
{
  // put in ID map
  add_unique_id(id, exprt(ID_nil, sig.type));
  // and put in synthesis function list
  synth_funt f;
  f.grammar = grammar;
  f.id = id;
  f.type = sig.type;
  f.parameters = sig.parameters;
  synthesis_functions[id] = f;
}

// I can add things to the CBMC parser by adding to the commands.
void parsert::setup_commands()
{
  commands["check-synth"] = [this] {
    exit=true;
  };

  commands["assert"] = [this]()
  {
    exprt e = expression();
    if (e.is_not_nil())
    {
      // expand_function_applications(e);
      assertions.push_back(e);
    }
  };
  commands["constraint"] = [this]()
  {
    exprt e = expression();
    if (e.is_not_nil())
    {
      // expand_function_applications(e);
      constraints.push_back(e);
    }
  };
  commands["assume"] = [this]()
  {
    exprt e = expression();
    if (e.is_not_nil())
    {
      // expand_function_applications(e);
      assumptions.push_back(e);
    }
  };
  commands["set-logic"] = [this]()
  {
    if (next_token() != smt2_tokenizert::SYMBOL)
      throw error("expected a string after set-logic");

    const auto s = smt2_tokenizer.get_buffer();
    logic = s;
  };
  commands["set-info"] = [this]()
  {
    if (next_token() != smt2_tokenizert::KEYWORD)
      throw error("expected a keyword after set-logic");

    std::string info = smt2_tokenizer.get_buffer();

    while (smt2_tokenizer.peek() != smt2_tokenizert::CLOSE)
    {
      next_token();
      const auto s = smt2_tokenizer.get_buffer();
      info += " " + s;
    }
    set_info_cmds.push_back(info);
  };

  commands["synth-fun"] = [this]()
  {
    if (next_token() != smt2_tokenizert::SYMBOL)
      throw error("expected a symbol after synth-fun");

    const irep_idt id = smt2_tokenizer.get_buffer();

    const auto signature = function_signature_definition();
    std::cout<<"parsed function signature and id "<< id2string(id)<<std::endl;

    // put the parameters into the scope and take care of hiding
    std::vector<std::pair<irep_idt, idt>> hidden_ids;

    for (const auto &pair : signature.ids_and_types())
    {
      auto insert_result =
          id_map.insert({pair.first, idt{idt::PARAMETER, pair.second}});
      if (!insert_result.second) // already there
      {
        auto &id_entry = *insert_result.first;
        hidden_ids.emplace_back(id_entry.first, std::move(id_entry.second));
        id_entry.second = idt{idt::PARAMETER, pair.second};
      }
    }

    // now parse grammar if there is one
    syntactic_templatet grammar = parse_grammar();

    // remove parameter ids
    for (auto &id : signature.parameters)
      id_map.erase(id);

    // restore the hidden ids, if any
    for (auto &hidden_id : hidden_ids)
      id_map.insert(std::move(hidden_id));

    // create the synthesis function
     add_synth_fun_id(id, signature, grammar);
  };
}

void parsert::build_smt_problem()
{
  if (constraints.size() != 0 || assumptions.size() != 0 || synthesis_functions.size() != 0)
    throw error("SMT problem cannot contain constraints or assumptions or synthess functions");

  smt_problem.logic = logic;

  for (const auto &a : assertions)
    smt_problem.assertions.push_back(a);

  for (const auto &cmd : set_info_cmds)
    smt_problem.comments.push_back(cmd);

  // identify the free variables and defined functions in the problem
  for (const auto &id : id_map)
  {
    if (id.second.definition.is_nil() && id.second.kind == smt2_parsert::idt::VARIABLE)
      smt_problem.free_var[symbol_exprt(id.first, id.second.type)] = nil_exprt();
    else
      smt_problem.defined_functions[symbol_exprt(id.first, id.second.type)] = id.second.definition;
  }
}

smt_problemt parsert::get_smt_problem()
{
  if(smt_problem.assertions.empty())
    build_smt_problem();
  if(smt_problem.assertions.empty())
    throw error("SMT problem has no assertions");
  return smt_problem;
}

sygus_problemt parsert::get_sygus_problem()
{
  if(sygus_problem.constraints.empty())
    build_sygus_problem();
  if(sygus_problem.constraints.empty())
    throw error("SyGuS problem has no constraints");
  return sygus_problem;
}

void parsert::build_sygus_problem()
{
  if (assertions.size() != 0)
    throw error("SyGuS problem cannot contain assertions");
  if (synthesis_functions.size() == 0)
    throw error("SyGuS problem has no synthesis functions");

  sygus_problem.logic = logic;
  for (const auto &a : constraints)
    sygus_problem.constraints.push_back(a);

  for (const auto &a : assumptions)
    sygus_problem.assumptions.push_back(a);

  for (const auto &cmd : set_info_cmds)
    sygus_problem.comments.push_back(cmd);

  for(const auto &f: synthesis_functions)
  {
    sygus_problem.synthesis_functions.push_back(f.second);
  }


  // identify the free variables and defined functions in the problem
  for (const auto &id : id_map)
  {
    if (id.second.definition.is_nil() && id.second.kind == smt2_parsert::idt::VARIABLE)
    {
      if(synthesis_functions.find(id.first)==synthesis_functions.end())
      {
        if(id.second.type.id()==ID_mathematical_function)
          throw error("SyGuS problem cannot contain n-ary uninterpreted functions");
        sygus_problem.free_var.push_back(symbol_exprt(id.first, id.second.type));
      }
    }
    else
    {
      sygus_problem.defined_functions[symbol_exprt(id.first, id.second.type)] = id.second.definition;
    }
  }
}

void parsert::parse_model()
{
  exit = false;

  if (smt2_tokenizer.peek() == smt2_tokenizert::END_OF_FILE)
    throw error("first token was an end of file'");

  if (next_token() != smt2_tokenizert::OPEN)
    throw error("model must start with '('");

  while (!exit)
  {
    if (smt2_tokenizer.peek() == smt2_tokenizert::END_OF_FILE ||
        smt2_tokenizer.peek() == smt2_tokenizert::CLOSE)
    {
      exit = true;
      return;
    }

    if (next_token() != smt2_tokenizert::OPEN)
      throw error("command must start with '('");

    if (next_token() != smt2_tokenizert::SYMBOL)
    {
      ignore_command();
      throw error("expected symbol as command");
    }
    command(smt2_tokenizer.get_buffer());

    switch (next_token())
    {
    case smt2_tokenizert::END_OF_FILE:
      throw error(
          "expected closing parenthesis at end of command,"
          " but got EOF");

    case smt2_tokenizert::CLOSE:
      // what we expect
      break;

    case smt2_tokenizert::OPEN:
    case smt2_tokenizert::SYMBOL:
    case smt2_tokenizert::NUMERAL:
    case smt2_tokenizert::STRING_LITERAL:
    case smt2_tokenizert::NONE:
    case smt2_tokenizert::KEYWORD:
      throw error("expected ')' at end of command");
    }
  }
}
