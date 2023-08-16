#ifndef MMATH_H
#define MMATH_H

#include <stdint.h>

/*
* Mihir Math
*
* Some whack math, don't worry about it
*/

/* Currently never planning on finishing */
int16_t mm_tan(int16_t x);
int16_t mm_cot(int16_t x);

/* Table lookups */
uint16_t mm_tant(uint16_t x);
uint16_t mm_cott(uint16_t x);
uint16_t mm_sect(uint16_t x);
uint16_t mm_csct(uint16_t x);
uint16_t mm_sint(uint16_t x);
uint16_t mm_cost(uint16_t x);

#endif