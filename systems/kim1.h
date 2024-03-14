#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdalign.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

} kim1_desc_t;

typedef struct {
    m6502_t cpu;
    m6530_t rriot002;
    m6530_t rriot003;
    mem_t mem;
} kim1_t;

void vic20_init(kim1_t* sys, const kim1_desc_t* desc);

#ifdef __cplusplus
} // extern "C"
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL

#include <string.h> // memcpy, memset
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#endif /* CHIPS_IMPL */
