/*
 * equalization.h
 * 
 * EQUALIZACE PRO LETOVY KUS
 */ 

#ifndef EQUALIZATION_H_
#define EQUALIZATION_H_

#include "system.h"

#define GET_FAR_ADDRESS(var)                          \
({                                                    \
	uint_farptr_t tmp;                                \
	\
	__asm__ __volatile__(                             \
	\
	"ldi    %A0, lo8(%1)"           "\n\t"    \
	"ldi    %B0, hi8(%1)"           "\n\t"    \
	"ldi    %C0, hh8(%1)"           "\n\t"    \
	"clr    %D0"                    "\n\t"    \
	:                                             \
	"=d" (tmp)                                \
	:                                             \
	"p"  (&(var))                             \
	);                                                \
	tmp;                                              \
})	

const uint8_t pseudoCount2high[8192];
const uint8_t pseudoCount2low[8192];
const uint8_t pseudoCount1high[8192];
const uint8_t pseudoCount1low[8192];

const uint8_t equalization8[8192];
const uint8_t equalization7[8192];
const uint8_t equalization6[8192];
const uint8_t equalization5[8192];
const uint8_t equalization4[8192];
const uint8_t equalization3[8192];
const uint8_t equalization2[8192];
const uint8_t equalization1[8192];

const uint32_t crc_tab[256];

#endif /* EQUALIZATION_H_ */
