# #############################################################################
# Project Customization
# #############################################################################

.DEFAULT_GOAL := all

PROJECT = {{patch_name}}

ifndef HEAP_SIZE
# Estimate the necessary heap size and store it into HEAP_SIZE_FILE
HEAP_SIZE_FILE := logue_heap_size.mk

$(HEAP_SIZE_FILE):
	@if command -v gcc >/dev/null 2>&1 && command -v g++ >/dev/null 2>&1; then \
		$(MAKE) -f Makefile.testmem ; \
		TMPFILE=$$(mktemp) ; \
		./testmem > $$TMPFILE ; \
		HEAP_SIZE_TESTMEM=$$(grep '^total:' $$TMPFILE | awk '{print $$2}') ; \
		echo "HEAP_SIZE := $$HEAP_SIZE_TESTMEM" > $(HEAP_SIZE_FILE) ; \
		rm -f $$TMPFILE ; \
		$(MAKE) -f Makefile.testmem clean ; \
	else \
		echo "HEAP_SIZE := 3072" > $(HEAP_SIZE_FILE) ; \
	fi

include $(HEAP_SIZE_FILE)
endif

UCSRC = logue_mem.c {{heavy_files_c}}

UCXXSRC = logue_heavy.cpp {{heavy_files_cpp}}

UINCDIR = 

UDEFS = -DNDEBUG -DUNIT_HEAP_SIZE=$(HEAP_SIZE) -fvisibility=hidden

# Assume Unix-like to suppress warning messages
UDEFS += -U_WIN32 -U_WIN64 -U_MSC_VER -D__unix

# Try disabling this option when the results are inaccurate.
UDEFS += -DLOGUE_FAST_MATH

# Disable this for better sound (for simple patches only)
UDEFS += -DRENDER_HALF

ULIB =

ULIBDIR =
