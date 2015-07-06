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
#include "errorCodes.h"
#include "medipix.h"
#include "adtTask.h"

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
void sendFreeHeapSpace() {
	
	char msg[20];
	itoa(xPortGetFreeHeapSize(), msg, 10);
	
	/* Copy message to packet */
	strcpy(outcomingPacket->data, msg);
	outcomingPacket->length = strlen(msg);

	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
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
	
	vTaskDelay(40);
	
	strcpy(outcomingPacket->data, in);
	outcomingPacket->length = strlen(in);
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
}

void replyOk() {
	
	vTaskDelay(40);
		
	outcomingPacket->data[0] = 'O';
	outcomingPacket->data[1] = 'K';
	outcomingPacket->data[2] = '\r';
	outcomingPacket->data[3] = '\n';
	
	outcomingPacket->length = 4;
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
	
	vTaskDelay(40);
}

void replyErr(uint8_t error) {
	
	vTaskDelay(40);
	
	outcomingPacket->data[0] = 'E';
	outcomingPacket->data[1] = error;
	outcomingPacket->data[2] = '\r';
	outcomingPacket->data[3] = '\n';
	
	outcomingPacket->length = 4;
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
	
	vTaskDelay(40);
}

void medipixInit() {
	
	loadImageParametersFromFram();
		
	pwrOnMedipix();	

	loadEqualization(&dataBuffer, &ioBuffer);
	
	eraseMatrix();
	
	setDACs(imageParameters.threshold);
	
	setBias(imageParameters.bias);
}

void medipixStop() {
	
	pwrOffMedipix();
}

void sendImageInfo() {
	
	// 'A' means the first packet of the image message
	outcomingPacket->data[0] = 'A';
	
	// load current info from fram
	loadImageParametersFromFram();
	
	// save current info to the packet
	memcpy(outcomingPacket->data+1, &imageParameters, sizeof(imageParameters_t));
	
	// set the size of the packet
	outcomingPacket->length = 1+sizeof(imageParameters_t);
	
	// send the final packet
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
	
	vTaskDelay(100);
}

