/*
 * mainTask.c
 *
 * Created: 11.10.2014 20:18:00
 *  Author: Tomas Baca
 */

#include "mainTask.h"
#include "cspTask.h"
#include "system.h"
#include "medipix.h"

csp_packet_t * outcomingPacket;
xQueueHandle * xCSPEventQueue;

unsigned int dest_p;
unsigned int source_p;

/* -------------------------------------------------------------------- */
/*	Reply the free heap space in human readable form					*/
/* -------------------------------------------------------------------- */
int sendFreeHeapSpace(csp_packet_t * inPacket) {
	
	char msg[20];
	itoa(xPortGetFreeHeapSize(), msg, 10);
	
	/* Copy message to packet */
	strcpy(outcomingPacket->data, msg);
	outcomingPacket->length = strlen(msg);

	/* Send packet */
	if (csp_sendto(CSP_PRIO_NORM, inPacket->id.src, inPacket->id.sport, inPacket->id.dport, CSP_O_NONE, outcomingPacket, 1000) == CSP_ERR_NONE) {
		/* Send succeeded */
		led_red_toggle();
		} else {
		/* Send failed */
	}

	return 0;
}

/* -------------------------------------------------------------------- */
/*	Reply with some status info message									*/
/* -------------------------------------------------------------------- */
int houseKeeping(csp_packet_t * inPacket) {
	
	// put the info message into the packet
	char msg[64];
	memset(msg, 0, 64);
	sprintf(msg, "Timepix Board\n\rUptime: %id %ih %im %ds\n\r\000", (int16_t) hoursTimer/24, (int16_t) hoursTimer%24, (int16_t) secondsTimer/60, (int16_t) secondsTimer%60);

	memset(outcomingPacket->data, 0, sizeof(outcomingPacket->data));
	strcpy(outcomingPacket->data, msg);
	outcomingPacket->length = strlen(msg)+1;

	/* Send packet */
	if (csp_sendto(CSP_PRIO_NORM, inPacket->id.src, inPacket->id.sport, inPacket->id.dport, CSP_O_NONE, outcomingPacket, 1000) == CSP_ERR_NONE) {
		
		/* Send succeeded */
		led_red_toggle();
	} else {
		/* Send failed */
	}

	return 0;
	
}

/* -------------------------------------------------------------------- */
/*	Sends back the incoming packet										*/
/* -------------------------------------------------------------------- */
int echoBack(csp_packet_t * inPacket) {

	/* Send packet */
	// reuses the incoming packet for the response
	if (csp_sendto(CSP_PRIO_NORM, inPacket->id.src, inPacket->id.sport, inPacket->id.dport, CSP_O_NONE, inPacket, 1000) == CSP_ERR_NONE) {
		/* Send succeeded */
		led_red_toggle();
		} else {
		/* Send failed */
	}

	return 0;
}

void sendBlankLine(unsigned int dport, unsigned int sport) {
	
	outcomingPacket->data[0] = '\n';
	outcomingPacket->data[1] = '\n';
	outcomingPacket->data[2] = '\r';
	outcomingPacket->data[3] = 0;
	outcomingPacket->length = 4;
	csp_sendto(CSP_PRIO_NORM, 1, dport, sport, CSP_O_NONE, outcomingPacket, 1000);
	vTaskDelay(20);
}

void interpretRow() {
	
	
}

/* -------------------------------------------------------------------- */
/*	The main task														*/
/* -------------------------------------------------------------------- */
void mainTask(void *p) {
	
	/* The variable used to receive from the queue. */
	xCSPStackEvent_t xReceivedEvent;
	
	outcomingPacket = csp_buffer_get(CSP_PACKET_SIZE);
	
	char inChar;
					
	// infinite while loop of the program 
	while (1) {

		//if (measuringProceeding == 0) {
			
			if (xQueueReceive(xCSPEventQueue, &xReceivedEvent, 1)) {
		
				switch( xReceivedEvent.eEventType ) {
		
					// Echo back the whole packet
					// incoming port => outcoming
					case echoBackEvent :
				
						echoBack(xReceivedEvent.pvData);
			
					break;
			
					// sends the info about the system
					case housKeepingEvent :
				
						houseKeeping(xReceivedEvent.pvData);
			
					break;
				
					// sends the info about the system
					case medipixEvent :
					
						dest_p = ((csp_packet_t *) (xReceivedEvent.pvData))->id.sport;
						source_p = ((csp_packet_t *) (xReceivedEvent.pvData))->id.dport;
				
						pwrOnMedipix();			
					break;
		
					default :
						/* Should not get here. */
					break;
				}
			}
			/*
		} else {
			
			// nastartuje medipix
			pwrOnMedipix();
			
			// vytiskne mezeru
			
			sendBlankLine(dest_p, source_p);
			
			vTaskDelay(2000);
			while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
				
				if (inChar == '\r')
				inChar = '<';
				
				if (inChar == '\n')
				inChar = '_';
				
				outcomingPacket->data[0] = inChar;
				outcomingPacket->data[1] = 0;
				outcomingPacket->length = 2;
				csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
				vTaskDelay(20);
			}
						
			measuringProceeding = 0;
			
			// pošle o
			//usartBufferPutByte(medipix_usart_buffer, 'o', 1000);
			
			vTaskDelay(10);
			
			//usartBufferPutByte(medipix_usart_buffer, 'c', 1000);
			/
			while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
					
				if (inChar == '\r')
				inChar = '<';
					
				if (inChar == '\n')
				inChar = '_';
					
				outcomingPacket->data[0] = inChar;
				outcomingPacket->data[1] = '|';
				outcomingPacket->data[2] = 0;
				outcomingPacket->length = 3;
				csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
				vTaskDelay(30);
			}
						
			sendBlankLine(dest_p, source_p);
						
			usartBufferPutByte(medipix_usart_buffer, 'm', 1000);
			
			while (usartBufferGetByte(medipix_usart_buffer, &inChar, 100)) {break;}
			while (usartBufferGetByte(medipix_usart_buffer, &inChar, 100)) {break;}
					
		}
		*/
	}
}