/*
 * medipix.c
 *
 * Created: 25.1.2015 20:11:33
 *  Author: klaxalk
 */ 

#include "system.h"
#include "medipix.h"

// 1 if medipix is powered
// 0 if medipix is turned off
char medipixOnline;

char medipixPowered() {
	
	return medipixOnline;
}

void pwrOnMedipix() {
	
	ioport_set_pin_level(MEDIPIX_PWR, true);
	medipixOnline = 1;
}

void pwrOffMedipix() {
	
	ioport_set_pin_level(MEDIPIX_PWR, false);
	medipixOnline = 0;
}

void pwrToggleMedipix() {
	
	if (medipixOnline == 1) {
		
		pwrOffMedipix();
		
	} else {
		
		pwrOnMedipix();	
	}
}