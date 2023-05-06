SRC=main.cpp smt2_frontend.cpp parser.cpp printing_utils.cpp printing_utils.h \
term_position.cpp anti_unification.cpp create_training_data.cpp sygus_problem.cpp \
 problem.cpp expr2sygus.cpp unification.cpp util.cpp subterm_heuristics.cpp

OBJ+= ../../$(CPROVER_DIR)/src/ansi-c/ansi-c$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/assembler/assembler$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/linking/linking$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/big-int/big-int$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/goto-symex/goto-symex$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/goto-programs/goto-programs$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/pointer-analysis/pointer-analysis$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/analyses/analyses$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/langapi/langapi$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/solvers/solvers$(LIBEXT) \
      ../../$(CPROVER_DIR)/src/analyses/dirty$(OBJEXT) \
      ../../$(CPROVER_DIR)/src/util/util$(LIBEXT)

INCLUDES= -I ../../$(CPROVER_DIR)/src -I ..

LIBS =

include ../config.inc
include ../../$(CPROVER_DIR)/src/config.inc
include ../../$(CPROVER_DIR)/src/common

CLEANFILES = CBMC_smt$(EXEEXT)

all: CBMC_smt$(EXEEXT)

###############################################################################

CBMC_smt$(EXEEXT): $(OBJ)
	$(LINKBIN)