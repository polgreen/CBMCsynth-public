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
      assertions.push_back(e);
  };
}

