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

static bool stop_osc_param;
static HeavyContextInterface* hvContext;

{% if shape is defined %}
    {% if shape['range'] is defined %}
static int32_t shape;
static bool shape_dirty;
    {% elif shape['range_f'] is defined %}
static float shape;
static bool shape_dirty;
    {% endif %}
{% endif %}
{% if alt is defined %}
    {% if alt['range'] is defined %}
static int32_t alt;
static bool alt_dirty;
    {% elif alt['range_f'] is defined %}
static float alt;
static bool alt_dirty;
    {% endif %}
{% endif %}

{% if noteon_trig is defined %}
static bool noteon_trig_dirty;
{% endif %}
{% if noteoff_trig is defined %}
static bool noteoff_trig_dirty;
{% endif %}

{% for i in range(1, 7) %}
    {% set id = "param_id" ~ i %}
    {% if param[id] is defined %}
        {% if param[id]['range'] is defined %}
static int32_t {{param[id]['name']}};
        {% elif param[id]['range_f'] is defined %}
static float {{param[id]['name']}};
        {% endif %}
    {% endif %}
{% endfor %}
{% if num_param > 0 %}
static bool param_dirty[{{num_param}}];
{% endif %}
{% for key, entry in table.items() %}
static float * table_{{ key }};
static unsigned int table_{{ key }}_len;
{% endfor %}

void OSC_INIT(uint32_t platform, uint32_t api)
{
    stop_osc_param = true;
    hvContext = hv_{{patch_name}}_new_with_options(48000, HV_MSGPOOLSIZE, HV_INPUTQSIZE, HV_OUTPUTQSIZE);
    {% if shape is defined %}
    shape = {{shape['default']}};
    shape_dirty = true;
    {% endif %}
    {% if alt is defined %}
    alt = {{alt['default']}};
    alt_dirty = true;
    {% endif %}
    {% for i in range(1, 7) %}
      {% set id = "param_id" ~ i %}
      {% if param[id] is defined %}
    {{param[id]['name']}} = {{param[id]['default']}};
    param_dirty[{{i - 1}}] = true;
      {% endif %}
    {% endfor %}
    {% for key, entry in table.items() %}
    table_{{ key }} = hv_table_getBuffer(hvContext, HV_{{patch_name|upper}}_TABLE_{{key|upper}});
    table_{{ key }}_len = hv_table_getLength(hvContext, HV_{{patch_name|upper}}_TABLE_{{key|upper}});
    {% endfor %}
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
#ifdef RENDER_HALF
    float buffer[frames >> 1];
    static float last_buf = 0.f;
#else
    float buffer[frames];
#endif
    float * __restrict p = buffer;
    q31_t * __restrict y = (q31_t *)yn;
    const q31_t * y_e = y + frames;
    {% if slfo is defined %}
    float slfo;
    {% endif %}
    {% if pitch is defined %} 
    const float pitch = osc_w0f_for_note((params->pitch)>>8, params->pitch & 0xFF) * k_samplerate;
    {% endif %}
    {% if pitch_note is defined %}
    const float note_f = (params->pitch >> 8) + (params->pitch & 0xFF) * 0.00390625;
    {% endif %}

    stop_osc_param = false;
    {% if slfo is defined %}
    slfo = q31_to_f32(params->shape_lfo);
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SLFO, slfo);
    {% endif %}
    {% if pitch is defined %} 
#ifdef RENDER_HALF
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH, pitch * 2.f);
#else
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH, pitch);
#endif
    {% endif %}
    {% if pitch_note is defined %}
#ifdef RENDER_HALF
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH_NOTE, note_f * 2.f);
#else
    hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH_NOTE, note_f);
