#include <cstdio>
#include "Heavy_{{patch_name}}.h"

extern size_t heap_offset;

int main(int argc, char* argv[]) {
    HeavyContextInterface* hvContext;
    float buffer[64];

    hvContext = hv_{{patch_name}}_new_with_options(48000, {{msg_pool_size_kb}}, {{input_queue_size_kb}}, {{output_queue_size_kb}});
    for(int i = 0; i < 100; i++) {
        {%if p_pitch is defined %} 
        hv_sendFloatToReceiver(hvContext, HV_{{patch_name|upper}}_PARAM_IN_PITCH, 440.f);
        {%endif %} 
        hv_processInline(hvContext, NULL, buffer, 64);
    }
    printf("total: %d\n", heap_offset);
}
