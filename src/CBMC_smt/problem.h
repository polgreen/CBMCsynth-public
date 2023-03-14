#ifndef _PROBLEM_H_
#define _PROBLEM_H_

// we store anything we might need to pass around here


class problemt
{
  public:
    std::vector<exprt> assertions;
    std::map<symbol_exprt, exprt> free_var;
    std::string logic;
    

};

#endif /*_PROBLEM_H_*/