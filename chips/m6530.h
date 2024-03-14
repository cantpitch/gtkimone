#pragma once
/*#
    # m6530.h

    Header-only MOS 6530 RRIOT emulator written in C.

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the
    implementation.

    Optionally provide the following macros with your own implementation

    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    ## Emulated Pins
    ***************************************
    *            +-----------+            *
    *    A0 ---> |           | <--> PA0   *
    *        ... |           | ...        *
    *    A9 ---> |           | <--> PA7   *
    *            |           |            *
    *   RS0 ---> |           | <--> PB0   *
    *            |           | ...        *
    *            |           | <--> PB4   *
    *   DB0 <--> |           |            *
    *        ... |   m6530   | <--- CS2 } *
    *   DB7 <--> |           | <--> PB5 } *
    *            |           |            *
    *    RW ---> |           | <--- CS1 } *
    *            |           | <--> PB6 } *
    * (RES) ---> |           |            *
    *            |           | ---> IRQ } *
    *            |           | <--> PB7 } *
    *            +-----------+            *
    ***************************************

    ## How to use

    Call m6522_init() to initialize a new m6522_t instance (note that
    there is no m6522_desc_t struct:

    ~~~C
    m6530_t rriot1;
    m6530_init(&rriot1);
    ~~~

    In each system tick, call the m6522_tick() function, this takes
    an input pin mask, and returns a (potentially modified) output
    pin mask.

    Depending on the emulated system, the I/O and control pins
    PA0..PA7, PB0..PB7, CA1, CA2, CB1 and CB2 must be set as needed
    in the input pin mask (these are often connected to the keyboard
    matrix or peripheral devices).

    If the CPU wants to read or write VIA registers, set the CS1 pin
    to 1 (keep CS2 at 0), and set the RW pin depending on whether it's
    a register read (RW=1 means read, RW=0 means write, just like
    on the M6502 CPU), and the RS0..RS3 register select pins
    (usually identical with the shared address bus pins A0..A4).

    Note that the pin positions for RS0..RS3 and RW are shared with the
    respective M6502 pins.

    On return m6522_tick() returns a modified pin mask where the following
    pins might have changed state:

    - the IRQ pin (same bit position as M6502_IRQ)
    - the port A I/O pins PA0..PA7
    - the port A control pins CA1 and CA2
    - the port B I/O pins PB0..PB7
    - the port B control pins CB1 and CB2
    - data bus pins D0..D7 if this was a register read function.

    For an example VIA ticking code, checkout the _vic20_tick() function
    in systems/vic20.h

    To reset a m6522_t instance, call m6522_reset():

    ~~~C
    m6522_reset(&sys->via);
    ~~~

    ## LINKS

    On timer behaviour when hitting zero:

    http://forum.6502.org/viewtopic.php?f=4&t=2901

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution.
#*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// address bus pins
#define M6530_PIN_A0    (0)
#define M6530_PIN_A1    (1)
#define M6530_PIN_A2    (2)
#define M6530_PIN_A3    (3)
#define M6530_PIN_A4    (4)
#define M6530_PIN_A5    (5)
#define M6530_PIN_A6    (6)
#define M6530_PIN_A7    (7)
#define M6530_PIN_A8    (8)
#define M6530_PIN_A9    (9)
#define M6530_PIN_RS0   (10)      // RAM Select 
#define M6530_PIN_CS1   (42)      // Chip Select 1 (same as PB6)
#define M6530_PIN_CS2   (41)      // Chip Select 2 (same as PB5) 

// control pins
#define M6530_PIN_RW    (11)      // in: memory read or write access
#define M6530_PIN_IRQ   (43)      // out: interrupt (same as PB7)
#define M6530_PIN_RES   (12)      // request RESET

// data bus pins
#define M6530_PIN_D0    (16)
#define M6530_PIN_D1    (17)
#define M6530_PIN_D2    (18)
#define M6530_PIN_D3    (19)
#define M6530_PIN_D4    (20)
#define M6530_PIN_D5    (21)
#define M6530_PIN_D6    (22)
#define M6530_PIN_D7    (23)

// peripheral A port
#define M6530_PIN_PA0       (24)
#define M6530_PIN_PA1       (25)
#define M6530_PIN_PA2       (26)
#define M6530_PIN_PA3       (27)
#define M6530_PIN_PA4       (28)
#define M6530_PIN_PA5       (29)
#define M6530_PIN_PA6       (30)
#define M6530_PIN_PA7       (31)

// peripheral B port
#define M6530_PIN_PB0       (32)
#define M6530_PIN_PB1       (33)
#define M6530_PIN_PB2       (34)
#define M6530_PIN_PB3       (35)
#define M6530_PIN_PB4       (36)
#define M6530_PIN_PB5       (37)
#define M6530_PIN_PB6       (38)
#define M6530_PIN_PB7       (39)


// pin bit masks
#define M6530_A0        (1ULL<<M6530_PIN_A0)
#define M6530_A1        (1ULL<<M6530_PIN_A1)
#define M6530_A2        (1ULL<<M6530_PIN_A2)
#define M6530_A3        (1ULL<<M6530_PIN_A3)
#define M6530_A4        (1ULL<<M6530_PIN_A4)
#define M6530_A5        (1ULL<<M6530_PIN_A5)
#define M6530_A6        (1ULL<<M6530_PIN_A6)
#define M6530_A7        (1ULL<<M6530_PIN_A7)
#define M6530_A8        (1ULL<<M6530_PIN_A8)
#define M6530_A9        (1ULL<<M6530_PIN_A9)
#define M6530_RS0       (1ULL<<6530_PIN_RS0)
#define M6530_CS1       (1ULL<<6530_PIN_CS1)
#define M6530_CS2       (1ULL<<6530_PIN_CS2)
#define M6530_RES       (1ULL<<M6530_PIN_RES)
#define M6530_D0        (1ULL<<M6530_PIN_D0)
#define M6530_D1        (1ULL<<M6530_PIN_D1)
#define M6530_D2        (1ULL<<M6530_PIN_D2)
#define M6530_D3        (1ULL<<M6530_PIN_D3)
#define M6530_D4        (1ULL<<M6530_PIN_D4)
#define M6530_D5        (1ULL<<M6530_PIN_D5)
#define M6530_D6        (1ULL<<M6530_PIN_D6)
#define M6530_D7        (1ULL<<M6530_PIN_D7)
#define M6530_DB_PINS   (0xFF0000ULL)
#define M6530_RW        (1ULL<<M6530_PIN_RW)
#define M6530_IRQ       (1ULL<<M6530_PIN_IRQ)
#define M6530_PA0       (1ULL<<M6530_PIN_PA0)
#define M6530_PA1       (1ULL<<M6530_PIN_PA1)
#define M6530_PA2       (1ULL<<M6530_PIN_PA2)
#define M6530_PA3       (1ULL<<M6530_PIN_PA3)
#define M6530_PA4       (1ULL<<M6530_PIN_PA4)
#define M6530_PA5       (1ULL<<M6530_PIN_PA5)
#define M6530_PA6       (1ULL<<M6530_PIN_PA6)
#define M6530_PA7       (1ULL<<M6530_PIN_PA7)
#define M6530_PA_PINS   (M6530_PA0|M6530_PA1|M6530_PA2|M6530_PA3|M6530_PA4|M6530_PA5|M6530_PA6|M6530_PA7)
#define M6530_PB0       (1ULL<<M6530_PIN_PB0)
#define M6530_PB1       (1ULL<<M6530_PIN_PB1)
#define M6530_PB2       (1ULL<<M6530_PIN_PB2)
#define M6530_PB3       (1ULL<<M6530_PIN_PB3)
#define M6530_PB4       (1ULL<<M6530_PIN_PB4)
#define M6530_PB5       (1ULL<<M6530_PIN_PB5)
#define M6530_PB6       (1ULL<<M6530_PIN_PB6)
#define M6530_PB7       (1ULL<<M6530_PIN_PB7)
#define M6530_PB_PINS   (M6530_PB0|M6530_PB1|M6530_PB2|M6530_PB3|M6530_PB4|M6530_PB5|M6530_PB6|M6530_PB7)


// I/O port state
typedef struct {
    uint8_t inpr;
    uint8_t outr;
    uint8_t ddr;
    uint8_t pins;
    bool c1_in;
    bool c1_out;
    bool c1_triggered;
    bool c2_in;
    bool c2_out;
    bool c2_triggered;
} m6530_port_t;

// timer state
typedef struct {
    uint16_t latch;     /* 16-bit initial value latch, NOTE: T2 only has an 8-bit latch */
    uint16_t counter;   /* 16-bit counter */
    bool t_bit;         /* toggles between true and false when counter underflows */
    bool t_out;         /* true for 1 cycle when counter underflow */
    /* merged delay-pipelines:
        2-cycle 'counter active':   bits 0..7
        1-cycle 'force load':       bits 8..16
    */
    uint16_t pip;
} m6530_timer_t;

