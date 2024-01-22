CPROVER_DIR ?= lib/cbmc
CXXFLAGS+= -Wno-error=misleading-indentation -std=c++17 -Wno-error=overloaded-virtual
CXXFLAGS+=-g -O0
LDLIBS+=-lcurl