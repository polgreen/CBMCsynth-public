DIRS = CBMCsynth

include config.inc

all: CBMCsynth.dir

.PHONY: $(DIRS)

$(patsubst %, %.dir, $(DIRS)):
	$(MAKE) $(MAKEARGS) -C $(basename $@)

# Dependencies

CBMCsynth.dir: cprover.dir

# building cbmc proper
.PHONY: cprover.dir
cprover.dir:
	$(MAKE) $(MAKEARGS) -C ../$(CPROVER_DIR)/src

.PHONY: clean
clean: $(patsubst %, %_clean, $(DIRS))

$(patsubst %, %_clean, $(DIRS)):
	if [ -e $(patsubst %_clean, %, $@)/. ] ; then \
	        $(MAKE) $(MAKEARGS) -C $(patsubst %_clean, %, $@) clean ; \
	fi
