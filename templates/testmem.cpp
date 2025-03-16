#include <cstdio>
#include "Heavy_{{patch_name}}.h"

extern size_t heap_offset;

int main(int argc, char* argv[]) {
    HeavyContextInterface* hvContext;
    float buffer[64];

    hvContext = hv_{{patch_name}}_new_with_options(48000, {{msg_pool_size_kb}}, {{input_queue_size_kb}}, {{output_queue_size_kb}});
    {% if noteon_trig is defined %}
    hv_sendBangToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_NOTEON_TRIG);
    {% endif %}
    {% if noteoff_trig is defined %}
    hv_sendBangToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_NOTEOFF_TRIG);
    {% endif %}
    for(int i = 0; i < 100; i++) {
        {% if slfo is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SLFO, 0);
        {% endif %}
        {% if pitch is defined %} 
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH, 440.f);
        {% elif pitch_note is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH_NOTE, 60);
        {% endif %}
        {% if shape is defined %}
        {% if shape['range'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE, {{shape['default']}});
        {% elif shape['range_f'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_SHAPE_F, {{shape['default']}});
        {% endif %}
        {% endif %}
        {% if alt is defined %}
        {% if alt['range'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT, {{alt['default']}});
        {% elif alt['range_f'] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_ALT_F, {{alt['default']}});
        {% endif %}
        {% endif %}
        {% for i in range(1, 7) %}
        {% set id = "param_id" ~ i %}
        {% if param[id] is defined %}
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_{{param[id]['name']|upper}}, {{param[id]['default']}});
        {% endif %}
        {% endfor %}
        hv_processInline(hvContext, NULL, buffer, 64);
    }
    printf("total: %ld\n", heap_offset);
}
