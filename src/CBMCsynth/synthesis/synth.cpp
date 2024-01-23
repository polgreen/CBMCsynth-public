
#include "synth.h"
#include <iostream>

// this file contains synthesis utils that don't need to be part of a class


std::string print_sequence(std::vector<unsigned> &sequence)
{
  std::string result;
  for(auto &s: sequence)
  {
    result+=std::to_string(s);
    result+=" ";
  }
  return result;
}