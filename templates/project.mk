# #############################################################################
# Project Customization
# #############################################################################

.DEFAULT_GOAL := all

PROJECT = {{patch_name}}

ifndef HEAP_SIZE
# Estimate the necessary heap size and store it into HEAP_SIZE_FILE
HEAP_SIZE_FILE := logue_heap_size.mk

$(HEAP_SIZE_FILE):
	$(MAKE) -f Makefile.testmem
	@TMPFILE=$$(mktemp) ; \
	./testmem > $$TMPFILE ; \
	HEAP_SIZE_TESTMEM=$$(grep '^total:' $$TMPFILE | awk '{print $$2}') ; \
	echo "HEAP_SIZE := $$HEAP_SIZE_TESTMEM" > $(HEAP_SIZE_FILE) ; \
	rm -f $$TMPFILE ; \
	$(MAKE) -f Makefile.testmem clean

include $(HEAP_SIZE_FILE)
endif

UCSRC = logue_mem.c {{heavy_files_c}}

UCXXSRC = logue_heavy.cpp {{heavy_files_cpp}}

UINCDIR = 

UDEFS = -DNDEBUG -DUNIT_HEAP_SIZE=$(HEAP_SIZE) -fvisibility=hidden

# Try disabling this option when the results are inaccurate.
UDEFS += -DLOGUE_FAST_MATH

# Disable this for better sound (for simple patches only)
UDEFS += -DRENDER_HALF

ULIB =

ULIBDIR =
