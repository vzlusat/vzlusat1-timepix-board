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
#include "ADT7420.h"
#include "imageProcessing.h"
#include "fram_mapping.h"

csp_packet_t * outcomingPacket;
xQueueHandle * xCSPEventQueue;

unsigned int dest_p;
unsigned int source_p;

char temp[40];

/* The variable used to receive from the queue. */
xCSPStackEvent_t xReceivedEvent;

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

void sendString(char * in) {
	
	vTaskDelay(30);
	
	strcpy(outcomingPacket->data, in);
	outcomingPacket->length = strlen(in);
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
}

void replyOk() {
	
	vTaskDelay(20);
		
	outcomingPacket->data[0] = 'O';
	outcomingPacket->data[1] = 'K';
	outcomingPacket->data[2] = '\r';
	outcomingPacket->data[3] = '\n';
	
	outcomingPacket->length = 4;
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
}

void medipixInit() {
		
	pwrOnMedipix();	

	loadEqualization(&dataBuffer, &ioBuffer);
	
	eraseMatrix();
}

void measure() {
	
	setDACs(imageParameters.threshold);
	
	setBias(imageParameters.bias);
		
	openShutter();
	
	vTaskDelay(imageParameters.exposure);
	
	closeShutter();
		
	readMatrix();
	
	vTaskDelay(20);

	filterOnePixelEvents();

	computeImageStatistics();
	
	// do binning
	if (imageParameters.outputForm >= BINNING_1 &&imageParameters.outputForm <= BINNING_32) {
		
		applyBinning();
		
	} else if (imageParameters.outputForm == HISTOGRAMS) {
		
		createHistograms();
	}
	
	imageParameters.imageId++;
	
	saveImageParametersToFram();
}

void medipixStop() {
	
	pwrOffMedipix();
}

void updateMedipixMode() {
			
	loadEqualization(&dataBuffer, &ioBuffer);
	
	vTaskDelay(50);
		
	eraseMatrix();
}

void sendImageInfo() {
	
	// 'A' means the first packet of the image message
	outcomingPacket->data[0] = 'A';
	
	// load current info from fram
	loadImageParametersFromFram();
	
	// save current info to the packet
	memcpy(outcomingPacket->data+1, &imageParameters, sizeof(imageParameters));
	
	// set the size of the packet
	outcomingPacket->length = 1+sizeof(imageParameters);
	
	// send the final packet
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
}

void waitForAck() {
		
	xQueueReceive(xCSPEventQueue, &xReceivedEvent, 1000);
}

uint16_t parseUint16(uint8_t * buffer) {
	
	uint16_t value;
	uint8_t * tempPtr;
	
	tempPtr = (uint8_t *) &value;
	
	*tempPtr = buffer[0];
	*(tempPtr+1) = buffer[1];
	
	return value;
}

void saveUint16(uint8_t * buffer, uint16_t value) {
	
	uint8_t * tempPtr = &value;
	
	*buffer = *tempPtr;
	*(buffer+1) = *(tempPtr+1);
}

// image == 0 -> original
// image == 0 -> compressed
uint8_t sendCompressed(uint8_t image) {
	
	uint8_t (*getPixel)(uint8_t, uint8_t);
	
	if (image == 0)
		getPixel = &getRawPixel;
	else
		getPixel = &getFilteredPixel;
	
	sendImageInfo();
	
	vTaskDelay(50);
	
	uint16_t i, j;
	
	uint8_t packetPointer, tempPixel, numPixelsInPacket;
	
	if (imageParameters.nonZeroPixelsOriginal == 0)
		return;
	
	// initialize the first packet
	outcomingPacket->data[0] = 'B';
	saveUint16(outcomingPacket->data+1, imageParameters.imageId);
	packetPointer = 4;
	numPixelsInPacket = 0;
	
	for (i = 0; i < 256; i++) {
		
		for (j = 0; j < 256; j++) {
			
			tempPixel = getPixel(i, j);
			
			// the pixel will be send
			if (tempPixel > 0) {
				
				// there is still a place in the packet
				if (packetPointer <= 61) {
					
					saveUint16(outcomingPacket->data + packetPointer, i*256+j);
					packetPointer += 2;
					*(outcomingPacket->data + packetPointer++) = tempPixel;
					numPixelsInPacket++;
				}
				
				// the packet is full, send it
				if (packetPointer > 61) {
					
					outcomingPacket->data[3] = numPixelsInPacket;
					outcomingPacket->length = packetPointer;
					csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
					
					waitForAck();
					
					outcomingPacket->data[0] = 'B';
					saveUint16(outcomingPacket->data+1, imageParameters.imageId);
					packetPointer = 4;
					numPixelsInPacket = 0;
				}
			}
		}
	}
	
	// send the last data packet
	if (packetPointer > 4) {
		
		outcomingPacket->length = packetPointer;
		outcomingPacket->data[3] = numPixelsInPacket;
		csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
	}
	
	waitForAck();
	
	// send the terminal packet
	outcomingPacket->data[0] = 'C';
	outcomingPacket->length = 1;
	
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
					
	waitForAck();
}

