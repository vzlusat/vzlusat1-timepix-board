/*
 * medipixTask.c
 *
 * Created: 2.11.2014 19:00:36
 *  Author: Tomas Baca
 */ 

#include "system.h"
#include "usart_driver_RTOS.h"
#include "mainTask.h"
#include "cspTask.h"

/* -------------------------------------------------------------------- */
/*	The Medipix task													*/
/* -------------------------------------------------------------------- */
void medipixTask(void *p) {
	
	char inChar;
	
	char medipixBuffer[64];
	uint16_t pos = 0;
	
	xCSPStackEvent_t * newEvent;
	
	int i;

	while (1) {
		
		if (usartBufferGetByte(medipix_usart_buffer, &inChar, 10)) {
			
			medipixBuffer[pos++] = inChar;
			
			if ((inChar == '\r') || (pos == 62)) {
				
				medipixBuffer[pos-1] = '\r';
				medipixBuffer[pos] = '\n';
				pos = 0;
				newEvent->eEventType = medipixEvent;
				strcpy(newEvent->pvData, medipixBuffer);
				xQueueSend(xCSPEventQueue, newEvent, 10);
				
				for (i = 0; i < 64; i++) {
					medipixBuffer[i] = 0;
				}
			}
		}
	}
}