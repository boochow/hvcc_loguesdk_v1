#ifndef __LOGUE_MATH_HV__
#define __LOGUE_MATH_HV__

#include "utils/float_math.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LOGUE_FAST_MATH

#undef hv_sin_f
#define hv_sin_f(a) fastsinfullf(a)

#undef hv_cos_f
#define hv_cos_f(a) fastcosfullf(a)

#undef hv_tan_f
#define hv_tan_f(a) fasttanfullf(a)

#undef hv_atan_f
#define hv_atan_f(a) fasteratan2f(a, 1.f)

#undef hv_atan2_f
#define hv_atan2_f(a, b) fasteratan2f(a, b)

#undef hv_exp_f
#define hv_exp_f(a) fastexpf(a)

#undef hv_abs_f
#define hv_abs_f(a) si_fabsf(a)

#undef hv_log_f
#define hv_log_f(a) fastlogf(a)

#undef hv_round_f
#define hv_round_f(a) si_roundf(a)

#undef hv_pow_f
#define hv_pow_f(a, b) fastpowf(a, b)

#endif

#ifdef __cplusplus
}
#endif

#endif