void waitForAck() {
		
	xQueueReceive(xCSPEventQueue, &xReceivedEvent, 5000);
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
// image == 1 -> compressed
uint8_t sendCompressed(uint8_t image) {
	
	uint8_t (*getPixel)(uint8_t, uint8_t);
	
	if (image == 0)
		getPixel = &getRawPixel;
	else
		getPixel = &getFilteredPixel;
	
	uint16_t i, j;
	
	uint8_t packetPointer, tempPixel, numPixelsInPacket;
	
	if (imageParameters.nonZeroPixelsOriginal == 0)
		return 0;
	
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
	
	return 0;
}

void sendPostProcessed() {
	
	uint16_t i, j, numPerLine;
	
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
	
	// send the histograms
	if (imageParameters.outputForm == HISTOGRAMS) {
		
		// send the 1st histogram
		// for 4 packets
		for (i = 0; i < 4; i++) {
			
			// it is a first histogram packet [0]
			outcomingPacket->data[0] = 'h';
			
			// save the image ID [1, 2]
			saveUint16(outcomingPacket->data+1, imageParameters.imageId);
			
			// save the number of the packet
			outcomingPacket->data[3] = i;
			
			// fill the packets
			for (j = 0; j < 64; j++) {
				
				outcomingPacket->data[j+4] = getHistogram1(j + i*64);
			}
			
			outcomingPacket->length = 64 + 4;
			
			csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
			
			waitForAck();
		}
		
		// send the 2nd histogram
		// for 4 packets
		for (i = 0; i < 4; i++) {
			
			// it is a first histogram packet [0]
			outcomingPacket->data[0] = 'H';
			
			// save the image ID [1, 2]
			saveUint16(outcomingPacket->data+1, imageParameters.imageId);
			
			// save the number of the packet
			outcomingPacket->data[3] = i;
			
			// fill the packets
			for (j = 0; j < 64; j++) {
				
				outcomingPacket->data[j+4] = getHistogram2(j + i*64);
			}
			
			outcomingPacket->length = 64 + 4;
			
			csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
			
			waitForAck();
		}
		
	// send the binned content
	} else if (imageParameters.outputForm >= BINNING_8 && imageParameters.outputForm <= BINNING_32) {
		
		uint8_t packetId = 0;
		uint8_t byteInPacket = 0;
		
		// it is a first histogram packet [0]
		outcomingPacket->data[0] = 'D';
			
		// save the image ID [1, 2]
		saveUint16(outcomingPacket->data+1, imageParameters.imageId);
		
		// bytes count
		outcomingPacket->data[3] = packetId++;
		
		for (i = 0; i < numPerLine; i++) {
			
			for (j = 0; j < numPerLine; j++) {
				
				outcomingPacket->data[4+byteInPacket++] = getBinnedPixel(i, j);
			
				// packet is full, send it
				if (byteInPacket == 64) {
					
					outcomingPacket->length = 64 + 4;
					
					csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
					
					waitForAck();
					
					byteInPacket = 0;
					
					outcomingPacket->data[3] = packetId++;
				}
			}
		}
	}
}

void shutterDelay() {
	
	if (imageParameters.exposure <= (uint16_t) 60000) {
		
		vTaskDelay(imageParameters.exposure);	
	} else {
		
		vTaskDelay((uint16_t) 60000);
		
		uint16_t i;
		
		for (i = 0; i < (imageParameters.exposure - 60000); i++) {
			
			vTaskDelay(1000);
		}
	}
}

void measure(uint8_t turnOff) {
	
	
	
	if (medipixPowered() == 0) {
		
		medipixInit();
	}
	
	loadImageParametersFromFram();
	
	openShutter();
	
	shutterDelay();
	
	closeShutter();
	
	readMatrix();

	if (turnOff == 1) {
		
		medipixStop();
	}

	imageParameters.temperature = adtTemp;

	filterOnePixelEvents();

	computeImageStatistics();
	
	
	// do binning
	if (imageParameters.outputForm >= BINNING_1 && imageParameters.outputForm <= BINNING_32) {
		
		applyBinning();
		
		} else if (imageParameters.outputForm == HISTOGRAMS) {
		
		createHistograms();
	}
	
	imageParameters.imageId++;
	
	saveImageParametersToFram();
	
	sendImageInfo();
	
	if (imageParameters.outputForm == BINNING_1) {
		
		sendCompressed(1);
		
	} else {
		
		sendPostProcessed();
	}
}

void sendBootupMessage() {
	
	uint8_t i;
	char myChar;
	
	for (i = 0; i < 64; i++) {
		
		myChar = spi_mem_read_byte(MEDIPIX_BOOTUP_MESSAGE+i);
		
		outcomingPacket->data[i] = myChar;
		
		if (myChar == '\0') {
			
			break;
		}
	}
	
	outcomingPacket->length = i;
	
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
}

void sendTemperature() {
	
	char temp[40];
	
	sprintf(temp, "Temp = %d %d\n\r", ADT_get_temperature(), adt_convert_temperature(ADT_get_temperature()));
	strcpy(outcomingPacket->data, temp);
	outcomingPacket->length = strlen(temp);
	csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
}

/* -------------------------------------------------------------------- */
/*	The main task														*/
/* -------------------------------------------------------------------- */
void mainTask(void *p) {
	
	outcomingPacket = csp_buffer_get(CSP_PACKET_SIZE);
		
	uint8_t modeChanging;
					
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
						
							if (settings->mode != imageParameters.mode)
								modeChanging = 1;
							else
								modeChanging = 0;
							
							imageParameters.threshold = settings->treshold;
							imageParameters.exposure = settings->exposure;
							imageParameters.bias = settings->bias;
							imageParameters.filtering = settings->filtering;
							imageParameters.mode = settings->mode;
							imageParameters.outputForm = settings->outputForm;
							
							saveImageParametersToFram();
							
							if ((modeChanging == 1) && (medipixPowered() == 1)) {

								medipixStop();

								vTaskDelay(1000);

								medipixInit();
								
							} else if (medipixPowered() == 1) {
								
								setDACs(imageParameters.threshold);
									
								setBias(imageParameters.bias);
							}
							
							replyOk();
							
						break;
						
						case MEDIPIX_SET_THRESHOLD:
						
							loadImageParametersFromFram();

							imageParameters.threshold = parseUint16(packetPayload);
							
							if (medipixPowered() == 1)
								setDACs(imageParameters.threshold);
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;
						
						case MEDIPIX_SET_BIAS:
						
							loadImageParametersFromFram();

							imageParameters.bias = *packetPayload;
							
							if (medipixPowered() == 1)
								setBias(imageParameters.bias);
							
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
						
							if (*packetPayload != imageParameters.mode)
								modeChanging = 1;
							else
								modeChanging = 0;
								
							imageParameters.mode = *packetPayload;
								
							saveImageParametersToFram();
							
							if ((modeChanging == 1) && (medipixPowered() == 1)) {

								medipixStop();

								vTaskDelay(1000);

								medipixInit();
							}
							
							replyOk();
													
						break;
						
						case MEDIPIX_SET_OUTPUT_FORM:
						
							loadImageParametersFromFram();

							imageParameters.outputForm = *packetPayload;
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;
						
						case MEDIPIX_MEASURE:
							
							measure(1);
						
						break;
						
						case MEDIPIX_MEASURE_WITH_PARAMETERS:
							
							measure(1);

						break;
						
						case MEDIPIX_MEASURE_NO_TURNOFF:
						
							measure(0);
						
						break;
						
						case MEDIPIX_SEND_ORIGINAL:
						
							sendImageInfo();
							sendCompressed(0);
						
						break;
						
						case MEDIPIX_SEND_FILTERED:
						
							sendImageInfo();
							sendCompressed(1);
						
						break;
						
						case MEDIPIX_SEND_BINNED:
						
							sendImageInfo();
							sendPostProcessed();
						
						break;	
						
						case MEDIPIX_SEND_METADATA:
							
							sendImageInfo();
						
						break;
						
						case MEDIPIX_GET_BOOTUP_MESSAGE:
						
							sendBootupMessage();
						
						break;
						
						case MEDIPIX_GET_TEMPERATURE:
						
							sendTemperature();
						
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