// interrupt state (same as m6522_int_t)
typedef struct {
    uint8_t ier;            /* interrupt enable register */
    uint8_t ifr;            /* interrupt flag register */
    uint16_t pip;
} m6530_int_t;

// m6530 state
typedef struct {
    m6530_port_t pa;
    m6530_port_t pb;
    m6530_timer_t t1;
    m6530_int_t intr;
    uint8_t acr;        /* auxilary control register */
    uint8_t pcr;        /* peripheral control register */
    uint64_t pins;
} m6530_t;

// extract 8-bit data bus from 64-bit pins
#define M6530_GET_DATA(p) ((uint8_t)((p)>>16))
// merge 8-bit data bus value into 64-bit pins
#define M6530_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
// extract port A pins
#define M6530_GET_PA(p) ((uint8_t)((p)>>24))
// extract port B pins
#define M6530_GET_PB(p) ((uint8_t)((p)>>32))
// merge port A pins into pin mask
#define M6530_SET_PA(p,a) {p=((p)&0xFFFFFFFF00FFFFFFULL)|(((a)&0xFFULL)<<24);}
// merge port B pins into pin mask
#define M6530_SET_PB(p,b) {p=((p)&0xFFFFFF00FFFFFFFFULL)|(((b)&0xFFULL)<<32);}
// merge port A and B pins into pin mask
#define M6530_SET_PAB(p,a,b) {p=((p)&0xFFFFFF0000FFFFFFULL)|(((a)&0xFFULL)<<24)|(((b)&0xFFULL)<<32);}

#ifdef __cplusplus
} // extern "C"
#endif

/*-- IMPLEMENTATION ----------------------------------------------------------*/
#ifdef CHIPS_IMPL
#include <string.h>
#ifndef CHIPS_ASSERT
    #include <assert.h>
    #define CHIPS_ASSERT(c) assert(c)
#endif

#endif /* CHIPS_IMPL */