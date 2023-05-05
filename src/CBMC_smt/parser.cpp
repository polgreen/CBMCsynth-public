#include "parser.h"
#include <util/replace_symbol.h>
#include <iostream>



// I can add things to the CBMC parser by adding to the commands.
void parsert::setup_commands()
{
  commands["assert"] = [this]()
  {
    exprt e = expression();
    if (e.is_not_nil())
    {
     // expand_function_applications(e);
      assertions.push_back(e);
    }
  };
  commands["set-logic"] = [this]() {
    
    if(next_token() != smt2_tokenizert::SYMBOL)
      throw error("expected a string after set-logic");
  
    std::cout<<"Parsing logic"<<std::endl;
    const auto s = smt2_tokenizer.get_buffer();
    logic = s;
  };

}

