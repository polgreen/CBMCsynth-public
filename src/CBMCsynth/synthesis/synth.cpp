
#include "synth.h"
#include <iostream>

// this file contains synthesis utils that don't need to be part of a class

bool contains_nonterminal(const exprt &expr, const syntactic_templatet& grammar)
{
  if (expr.id() == ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if (grammar.production_rules.find(symbol.get_identifier()) != grammar.production_rules.end())
    {
      return true;
    }
  }
  else
  {
    for (auto &op : expr.operands())
    {
      if (contains_nonterminal(op, grammar))
        return true;
    }
  }
  return false;
}

void count_nonterminals(const exprt &expr, const syntactic_templatet& grammar, std::size_t &count)
{

  if (expr.id() == ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if (grammar.production_rules.find(symbol.get_identifier()) != grammar.production_rules.end())
    {
      count++;
    }
  }
  else
  {
    for (auto &op : expr.operands())
    {
     count_nonterminals(op, grammar, count);
    }
  }
}

std::size_t count_nonterminals(const exprt &expr, const syntactic_templatet& grammar)
{
  std::size_t count=0;
  count_nonterminals(expr, grammar, count);
  return count;
}

void list_nonterminals(const exprt &expr, const syntactic_templatet& grammar, std::vector<symbol_exprt> &list)
{

  if (expr.id() == ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if (grammar.production_rules.find(symbol.get_identifier()) != grammar.production_rules.end())
    {
      list.push_back(to_symbol_expr(expr));
    }
  }
  else
  {
    for (auto &op : expr.operands())
    {
     list_nonterminals(op, grammar, list);
    }
  }
}


std::vector<symbol_exprt> list_nonterminals(const exprt &expr, const syntactic_templatet& grammar)
{
  std::vector<symbol_exprt> list;
  list_nonterminals(expr, grammar, list);
  return list;
}

void get_nonterminal_counts(const exprt &expr, const syntactic_templatet& grammar, std::unordered_map<irep_idt, unsigned> &counts)
{
  if (expr.id() == ID_symbol)
  {
    auto &symbol = to_symbol_expr(expr);
    if (grammar.production_rules.find(symbol.get_identifier()) != grammar.production_rules.end())
    {
      if(counts.find(symbol.get_identifier())==counts.end())
        counts[symbol.get_identifier()]=1;
      else
        counts[symbol.get_identifier()]++;
    }
  }
  else
  {
    for (auto &op : expr.operands())
    {
     get_nonterminal_counts(op, grammar, counts);
    }
  }
}

std::unordered_map<irep_idt, unsigned> get_nonterminal_counts(const exprt &expr, const syntactic_templatet& grammar)
{
  std::unordered_map<irep_idt, unsigned> counts;
  get_nonterminal_counts(expr, grammar, counts);
  return counts;
}