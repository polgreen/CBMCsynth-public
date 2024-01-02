SRC=main.cpp smt2_frontend.cpp sygus_frontend.cpp \
parsing/parser.cpp utils/printing_utils.cpp utils/util.cpp \
 sygus_problem.cpp synthesis/synth_td.cpp synthesis/synth_bu.cpp \
 smt_problem.cpp utils/expr2sygus.cpp cegis.cpp synthesis/synth.cpp \
 utils/cvc5_synth.cpp verification/verify.cpp verification/mini_verify.cpp \
 synthesis/synth_prob_bu.cpp

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

CLEANFILES = CBMCsynth$(EXEEXT)

all: CBMCsynth$(EXEEXT)

###############################################################################

CBMCsynth$(EXEEXT): $(OBJ)
	$(LINKBIN)