void sendPostProcessed() {
	
	uint16_t i, j, x, y;
	
	uint16_t sum;
	
	uint8_t numPerLine;
	
	switch (imageParameters.outputForm) {
		
		case BINNING_8:
		numPerLine = 32;
		break;
		
		case BINNING_16:
		numPerLine = 16;
		break;
		
		case BINNING_32:
		numPerLine = 8;
		break;
	}
	
	sendImageInfo();
	
	vTaskDelay(50);
	
	
}

/* -------------------------------------------------------------------- */
/*	The main task														*/
/* -------------------------------------------------------------------- */
void mainTask(void *p) {
	
	outcomingPacket = csp_buffer_get(CSP_PACKET_SIZE);
	
	char inChar;
	
	uint8_t * ptr;
					
	// infinite while loop of the program 
	while (1) {
			
		if (xQueueReceive(xCSPEventQueue, &xReceivedEvent, 1)) {
		
			switch( xReceivedEvent.eEventType ) {
			
				// sends the info about the system
				case housKeepingEvent :
				
					houseKeeping(xReceivedEvent.pvData);
			
				break;
				
				// sends the info about the system
				case medipixEvent :
					
					dest_p = ((csp_packet_t *) (xReceivedEvent.pvData))->id.sport;
					source_p = ((csp_packet_t *) (xReceivedEvent.pvData))->id.dport;
					
					uint8_t command = ((csp_packet_t *) xReceivedEvent.pvData)->data[0];
					uint8_t * packetPayload = ((csp_packet_t *) xReceivedEvent.pvData)->data+1;
					
					switch (command) {
						
						case MEDIPIX_PWR_ON:
							
							medipixInit();
							
							replyOk();
							
						break;
						
						case MEDIPIX_PWR_OFF:
						
							medipixStop();
						
							replyOk();
							
						break;
						
						case MEDIPIX_SET_ALL_PARAMS:
						
							loadImageParametersFromFram();
						
							newSettings_t * settings = packetPayload;
							
							imageParameters.threshold = settings->treshold;
							imageParameters.exposure = settings->exposure;
							imageParameters.bias = settings->bias;
							imageParameters.filtering = settings->filtering;
							imageParameters.mode = settings->mode;
							imageParameters.outputForm = settings->outputForm;
						
							saveImageParametersToFram();
							
							replyOk();
							
						break;
						
						case MEDIPIX_SET_THRESHOLD:
						
							loadImageParametersFromFram();

							imageParameters.threshold = parseUint16(packetPayload);
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;
						
						case MEDIPIX_SET_BIAS:
						
							loadImageParametersFromFram();

							imageParameters.bias = *packetPayload;
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;
						
						case MEDIPIX_SET_EXPOSURE:
						
							loadImageParametersFromFram();

							imageParameters.exposure = parseUint16(packetPayload);
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;
						
						case MEDIPIX_SET_FILTERING:
						
							loadImageParametersFromFram();

							imageParameters.filtering = *packetPayload;
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;
						
						case MEDIPIX_SET_MODE:
						
							loadImageParametersFromFram();

							// medipix was powered, need to reload the equalization							
							if (medipixPowered() == 1) {
								
								// if the mode is different, reload the equalization
								if (imageParameters.mode != *packetPayload) {
									
									updateMedipixMode();
								}
							}

							imageParameters.mode = *packetPayload;
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;
						
						case MEDIPIX_SET_OUTPUT_FORM:
						
							loadImageParametersFromFram();

							imageParameters.outputForm = *packetPayload;
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;
						
						case MEDIPIX_MEASURE:

							loadImageParametersFromFram();
							
							measure();
							
							replyOk();
						
						break;
						
						case MEDIPIX_MEASURE_WITH_PARAMETERS:

							loadImageParametersFromFram();
							
							measure();
						
						break;
						
						case MEDIPIX_SEND_ORIGINAL:
						
							sendCompressed(0);
						
						break;
						
						case MEDIPIX_SEND_FILTERED:
						
							sendCompressed(1);
						
						break;
						
						case MEDIPIX_SEND_BINNED:
						
							sendPostProcessed();
						
						break;	
						
						case MEDIPIX_SEND_METADATA:
							
							sendImageInfo();
						
						break;
					}
							
				break;
		
				default :
					/* Should not get here. */
				break;
			}
		}
	}
}