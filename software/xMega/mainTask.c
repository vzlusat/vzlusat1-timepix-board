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
#include "equalization.h"

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

void sendString(char * in) {
	
	strcpy(outcomingPacket->data, in);
	outcomingPacket->length = strlen(in);
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
}

void medipixInit() {
	
	/*
	char temp[40];
	sprintf(temp, "%d\r\n", pgm_read_byte_far(GET_FAR_ADDRESS(pseudo2Count2high) + 8192));
	strcpy(outcomingPacket->data, temp);
	outcomingPacket->length = strlen(temp);
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
	*/
		
	pwrOnMedipix();	

	loadEqualization(&dataBuffer, &ioBuffer);
	
	eraseMatrix();
	
	#if DEBUG_OUTPUT == 1
	sendBlankLine(15, 16);
	#endif
	
	sendString("Medipix ON\r\n");

	#if DEBUG_OUTPUT == 1
	sendBlankLine(15, 16);
	#endif
}

void measure(uint16_t thr, uint16_t time, uint8_t bias, uint8_t mode) {
		
	medipixMode = mode;
	
	setDACs(thr);
	
	setBias(bias);
		
	openShutter();
	
	vTaskDelay(time);
	
	closeShutter();
		
	readMatrix();
	
	sendString("Measuring done\r\n");
	
	vTaskDelay(20);
	
	// výpis
	char temp[40];
	if (medipixMode == MODE_MEDIPIX)
		sprintf(temp, "Tht %d Exp %d Bia %d Mode Mpx\r\n", thr, time, bias);
	else
		sprintf(temp, "Tht %d Exp %d Bia %d Mode Tpx\r\n", thr, time, bias);
	strcpy(outcomingPacket->data, temp);
	outcomingPacket->length = strlen(temp);
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
}

medipixStop() {
	
	pwrOffMedipix();
}

/* -------------------------------------------------------------------- */
/*	The main task														*/
/* -------------------------------------------------------------------- */
void mainTask(void *p) {
	
	/* The variable used to receive from the queue. */
	xCSPStackEvent_t xReceivedEvent;
	
	outcomingPacket = csp_buffer_get(CSP_PACKET_SIZE);
	
	char inChar;
	
	uint16_t thrIn = 290;
	uint16_t timeIn = 10;
	uint8_t biasIn = 203;
	uint8_t modeIn = 0;
	
	uint8_t * ptr;
					
	// infinite while loop of the program 
	while (1) {
			
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
					
					if (((csp_packet_t *) xReceivedEvent.pvData)->data[0] == 1) {
						
						medipixInit();
						
					} else if (((csp_packet_t *) xReceivedEvent.pvData)->data[0] == 2) {
						
						medipixStop();
						
					} else if (((csp_packet_t *) xReceivedEvent.pvData)->data[0] == 3) {

						ptr = &thrIn;
						*(ptr++) = ((csp_packet_t *) xReceivedEvent.pvData)->data[1];
						*ptr = ((csp_packet_t *) xReceivedEvent.pvData)->data[2];
						
						ptr = &timeIn;
						*(ptr++) = ((csp_packet_t *) xReceivedEvent.pvData)->data[3];
						*ptr = ((csp_packet_t *) xReceivedEvent.pvData)->data[4];
						
						ptr = &biasIn;
						*ptr = ((csp_packet_t *) xReceivedEvent.pvData)->data[5];
						
						modeIn = ((csp_packet_t *) xReceivedEvent.pvData)->data[6];
																	
						measure(thrIn, timeIn, biasIn, modeIn);
					}
							
				break;
		
				default :
					/* Should not get here. */
				break;
			}
		}
	}
}