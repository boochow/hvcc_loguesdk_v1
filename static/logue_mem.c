#include "logue_mem.h"
#ifdef DEBUG
#include <stdio.h>
#endif

#ifndef UNIT_HEAP_SIZE
#define UNIT_HEAP_SIZE (1024 * 3)
#endif

static unsigned char heap[UNIT_HEAP_SIZE];
#ifndef DEBUG
static size_t heap_offset = 0;
#else
size_t heap_offset = 0;
#endif

void* logue_malloc(size_t size) {
    if (heap_offset + size > UNIT_HEAP_SIZE) {
#ifdef DEBUG
        printf("malloc: %ld : NG\n", size);
#endif
        return NULL;
    }
    void* ptr = &heap[heap_offset];
    heap_offset += size;
#ifdef DEBUG
    printf("malloc: %ld : OK\n", size);
#endif
    return ptr;
}

void logue_free(void *ptr) {
    // do nothing
}
