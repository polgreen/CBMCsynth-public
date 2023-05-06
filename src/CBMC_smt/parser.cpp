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

