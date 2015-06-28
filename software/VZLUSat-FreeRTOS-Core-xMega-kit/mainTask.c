/*
 * mainTask.c
 *
 * Created: 11.10.2014 20:18:00
 *  Author: Tomas Baca
 */

#include "mainTask.h"
#include "cspTask.h"
#include "system.h"
#include "queue.h"
#include "usart_driver_RTOS.h"
#include "medipixHeaders.h"

// to save outcomming time of ping
uint32_t pingSent;

// for ping time difference
int16_t timediff;

uint8_t * ptr;

newSettings_t settings;

/* -------------------------------------------------------------------- */
/*	The main task														*/
/* -------------------------------------------------------------------- */
void mainTask(void *p) {
	
	settings.treshold = 250;
	settings.exposure = 1000;
	settings.bias = 109;
	settings.filtering = FILTERING_ON;
	settings.mode = MODE_MEDIPIX;
	settings.outputForm = BINNING_1;
		
	// packet used to handle incoming communication
	csp_packet_t * outcomingPacket = csp_buffer_get(CSP_PACKET_SIZE);

	// character received from uart
	char inChar;
	
	/* The variable used to receive from the queue. */
	xCSPStackEvent_t xReceivedEvent;
	
	int i; // iterator
	
	// text buffer for uart
	char text[20];
	
	// infinite while loop of the program 
	while (1) {
		
		if (xQueueReceive(xCSPEventQueue, &xReceivedEvent, 1)) {
			
			switch (xReceivedEvent.eEventType) {
			
				case generalCommEvent:
			
					// send its content to the uart
					for (i = 0; i < ((csp_packet_t *) xReceivedEvent.pvData)->length; i++) {
						usartBufferPutByte(pc_usart_buffer, ((csp_packet_t *) xReceivedEvent.pvData)->data[i], 10);
					}
			
				break;
				case pingReceivedEvent:
			
					// calculate the ping return time
					if ((int16_t) milisecondsTimer - (int16_t) pingSent > 0)
						timediff = milisecondsTimer - pingSent;
					else
						timediff = (int16_t) milisecondsTimer - (int16_t) pingSent + (int16_t) 1000;

					itoa(timediff, text, 10);
					usartBufferPutString(pc_usart_buffer, "ping received in ", 10);
					usartBufferPutString(pc_usart_buffer, text, 10);
					usartBufferPutString(pc_usart_buffer, "ms\n\r", 10);
			
				break;
				
				case dataPacketEvent:
				
					// send its content to the uart
					for (i = 0; i < ((csp_packet_t *) xReceivedEvent.pvData)->length; i++) {
						usartBufferPutByte(pc_usart_buffer, ((csp_packet_t *) xReceivedEvent.pvData)->data[i], 10);
					}
					
					usartBufferPutString(pc_usart_buffer, "\r\n", 10);
				
				break;
			
				default:
					/* Should not get here. */
				break;
			}
		}
		
		// if there is something from the uart
		if (usartBufferGetByte(pc_usart_buffer, &inChar, 0)) {
		
			outcomingPacket->data[0] = inChar;
			outcomingPacket->length = 1;
			uint8_t k;
			
			uint8_t * tempPtr = (uint8_t) &settings;
			unsigned char inChar2;
		
			switch (inChar) {
			
				// turn on medipix
				case '0':
					outcomingPacket->data[0] = MEDIPIX_PWR_ON;
					outcomingPacket->length = 1;
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 15, CSP_O_NONE, outcomingPacket, 10);
				break;
			
				// power off medipix
				case '1':
					outcomingPacket->data[0] = MEDIPIX_PWR_OFF;
					outcomingPacket->length = 1;
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 15, CSP_O_NONE, outcomingPacket, 10);
				break;
				
				// set parameters
				case '2':
				
					tempPtr = &settings;
				
					// receive the parameters over the uart
					for (k = 0; k < sizeof(newSettings_t); k++) {
					
						if (usartBufferGetByte(pc_usart_buffer, &inChar, 1000)) {
							
							*(tempPtr+k) = inChar;
						}
					}
				
					outcomingPacket->data[0] = MEDIPIX_SET_ALL_PARAMS;
					
					memcpy(outcomingPacket->data+1, &settings, sizeof(newSettings_t));
					
					outcomingPacket->length = 1+sizeof(newSettings_t);
					
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 15, CSP_O_NONE, outcomingPacket, 10);
					
				break;
				
				// set treshold
				case '3':
				
					usartBufferGetByte(pc_usart_buffer, outcomingPacket->data+1, 1000);
					usartBufferGetByte(pc_usart_buffer, outcomingPacket->data+2, 1000);
											
					outcomingPacket->data[0] = MEDIPIX_SET_THRESHOLD;
					
					outcomingPacket->length = 1+2;
					
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 15, CSP_O_NONE, outcomingPacket, 10);
				
				break;
				
				// set bias
				case '4':
				
					usartBufferGetByte(pc_usart_buffer, outcomingPacket->data+1, 1000);
					
					outcomingPacket->data[0] = MEDIPIX_SET_BIAS;
					
					outcomingPacket->length = 1+1;
					
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 15, CSP_O_NONE, outcomingPacket, 10);
				
				break;
				
				// set exposure
				case '5':
				
					usartBufferGetByte(pc_usart_buffer, outcomingPacket->data+1, 1000);
					usartBufferGetByte(pc_usart_buffer, outcomingPacket->data+2, 1000);
					
					outcomingPacket->data[0] = MEDIPIX_SET_EXPOSURE;
					
					outcomingPacket->length = 1+2;
					
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 15, CSP_O_NONE, outcomingPacket, 10);
				
				break;
				
				// set filtering
				case '6':
				
					usartBufferGetByte(pc_usart_buffer, outcomingPacket->data+1, 1000);
					
					outcomingPacket->data[0] = MEDIPIX_SET_FILTERING;
					
					outcomingPacket->length = 1+1;
					
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 15, CSP_O_NONE, outcomingPacket, 10);
				
				break;
				
				// set mode
				case '7':
				
					usartBufferGetByte(pc_usart_buffer, outcomingPacket->data+1, 1000);
					
					outcomingPacket->data[0] = MEDIPIX_SET_MODE;
					
					outcomingPacket->length = 1+1;
					
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 15, CSP_O_NONE, outcomingPacket, 10);
				
				break;
				
				// set output form
				case '8':
				
					usartBufferGetByte(pc_usart_buffer, outcomingPacket->data+1, 1000);
					
					outcomingPacket->data[0] = MEDIPIX_SET_OUTPUT_FORM;
					
					outcomingPacket->length = 1+1;
					
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 15, CSP_O_NONE, outcomingPacket, 10);
				
				break;
				
				// measure 
				case '9':
				
					outcomingPacket->data[0] = MEDIPIX_MEASURE;
					
					outcomingPacket->length = 1;
					
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 15, CSP_O_NONE, outcomingPacket, 10);
					
				break;
				
				// read original image
				case 'e':
				
					outcomingPacket->data[0] = MEDIPIX_SEND_ORIGINAL;
					
					outcomingPacket->length = 1;
					
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 17, CSP_O_NONE, outcomingPacket, 10);

				break;
				
				// read original image
				case 'r':
				
					outcomingPacket->data[0] = MEDIPIX_SEND_FILTERED;
					
					outcomingPacket->length = 1;
					
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 17, CSP_O_NONE, outcomingPacket, 10);

				break;
				
				// read the image metadata
				case 't':
					outcomingPacket->data[0] = MEDIPIX_SEND_METADATA;
					outcomingPacket->length = 1;
					pingSent = milisecondsTimer;
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 16, 17, CSP_O_NONE, outcomingPacket, 10);
				break;
			 
				// ping
				case 'p':	
					outcomingPacket->data[0] = 0;
					outcomingPacket->length = 1;
					pingSent = milisecondsTimer;
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 1, 32, CSP_O_NONE, outcomingPacket, 10);
				break;
				
				// ask board for status
				case 'h':
					csp_sendto(CSP_PRIO_NORM, CSP_BOARD_ADDRESS, 17, 15, CSP_O_NONE,  outcomingPacket, 10);
				break;
			
				// sends the char and is supposed to receive it back
				default:
				break;
			}
		}
	}
}