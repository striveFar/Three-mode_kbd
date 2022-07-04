#ifndef PROGMEM_H
#define PROGMEM_H 1

#if defined(__AVR__)
#   include <avr/pgmspace.h>
#elif defined(__arm__)
#   define PROGMEM
#   define pgm_read_byte(p)     *((unsigned char*)p)
#   define pgm_read_word(p)     *((uint16_t*)p)
#endif

#if defined(__RISCV__)
#   include "CH583SFR.h"
#   define PROGMEM
#   define pgm_read_byte(p)     *((UINT8*)p)
#   define pgm_read_word(p)     *((UINT16*)p)
#endif

#endif
