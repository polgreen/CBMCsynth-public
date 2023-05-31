#include "parser.h"
#include <util/replace_symbol.h>
#include <iostream>


// I can add things to the CBMC parser by adding to the commands.
void parsert::setup_commands() {
    commands["assert"] = [this]() {
        exprt e = expression();
        if (e.is_not_nil()) {
            // expand_function_applications(e);
            assertions.push_back(e);
        }
    };
    commands["set-logic"] = [this]() {

        if (next_token() != smt2_tokenizert::SYMBOL)
            throw error("expected a string after set-logic");

        const auto s = smt2_tokenizer.get_buffer();
        logic = s;
    };
    commands["set-info"] = [this]() {

        if (next_token() != smt2_tokenizert::KEYWORD)
            throw error("expected a keyword after set-logic");

        std::string info = smt2_tokenizer.get_buffer();

        while (smt2_tokenizer.peek() != smt2_tokenizert::CLOSE) {
            next_token();
            const auto s = smt2_tokenizer.get_buffer();
            info += " " + s;
        }
        set_info_cmds.push_back(info);
    };


}

void parsert::parse_model()
{
  exit = false;

  if (smt2_tokenizer.peek() == smt2_tokenizert::END_OF_FILE)
  {
    throw error("first token was an end of file'");
  }

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
