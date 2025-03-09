#include <errno.h>
#include <sys/types.h>
#include <cstdlib>

#include "userosc.h"

#include "Heavy_{{patch_name}}.h"

#ifndef HV_MSGPOOLSIZE
 #define HV_MSGPOOLSIZE {{msg_pool_size_kb}}
#endif
#ifndef HV_INPUTQSIZE
 #define HV_INPUTQSIZE {{input_queue_size_kb}}
#endif
#ifndef HV_OUTPUTQSIZE
 #define HV_OUTPUTQSIZE {{output_queue_size_kb}}
#endif

static HeavyContextInterface* hvContext;

void OSC_INIT(uint32_t platform, uint32_t api)
{
    hvContext = hv_{{patch_name}}_new_with_options(48000, HV_MSGPOOLSIZE, HV_INPUTQSIZE, HV_OUTPUTQSIZE);
    {% if p_shape_range is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE, {{p_shape_default}});
    {% elif p_shape_range_f is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE_F, {{p_shape_default}});
    {% endif %}
    {% if p_alt_range is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT, {{p_alt_default}});
    {% elif p_alt_range_f is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT_F, {{p_alt_default}});
    {% endif %}
    {% if p_1_range is defined or p_1_range_i is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_1_name|upper}}, {{p_1_default}});
    {% endif %}
    {% if p_2_range is defined or p_2_range_i is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_2_name|upper}}, {{p_2_default}});
    {% endif %}
    {% if p_3_range is defined or p_3_range_i is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_3_name|upper}}, {{p_3_default}});
    {% endif %}
    {% if p_4_range is defined or p_4_range_i is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_4_name|upper}}, {{p_4_default}});
    {% endif %}
    {% if p_5_range is defined or p_5_range_i is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_5_name|upper}}, {{p_5_default}});
    {% endif %}
    {% if p_6_range is defined or p_6_range_i is defined %}
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_6_name|upper}}, {{p_6_default}});
    {% endif %}
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
    float buffer[frames];
    float * __restrict p = buffer;
    q31_t * __restrict y = (q31_t *)yn;
    const q31_t * y_e = y + frames;
    {% if p_slfo is defined %}
    float slfo;

    slfo = q31_to_f32(params->shape_lfo);
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SLFO, slfo);
    {% endif %}
    {% if p_pitch is defined %} 
    const float pitch = osc_w0f_for_note((params->pitch)>>8, params->pitch & 0xFF) * k_samplerate;

    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH, pitch);
    {% endif %}
    {% if p_pitch_note is defined %}
    const float note_f = (params->pitch >> 8) + (params->pitch & 0xFF) * 0.00390625;

    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH_NOTE, note_f);
    {% endif %}
    hv_processInline(hvContext, NULL, buffer, frames);
    for(; y!= y_e; ) {
        *(y++) = f32_to_q31(*p++);
    }
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
    {% if p_noteon_trig is defined %} 
    hv_sendBangToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_NOTEON_TRIG);
    {% else %}
    (void)params;
    {% endif %}
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
    {% if p_noteoff_trig is defined %} 
    hv_sendBangToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_NOTEOFF_TRIG);
    {% else %}
    (void)params;
    {% endif %}
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
    float knob_f = param_val_to_f32(value);
    float f;
    switch(index){
    case k_user_osc_param_shape:
        {% if p_shape_range is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE, value);
        {% elif p_shape_range_f is defined %}
        f = {{p_shape_range_f}} * knob_f + {{p_shape_min}};
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE_F, f);
        {% endif %}
        break;
    case k_user_osc_param_shiftshape:
        {% if p_alt_range is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT, value);
        {% elif p_alt_range_f is defined %}
        f = {{p_alt_range_f}} * knob_f + {{p_alt_min}};
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT_F, f);
        {% endif %}
        break;
    case k_user_osc_param_id1:
        {% if p_1_range_f is defined %}
        f = {{p_1_range_f}} * value / {{p_1_range}} + ({{p_1_min_f}});
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_1_name|upper}}, f);
        {% elif p_1_range is defined %}
        f = value;
        {% if p_1_min < 0 %}
        f -= 100;
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_1_name|upper}}, f);
        {% endif %}
        break;
    case k_user_osc_param_id2:
        {% if p_2_range_f is defined %}
        {% if p_2_min < 0 %}
        f = value - 100 - ({{p_2_min}});
        f = {{p_2_range_f}} * f / {{p_2_range}} + ({{p_2_min_f}});
        {% else %}
        f = {{p_2_range_f}} * value / {{p_2_range}} + ({{p_2_min_f}});
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_2_name|upper}}, f);
        {% elif p_2_range is defined %}
        f = value;
        {% if p_2_min < 0 %}
        f -= 100;
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_2_name|upper}}, f);
        {% endif %}
	break;
    case k_user_osc_param_id3:
        {% if p_3_range_f is defined %}
        {% if p_3_min < 0 %}
        f = value - 100 - ({{p_3_min}});
        f = {{p_3_range_f}} * f / {{p_3_range}} + ({{p_3_min_f}});
        {% else %}
        f = {{p_3_range_f}} * value / {{p_3_range}} + ({{p_3_min_f}});
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_3_name|upper}}, f);
        {% elif p_3_range is defined %}
        f = value;
        {% if p_3_min < 0 %}
        f -= 100;
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_3_name|upper}}, f);
        {% endif %}
        break;
    case k_user_osc_param_id4:
        {% if p_4_range_f is defined %}
        {% if p_4_min < 0 %}
        f = value - 100 - ({{p_4_min}});
        f = {{p_4_range_f}} * f / {{p_4_range}} + ({{p_4_min_f}});
        {% else %}
        f = {{p_4_range_f}} * value / {{p_4_range}} + ({{p_4_min_f}});
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_4_name|upper}}, f);
        {% elif p_4_range is defined %}
        f = value;
        {% if p_4_min < 0 %}
        f -= 100;
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_4_name|upper}}, f);
        {% endif %}
        break;
    case k_user_osc_param_id5:
        {% if p_5_range_f is defined %}
        {% if p_5_min < 0 %}
        f = value - 100 - ({{p_5_min}});
        f = {{p_5_range_f}} * f / {{p_5_range}} + ({{p_5_min_f}});
        {% else %}
        f = {{p_5_range_f}} * value / {{p_5_range}} + ({{p_5_min_f}});
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_5_name|upper}}, f);
        {% elif p_5_range is defined %}
        f = value;
        {% if p_5_min < 0 %}
        f -= 100;
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_5_name|upper}}, f);
        {% endif %}
        break;
    case k_user_osc_param_id6:
        {% if p_6_range_f is defined %}
        {% if p_6_min < 0 %}
        f = value - 100 - ({{p_6_min}});
        f = {{p_6_range_f}} * f / {{p_6_range}} + ({{p_6_min_f}});
        {% else %}
        f = {{p_6_range_f}} * value / {{p_6_range}} + ({{p_6_min_f}});
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_6_name|upper}}, f);
        {% elif p_6_range is defined %}
        f = value;
        {% if p_6_min < 0 %}
        f -= 100;
        {% endif %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{p_6_name|upper}}, f);
        {% endif %}
        break;
    default:
      break;
    }
}

// dummy implementation for some starndard functions

int snprintf(char str[], size_t size, const char *format, ...) {
  return 0;
}

extern "C" void __cxa_pure_virtual() {
    while (1);  // do nothing
}

extern "C" void operator delete(void* ptr) noexcept {
    free(ptr);
}

extern "C" void operator delete[](void* ptr) noexcept {
    free(ptr);
}

extern "C" void* _sbrk(ptrdiff_t incr) {
    errno = ENOMEM;
    return (void*)-1;
}
