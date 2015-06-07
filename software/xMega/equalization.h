/*
 * equalization.h
 * 
 * EQUALIZACE PRO LETOVY KUS
 */ 

#ifndef EQUALIZATION_H_
#define EQUALIZATION_H_

#include <avr/io.h>
#include <avr/pgmspace.h>

#if MEDIPIX_VERSION == FLIGHT

const uint8_t equalization1[8192] PROGMEM;
const uint8_t equalization2[8192] PROGMEM;
const uint8_t equalization3[8192] PROGMEM;
const uint8_t equalization4[8192] PROGMEM;
const uint8_t equalization5[8192] PROGMEM;
const uint8_t equalization6[8192] PROGMEM;
const uint8_t equalization7[8192] PROGMEM;
const uint8_t equalization8[8192] PROGMEM;

const uint8_t pseudoCount1low[8192] PROGMEM;
const uint8_t pseudoCount1high[8192] PROGMEM;
const uint8_t pseudoCount2low[8192] PROGMEM;
const uint8_t pseudoCount2high[8192] PROGMEM;

#elif MEDIPIX_VERSION == EQM

const uint8_t equalization21[8192] PROGMEM;
const uint8_t equalization22[8192] PROGMEM;
const uint8_t equalization23[8192] PROGMEM;
const uint8_t equalization24[8192] PROGMEM;
const uint8_t equalization25[8192] PROGMEM;
const uint8_t equalization26[8192] PROGMEM;
const uint8_t equalization27[8192] PROGMEM;
const uint8_t equalization28[8192] PROGMEM;

const uint8_t pseudo2Count1low[8192] PROGMEM;
const uint8_t pseudo2Count1high[8192] PROGMEM;
const uint8_t pseudo2Count2low[8192] PROGMEM;
const uint8_t pseudo2Count2high[8192] PROGMEM;

#endif

#endif /* EQUALIZATION_H_ */
