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
#include "pseudoTables.h"

csp_packet_t * outcomingPacket;
xQueueHandle * xCSPEventQueue;

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

// Number of bytes in stream containing one MXR row
#define MPXMXR_ROW_BYTES (32*14)

// Deserialization of data stream from one MXR chip
// No dummy byte is expected:

void MpxBitStream2DataSingleMXR(char * byteStream, uint16_t * data, int16_t * rowByteIdx) {
	
	int16_t j, k;
	int16_t byteIdx;
	uint16_t bitMsk, valMsk;

	for (j = 255; j >= 0; j--) {                                                 // Loop for 256 pixels of i-th row
		
		byteIdx = *rowByteIdx + (j>>3);                                            // index of first byte in bytestream which contains data bits for [i,j] element in data matrix
		bitMsk = 1 << ((~j) & 0x7);                                                // mask of bit for [i,j] element in byte on byteIdx, bits are in reverse order (=> ~j instead of j)
		for (k=0, *data=0, valMsk=1; k<14; k++, valMsk <<=1, byteIdx-=32)         // Loop for 13 bits of j-th pixel in i-th row
			if (byteStream[byteIdx] & bitMsk)
			
		(*data) |= valMsk;
		data++;
	}
		
	*rowByteIdx += MPXMXR_ROW_BYTES;
}

// Derandomization for MXR:
int16_t MpxConvertValuesMXR(uint16_t * values){

	int i;
	int16_t sum = 0;
	
	for (i = 0; i < 256; i++){
		
		if (*values < 16384) {
			
			if (*values < 8192)
				*values = pgm_read_word(&(pseudo2Count1[*values]));
			else
				*values = pgm_read_word(&(pseudo2Count2[*values - 8192]));
				
			sum += *values;
		} else {
			*values = 0xFFFF;
		}
		
		values++;
	}
	return sum;
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
	
	unsigned int dest_p;
	unsigned int source_p;
	
	unsigned char incomingBuffer[448];
	unsigned char overBuffer[256];
	uint16_t decodedBuffer[256];
	
	int16_t actIncomingPosition = 0;
	
	char measuringProceeding = 0;
	
	char temp[40];
	
	char inChar;
	
	int16_t rowByteIdx;
	
	// infinite while loop of the program 
	while (1) {

		if (measuringProceeding == 0) {
			
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
				
						measuringProceeding = 1;
									
					break;
		
					default :
						/* Should not get here. */
					break;
				}
			}
			
		} else {
			
			// nastartuje medipix
			pwrOnMedipix();
			
			// pocka az nastartuje
			
			sendBlankLine(dest_p, source_p);
			
			vTaskDelay(2000);
			while (usartBufferGetByte(medipix_usart_buffer, &inChar, 2000)) {
				
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
			
			// vytiskne mezeru
			
			sendBlankLine(dest_p, source_p);
			
			// pošle o
			usartBufferPutByte(medipix_usart_buffer, 'o', 1000);
			
			vTaskDelay(10);
			
			usartBufferPutByte(medipix_usart_buffer, 'c', 1000);
				
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
					
			int16_t bytesInBuffer = 0;
			int16_t bytesInOverBuffer = 0;
			char bufferFull = 0;
			int16_t receivedBytes = 2;
			int16_t rowsReceived = 0;
			
			while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
				
				receivedBytes++;
				
				if (!bufferFull) {
					incomingBuffer[bytesInBuffer++] = inChar;
					
					if (bytesInBuffer == 448)
						bufferFull = 1;
						
				} else {
					overBuffer[bytesInOverBuffer++] = inChar;
				}
				
				if (receivedBytes == 256 || (rowsReceived == 255 && bufferFull && receivedBytes == 36)) {
					
					receivedBytes = 0;
				
					if (!bufferFull) {
						
						usartBufferPutByte(medipix_usart_buffer, 'i', 1000);	
					} else {
						
						// decode the message
						
						rowByteIdx = 32*13;
						
						MpxBitStream2DataSingleMXR(&incomingBuffer, &decodedBuffer, &rowByteIdx);
						
						int16_t rowSum = MpxConvertValuesMXR(&decodedBuffer);
						
						// copy overbuffer to main buffer
						memcpy(incomingBuffer, overBuffer, bytesInOverBuffer);
						bytesInBuffer = bytesInOverBuffer;
						bytesInOverBuffer = 0;
						bufferFull = 0;
						rowsReceived++;
						
						sprintf(temp, "Row %d, sum %d, first %d\n\r", rowsReceived, rowSum, decodedBuffer[0]);
						strcpy(outcomingPacket->data, temp);
						outcomingPacket->length = strlen(temp);
						csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
						
						if (rowsReceived == 256) {
							
							vTaskDelay(50);
							memcpy(outcomingPacket->data, overBuffer+32, 2);
							outcomingPacket->length = 2;
							csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);

							measuringProceeding = 0;
						} else
							usartBufferPutByte(medipix_usart_buffer, 'i', 1000);
					}
				}
			}
		}
	}
}