CPROVER_DIR ?= lib/cbmc
CXXFLAGS+= -Wno-error=misleading-indentation -std=c++17 -Wno-error=overloaded-virtual
LDLIBS+=-lcurl
