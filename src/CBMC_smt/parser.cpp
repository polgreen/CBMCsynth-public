#include "parser.h"
#include <util/replace_symbol.h>
#include <iostream>


void parsert::expand_function_applications(exprt &expr)
{
  for(exprt &op : expr.operands())
    expand_function_applications(op);

  if(expr.id()==ID_function_application)
  {
    auto &app=to_function_application_expr(expr);

    if(app.function().id() == ID_symbol)
    {
      // look up the symbol
      auto identifier = to_symbol_expr(app.function()).get_identifier();
      auto f_it = id_map.find(identifier);

      if(f_it != id_map.end())
      {
        const auto &f = f_it->second;

        DATA_INVARIANT(
          f.type.id() == ID_mathematical_function,
          "type of function symbol must be mathematical_function_type");

        const auto &domain = to_mathematical_function_type(f.type).domain();

        DATA_INVARIANT(
          domain.size() == app.arguments().size(),
          "number of parameters must match number of arguments");

        // Does it have a definition? It's otherwise uninterpreted.
        if(!f.definition.is_nil())
        {
          exprt body = f.definition;

          if(body.id() == ID_lambda)
            body = to_lambda_expr(body).application(app.arguments());

          expand_function_applications(body); // rec. call
          expr = body;
        }
      }
    }
  }
}


// I can add things to the CBMC parser by adding to the commands.
void parsert::setup_commands()
{
  commands["assert"] = [this]()
  {
    exprt e = expression();
    if (e.is_not_nil())
    {
      expand_function_applications(e);
      assertions.push_back(e);
    }
  };
}

