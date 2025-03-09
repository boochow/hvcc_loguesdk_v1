# #############################################################################
# Project Customization
# #############################################################################

PROJECT = {{patch_name}}

UCSRC = logue_mem.c {{heavy_files_c}}

UCXXSRC = logue_heavy.cpp {{heavy_files_cpp}}

UINCDIR = 

UDEFS = -DNDEBUG -DUNIT_HEAP_SIZE={{heap_size}} -fvisibility=hidden

# Try disabling this option when the results are got inaccurate.
UDEFS += -DLOGUE_FAST_MATH

ULIB =

ULIBDIR =
