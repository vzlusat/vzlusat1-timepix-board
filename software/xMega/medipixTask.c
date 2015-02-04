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
#include "medipixTask.h"
#include "medipix.h"

/* -------------------------------------------------------------------- */
/*	The Medipix task													*/
/* -------------------------------------------------------------------- */
void medipixTask(void *p) {
			
	medipixActionMessage_t medipixActionMessage;
	
	csp_packet_t * outcomingPacket = csp_buffer_get(CSP_PACKET_SIZE);
	
	char incomingBuffer[448];
	char decodedBuffer[448];
	
	unsigned int dest_p;
	unsigned int source_p;
	
	int16_t actIncomingPosition = 0;
	
	char taskState = 0;
	
	char inChar;
	
	while (1) {
		
		if (xQueueReceive(medipixActionQueue, &medipixActionMessage, 10)) {
			
			if (medipixActionMessage.action == PWR_ON) {
				
				pwrOnMedipix();
				
			} else if (medipixActionMessage.action == PWR_OFF) {
				
				pwrOffMedipix();
				
			} else if (medipixActionMessage.action == PWR_TOGGLE) {
			
				pwrToggleMedipix();
			} else if (medipixActionMessage.action == MEASURE) {
				
				taskState = 1;
			}
		}
		
		if (taskState == 1) {
			
			pwrOnMedipix();
			vTaskDelay(3000);
			
			usartBufferPutByte(medipix_usart_buffer, 'm', 10);
			
			actIncomingPosition = 0;
			
			while (usartBufferGetByte(medipix_usart_buffer, &inChar, 10)) {
				
				incomingBuffer[actIncomingPosition++] = inChar;
				
				if (actIncomingPosition == 32) {
					
					actIncomingPosition = 0;
					memcpy(outcomingPacket->data, &incomingBuffer, sizeof(char)*32);
					outcomingPacket->length = 32;
					csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
				}
			}
		}
	}
}

/*
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
*/