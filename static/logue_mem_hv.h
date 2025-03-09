#ifndef __LOGUE_MEM_HV__
#define __LOGUE_MEM_HV__

#include "logue_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef hv_malloc
#define hv_malloc(_n)  logue_malloc(_n)

#undef hv_realloc
#define hv_realloc(a, b) logue_malloc(b)

#undef hv_free
#define hv_free(x)  logue_free(x)

#ifdef __cplusplus
}
#endif

#endif
