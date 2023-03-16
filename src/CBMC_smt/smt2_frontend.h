#ifndef _SMT2_FRONTEND_H_
#define _SMT2_FRONTEND_H_

#include <util/cmdline.h>

int smt2_frontend(const cmdlinet &);

int smt2_frontend(const cmdlinet &, std::istream &in);


#endif /*_SMT2_FRONTEND_H_*/