#endif
    {% endif %}
    {% if shape is defined %}
        {% if shape['range'] is defined %}
    if (shape_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE, shape)) {
            shape_dirty = false;
        }
    }
        {% elif shape['range_f'] is defined %}
    if (shape_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE_F, shape)) {
            shape_dirty = false;
        }
    }
        {% endif %}
    {% endif %}
    {% if alt is defined %}
        {% if alt['range'] is defined %}
    if (alt_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT, alt)) {
            alt_dirty = false;
        }
    }
        {% elif alt['range_f'] is defined %}
    if (alt_dirty) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT_F, alt)) {
            alt_dirty = false;
        }
    }
        {% endif %}
    {% endif %}
    {% if noteon_trig is defined %}
    if (noteon_trig_dirty) {
        if (hv_sendBangToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_NOTEON_TRIG)) {
            noteon_trig_dirty = false;
        }
    }
    {% endif %}
    {% if noteoff_trig is defined %}
    if (noteoff_trig_dirty) {
        if (hv_sendBangToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_NOTEOFF_TRIG)) {
            noteoff_trig_dirty = false;
        }
    }
    {% endif %}
    {% for i in range(1, 7) %}
    {% set id = "param_id" ~ i %}
    {% if param[id] is defined %}
    if (param_dirty[{{i - 1}}]) {
        if (hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{param[id]['name']|upper}}, {{param[id]['name']}})) {
            param_dirty[{{i - 1}}] = false;
        }
    }
    {% endif %}
    {% endfor %}
    {% for key, entry in table.items() %}
    {% if entry.type == "random" %}
    table_{{ key }}_len = hv_table_getLength(hvContext, HV_{{patch_name|upper}}_TABLE_{{key|upper}});
    for (int i = 0; i < table_{{ key }}_len ; i++) {
        table_{{ key }}[i] = osc_white();
    }
    {% endif %}
    {% endfor %}

#ifdef RENDER_HALF
    hv_processInline(hvContext, NULL, buffer, frames >> 1);
    for(int i = 0; y!= y_e; i++) {
        if (i & 1) {
            last_buf = *p++;
            *(y++) = f32_to_q31(last_buf);
        } else {
            *(y++) = f32_to_q31((*p + last_buf) * 0.5);
        }
    }
#else
    hv_processInline(hvContext, NULL, buffer, frames);
    for(; y!= y_e; ) {
        *(y++) = f32_to_q31(*p++);
    }
#endif
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
    {% if noteon_trig is defined %} 
    noteon_trig_dirty = true;
    {% else %}
    (void)params;
    {% endif %}
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
    {% if noteoff_trig is defined %} 
    noteoff_trig_dirty = true;
    {% else %}
    (void)params;
    {% endif %}
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
    float knob_f = param_val_to_f32(value);
    float f;

    if (stop_osc_param) {
        return; // avoid all parameters to be zero'ed after OSC_INIT
    }
    switch(index){
    case k_user_osc_param_shape:
        {% if shape is defined %}
            {% if shape['range'] is defined %}
        shape = value;
        shape_dirty = true;
            {% elif shape['range_f'] is defined %}
        shape = {{shape['range_f']}} * knob_f + {{shape['min']}};
        shape_dirty = true;
            {% endif %}
        {% endif %}
        break;        
    case k_user_osc_param_shiftshape:
        {% if alt is defined %}
            {% if alt['range'] is defined %}
        alt = value;
        alt_dirty = true;
            {% elif alt['range_f'] is defined %}
        alt = {{alt['range_f']}} * knob_f + {{alt['min']}};
        alt_dirty = true;
            {% endif %}
        {% endif %}
        break;
    {% for i in range(1, 7) %}
    {% set id = "param_id" ~ i %}
    {% if param[id] is defined %}
    case k_user_osc_{{id}}:
        {% if param[id]['range'] is defined %}
        {{param[id]['name']}} = value;
        param_dirty[{{i - 1}}] = true;
        {% elif param[id]['range_f'] is defined %}
        {{param[id]['name']}} = {{param[id]['range_f']}} * value / ({{param[id]['max'] - param[id]['min']}}) + ({{param[id]['min_f']}});;
        param_dirty[{{i - 1}}] = true;
        {% endif %}
    {% endif %}
    {% endfor %}
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
