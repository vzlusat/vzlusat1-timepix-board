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
				
			if (*values < 100)
				sum += *values;
		} else {
			*values = 0xFFFF;
		}
		
		values++;
	}
	return sum;
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
	unsigned char overBuffer[64];
	uint16_t decodedBuffer[256];
	
	int16_t actIncomingPosition = 0;
	
	char measuringProceeding = 0;
	
	int16_t pocitacRadku = 0;
	
	char firstRow = 0;
	
	char temp[20];
	
	char inChar;
	
	int16_t baf;
	
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
			
			vTaskDelay(2000);
			
			usartBufferPutByte(medipix_usart_buffer, 'o', 10);
			
			vTaskDelay(1000);
			
			usartBufferPutByte(medipix_usart_buffer, 'c', 10);
			
			vTaskDelay(10);
						
			// send 'm'
			usartBufferPutByte(medipix_usart_buffer, 'm', 10);
						
			// skip over the dummy byte
			while (usartBufferGetByte(medipix_usart_buffer, &inChar, 100)) {break;};
				
			firstRow = 1;
			actIncomingPosition = 0;
			pocitacRadku = 0;
			
			while (usartBufferGetByte(medipix_usart_buffer, &inChar, 100)) {
				
				if (actIncomingPosition < 448)
					incomingBuffer[actIncomingPosition++] = inChar;
				else
					overBuffer[actIncomingPosition++ - 448] = inChar;
					
				if (firstRow == 1 && actIncomingPosition == 256) {
					
					usartBufferPutByte(medipix_usart_buffer, 'i', 10);
					firstRow = 0;	
				} else if (firstRow == 0 && actIncomingPosition == 320) {
					usartBufferPutByte(medipix_usart_buffer, 'i', 10);
				}
				
				if (actIncomingPosition == 512) {
					
					actIncomingPosition = 64;
					pocitacRadku++;	
					
					rowByteIdx = 32*13;
					MpxBitStream2DataSingleMXR(&incomingBuffer, &decodedBuffer, &rowByteIdx);
										
					baf = MpxConvertValuesMXR(&decodedBuffer);
					
					memcpy(incomingBuffer, overBuffer, sizeof(uint16_t)*64);
					
					sprintf(temp, "%d %d %d\n\r", pocitacRadku, baf, decodedBuffer[0]);
					
					strcpy(outcomingPacket->data, temp);
					
					// memcpy(outcomingPacket->data, (char*) &decodedBuffer, 20);
					outcomingPacket->length = strlen(temp);
					csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
					
					if (pocitacRadku < 256)
						usartBufferPutByte(medipix_usart_buffer, 'o', 10);
					
					if (pocitacRadku == 256) {
						
						pwrOffMedipix();
						measuringProceeding = 0;
						break;
					}
				}
			}
		}
	}
}