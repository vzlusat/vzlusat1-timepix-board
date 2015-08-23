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
#include "spi_memory_FM25.h"
#include "dkHandler.h"
#include "myADC.h"
#include "csp_endian.h"

csp_packet_t * outcomingPacket;
xQueueHandle * xCSPEventQueue;
xQueueHandle * xCSPAckQueue;
xQueueHandle * xCSPTimeQueue;
xQueueHandle * xCSPAttitudeQueue;

unsigned int dest_addr;
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

	csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
}

void replyErr(uint8_t error) {
	
	vTaskDelay(50);
	
	outcomingPacket->data[0] = 'E';
	outcomingPacket->data[1] = error;
	outcomingPacket->data[2] = '\0';
	
	outcomingPacket->length = 3;
	csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
	
	vTaskDelay(50);
}

void replyOk() {
	
	vTaskDelay(50);
	
	outcomingPacket->data[0] = 'O';
	outcomingPacket->data[1] = 'K';
	outcomingPacket->data[2] = '\0';
	
	outcomingPacket->length = 3;
	csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
	
	vTaskDelay(50);
}

uint8_t waitForDkAck() {
	
	int32_t err;
	
	if (pdTRUE == xQueueReceive(xCSPAckQueue, &err, 3000)) {
		
		if (err != 0) {
			
			return 0;
		} else {
			
			return 1;
		}
	}

	return 0;
}

/* -------------------------------------------------------------------- */
/*	Reply with some status info message									*/
/* -------------------------------------------------------------------- */
void houseKeeping(uint8_t outputTo) {
	
	loadImageParametersFromFram();
	
	hk_data_t * hk_data;
	dk_msg_store_ack_t * message;
	
	if (outputTo == OUTPUT_DIRECT) {
		
		hk_data = (hk_data_t *) outcomingPacket->data;
	} else {
		
		message = (dk_msg_store_ack_t *) outcomingPacket->data;
		hk_data = (hk_data_t *) message->data;
	}
	
	hk_data->packetType = 'Z';
	hk_data->bootCount = csp_hton16(getBootCount());
	hk_data->imagesTaken = csp_hton16(imageParameters.imageId);
	hk_data->temperature = adt_convert_temperature(ADT_get_temperature());
	hk_data->framStatus = fram_test();
	hk_data->medipixStatus = medipixCheckStatus();
	hk_data->seconds = csp_hton32(secondsTimer);
	hk_data->TIR_max = csp_hton16(uv_ir_data.TIR_max);
	hk_data->TIR_min = csp_hton16(uv_ir_data.TIR_min);
	hk_data->IR_max = csp_hton16(uv_ir_data.IR_max);
	hk_data->IR_min = csp_hton16(uv_ir_data.IR_min);
	hk_data->UV1_max = csp_hton16(uv_ir_data.UV1_max);
	hk_data->UV1_min = csp_hton16(uv_ir_data.UV1_min);
	hk_data->UV2_max = csp_hton16(uv_ir_data.UV2_max);
	hk_data->UV2_min = csp_hton16(uv_ir_data.UV2_min);
	hk_data->temperature_max = adtTemp_max;
	hk_data->temperature_min = adtTemp_min;
	
	// direct answer
	if (outputTo == OUTPUT_DIRECT) {

		outcomingPacket->length = sizeof(hk_data_t);

		csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
	
	// save it to datakeeper
	} else {
	
		message->parent.cmd = DKC_STORE_ACK;
		message->port = STORAGE_HK_ID;
		message->host = CSP_DK_MY_ADDRESS;
		
		uint8_t k;
		for (k = 0; k < 3; k++) {
			
			outcomingPacket->length = sizeof(dk_msg_store_ack_t) + sizeof(hk_data_t);
			csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);
			
			if (waitForDkAck() == 1) {
				
				break;
			}
		}
		
		if (k == 3) {
			
			replyErr(ERROR_DATA_NOT_SAVED);
		} else {
			
			replyOk();
		}
	}
}

void medipixInit() {
	
	loadImageParametersFromFram();
	
	if (medipixPowered() == 0)	
		pwrOnMedipix();	

	if (medipixPowered() == 1) {

		loadEqualization(&dataBuffer, &ioBuffer);
		
		eraseMatrix();
		
		setDACs(imageParameters.threshold);
		
		setBias(imageParameters.bias);
	}
}

void medipixStop() {
	
	pwrOffMedipix();
}

void sendImageInfo(uint8_t repplyTo, uint8_t outputForm) {
	
	if (repplyTo == OUTPUT_DIRECT) {

		// load current info from fram
		loadImageParametersFromFram();
		
		imageParameters.outputForm = outputForm;

		imageParameters.packetType = 'A';

		// save current info to the packet
		memcpy(outcomingPacket->data, &imageParameters, sizeof(imageParameters_t));
		
		imageParameters_t * params = (imageParameters_t *) (outcomingPacket->data);
		params->imageId = csp_hton16(params->imageId);
		params->threshold = csp_hton16(params->threshold);
		params->exposure = csp_hton16(params->exposure);
		params->nonZeroPixelsFiltered = csp_hton16(params->nonZeroPixelsFiltered);
		params->nonZeroPixelsOriginal = csp_hton16(params->nonZeroPixelsOriginal);
		params->pixelCountThr = csp_hton16(params->pixelCountThr);
		params->uv1_treshold = csp_hton16(params->uv1_treshold);
		params->chunkId = csp_hton32(params->chunkId);
		
		uint8_t i;
		for (i = 0; i < 7; i++) {
			params->attitude[i] = csp_hton16(params->attitude[i]);
		}
		for (i = 0; i < 3; i++) {
			params->position[i] = csp_hton16(params->position[i]);
		}

		// set the size of the packet
		outcomingPacket->length = sizeof(imageParameters_t);

		// send the final packet
		csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);

		vTaskDelay(100);
		
		// restore current parameters (they have changed)
		loadImageParametersFromFram();
		
	// save data to datakeeper
	} else {
		
		// load current info from fram
		loadImageParametersFromFram();
		
		imageParameters.outputForm = outputForm;
		
		dk_msg_store_ack_t * message = (dk_msg_store_ack_t *) outcomingPacket->data;
		
		imageParameters.packetType = 'A';
		
		message->parent.cmd = DKC_STORE_ACK;
		message->port = STORAGE_METADATA_ID;
		message->host = CSP_DK_MY_ADDRESS;
		
		imageParameters.packetType = 'A';
		
		// save current info to the packet
		memcpy(message->data, &imageParameters, sizeof(imageParameters_t));
		
		uint8_t k;
		for (k = 0; k < 3; k++) {
			
			outcomingPacket->length = sizeof(dk_msg_store_ack_t) + sizeof(imageParameters_t);
			csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);
			
			if (waitForDkAck() == 1) {
				
				break;
			}
		}

		// restore current parameters (they have changed)
		loadImageParametersFromFram();
	}
}

void waitForAck() {
		
	xQueueReceive(xCSPEventQueue, &xReceivedEvent, 100);
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
uint8_t sendCompressed(uint8_t image, uint8_t replyTo) {
	
	uint8_t (*getPixel)(uint8_t, uint8_t);
	
	if (image == 0)
		getPixel = &getRawPixel;
	else
		getPixel = &getFilteredPixel;
	
	uint16_t i, j;
	
	uint8_t packetPointer, tempPixel, numPixelsInPacket;
	
	uint8_t noErr = 1;
	
	if (imageParameters.nonZeroPixelsOriginal == 0)
		return 0;
		
	if (replyTo == OUTPUT_DIRECT) {
	
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
						csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
					
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
			csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
		}
	
		waitForAck();
		
		vTaskDelay(20);
	
		// send the terminal packet
		outcomingPacket->data[0] = 'C';
		outcomingPacket->length = 1;
	
		csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
	
		waitForAck();
	
		return 1;
	
	// output to DK
	} else {
		
		dk_msg_store_ack_t * message = (dk_msg_store_ack_t *) outcomingPacket->data;
	
		message->parent.cmd = DKC_STORE_ACK;
		message->port = STORAGE_DATA_ID;
		message->host = CSP_DK_MY_ADDRESS;
		
		// initialize the first packet
		message->data[0] = 'B';
		saveUint16(message->data+1, imageParameters.imageId);
		packetPointer = 4;
		numPixelsInPacket = 0;
		
		for (i = 0; i < 256; i++) {
			
			for (j = 0; j < 256; j++) {
				
				tempPixel = getPixel(i, j);
				
				// the pixel will be send
				if (tempPixel > 0) {
					
					// there is still a place in the packet
					if (packetPointer <= 61) {
						
						saveUint16(message->data + packetPointer, i*256+j);
						packetPointer += 2;
						*(message->data + packetPointer++) = tempPixel;
						numPixelsInPacket++;
					}
					
					// the packet is full, send it
					if (packetPointer > 61) {
						
						message->data[3] = numPixelsInPacket;
						
						uint8_t k;
						for (k = 0; k < 3; k++) {
						
							outcomingPacket->length = packetPointer + sizeof(dk_msg_store_ack_t);
							csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);
							
							if (waitForDkAck() == 1) {
								
								break;
							}
						}
						
						if (k == 3) {
							
							noErr *= 0;
						}
						
						message->data[0] = 'B';
						saveUint16(message->data+1, imageParameters.imageId);
						packetPointer = 4;
						numPixelsInPacket = 0;
					}
				}
			}
		}
		
		// send the last data packet
		if (packetPointer > 4) {
			
			message->data[3] = numPixelsInPacket;
			
			uint8_t k;
			for (k = 0; k < 3; k++) {	
				
				outcomingPacket->length = packetPointer + sizeof(dk_msg_store_ack_t);
				csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);
				
				if (waitForDkAck() == 1) {
								
					break;
				}
			}
						
			if (k == 3) {
							
				noErr *= 0;
			}
		}

		return noErr;
	}

}

void sendPostProcessed(uint8_t replyTo, uint8_t outputForm) {
	
	uint16_t i, j;
	
	uint16_t numPerLine = 0;
	
	dk_msg_store_ack_t * message = (dk_msg_store_ack_t *) outcomingPacket->data;
	
	uint8_t noErr = 1;
	
	switch (outputForm) {
		
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
	
	// if saving data to datakeeper, set the correct data storage
	if (replyTo == OUTPUT_DATAKEEPER) {
		
		message->parent.cmd = DKC_STORE_ACK;
		message->port = STORAGE_DATA_ID;
		message->host = CSP_DK_MY_ADDRESS;
	}
	
	// send the energy histogram
	if (outputForm == ENERGY_HISTOGRAM) {
		
		if (replyTo == OUTPUT_DIRECT) {
			
			// it is a first histogram packet [0]
			outcomingPacket->data[0] = 'e';
			
			// save the image ID [1, 2]
			saveUint16(outcomingPacket->data+1, imageParameters.imageId);
				
			// fill the packets
			for (j = 0; j < 32; j++) {
					
				outcomingPacket->data[j+3] = spi_mem_read_byte((unsigned long) (ENERGY_HISTOGRAM_ADRESS + j));
			}
				
			outcomingPacket->length = 32 + 3;
				
			csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
				
			waitForAck();

		} else {

			// it is a first histogram packet [0]
			message->data[0] = 'e';
			
			// save the image ID [1, 2]
			saveUint16(message->data+1, imageParameters.imageId);
			
			// fill the packets
			for (j = 0; j < 32; j++) {
				
				message->data[j+3] = spi_mem_read_byte((unsigned long) (ENERGY_HISTOGRAM_ADRESS + j));
			}
				
			uint8_t k;
			for (k = 0; k < 3; k++) {
					
				outcomingPacket->length = 32 + 3 + sizeof(dk_msg_store_ack_t);
				csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);
					
				if (waitForDkAck() == 1) {
						
					break;
				}
			}
				
			if (k == 3) {
					
				noErr *= 0;
			}
		}
		
	} else if (outputForm == HISTOGRAMS) {
		
		if (replyTo == OUTPUT_DIRECT) {
			
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
			
				csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
			
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
			
				csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
			
				waitForAck();
			}
		} else {
			
			// send the 1st histogram
			// for 4 packets
			for (i = 0; i < 4; i++) {
				
				// it is a first histogram packet [0]
				message->data[0] = 'h';
				
				// save the image ID [1, 2]
				saveUint16(message->data+1, imageParameters.imageId);
				
				// save the number of the packet
				message->data[3] = i;
				
				// fill the packets
				for (j = 0; j < 64; j++) {
					
					message->data[j+4] = getHistogram1(j + i*64);
				}
				
				uint8_t k;
				for (k = 0; k < 3; k++) {
					
					outcomingPacket->length = 64 + 4 + sizeof(dk_msg_store_ack_t);
					csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);
					
					if (waitForDkAck() == 1) {
						
						break;
					}
				}
				
				if (k == 3) {
					
					noErr *= 0;
				}
			}
			
			// send the 2nd histogram
			// for 4 packets
			for (i = 0; i < 4; i++) {
				
				// it is a first histogram packet [0]
				message->data[0] = 'H';
				
				// save the image ID [1, 2]
				saveUint16(message->data+1, imageParameters.imageId);
				
				// save the number of the packet
				message->data[3] = i;
				
				// fill the packets
				for (j = 0; j < 64; j++) {
					
					message->data[j+4] = getHistogram2(j + i*64);
				}
				
				uint8_t k;
				for (k = 0; k < 3; k++) {
							
					outcomingPacket->length = 64 + 4 + sizeof(dk_msg_store_ack_t);
					csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);
							
					if (waitForDkAck() == 1) {
								
						break;
					}
				}
						
				if (k == 3) {
							
					noErr *= 0;
				}
			}
		}
		
	// send the binned content
	} else if (outputForm >= BINNING_8 && outputForm <= BINNING_32) {
		
		uint8_t packetId = 0;
		uint8_t byteInPacket = 0;
		uint8_t outId = 0;
		
		if (outputForm == BINNING_8)
			outId = 'D';
		else if (outputForm == BINNING_16)
			outId = 'E';
		else if (outputForm == BINNING_32)
			outId = 'F';
		
		if (replyTo == OUTPUT_DIRECT) {
		
			// it is a first histogram packet [0]
		
			// save the image ID [1, 2]
			saveUint16(outcomingPacket->data+1, imageParameters.imageId);
			
			outcomingPacket->data[0] = outId;
		
			// bytes count
			outcomingPacket->data[3] = packetId++;
		
			for (i = 0; i < numPerLine; i++) {
			
				for (j = 0; j < numPerLine; j++) {
				
					outcomingPacket->data[4+byteInPacket++] = getBinnedPixel(i, j, outputForm);
				
					// packet is full, send it
					if (byteInPacket == 64) {
					
						outcomingPacket->length = 64 + 4;
					
						csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
					
						waitForAck();
					
						byteInPacket = 0;
					
						outcomingPacket->data[3] = packetId++;
					}
				}
			}
			
		// save to datakeeper
		} else {
			
			// it is a first histogram packet [0]
			message->data[0] = outId;
			
			// save the image ID [1, 2]
			saveUint16(message->data+1, imageParameters.imageId);
			
			// bytes count
			message->data[3] = packetId++;
			
			for (i = 0; i < numPerLine; i++) {
				
				for (j = 0; j < numPerLine; j++) {
					
					message->data[4+byteInPacket++] = getBinnedPixel(i, j, outputForm);
					
					// packet is full, send it
					if (byteInPacket == 64) {
						
						uint8_t k;
						for (k = 0; k < 3; k++) {
							
							outcomingPacket->length = 64 + 4 + sizeof(dk_msg_store_ack_t);
							csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);
							
							if (waitForDkAck() == 1) {
								
								break;
							}
						}
						
						if (k == 3) {
							
							noErr *= 0;
						}
						
						byteInPacket = 0;
						
						message->data[3] = packetId++;
					}
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

uint32_t getNextChunkId(uint8_t port) {
	
	dk_msg_storage_t * message = (dk_msg_storage_t *) outcomingPacket->data;
	
	message->host = CSP_DK_MY_ADDRESS;
	message->port = STORAGE_DATA_ID;
	message->parent.cmd = DKC_INFO;
	
	uint32_t chunksNum;
	
	uint8_t i;
	for (i = 0; i < 3; i++) {
		
		outcomingPacket->length = sizeof(dk_msg_storage_t);
		
		csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 21, CSP_O_NONE, outcomingPacket, 1000);
		
		chunksNum = csp_hton32(waitForTimeAck());
		
		if (chunksNum > 0) {
				
			break;
		}	
	}
	
	if (chunksNum == 0)
		return 0;
		
	return chunksNum;
}

uint8_t measure(uint8_t turnOff, uint8_t withoutData, uint8_t repplyTo, uint8_t usePixelTreshold) {
	
	char inChar;
	
	if (medipixPowered() == 0) {
		
		medipixInit();
	}
	
	// pokud se nepustil
	if (medipixPowered() == 0) {
		
		// zalogovat chybu
		return 0;
	}
	
	loadImageParametersFromFram();
	
	openShutter();
	
	shutterDelay();
	
	closeShutter();
	
	vTaskDelay(10);
	
	readMatrix();
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1)) {}

	if (turnOff == MEASURE_TURNOFF_YES) {
		
		medipixStop();
	}

	imageParameters.temperature = adtTemp;
	
	imageParameters.time = getTime();

	getAttitude(&imageParameters.attitude, &imageParameters.position);

	filterOnePixelEvents();

	computeImageStatistics();

	if (usePixelTreshold == 1) {
		
		if (imageParameters.nonZeroPixelsFiltered < imageParameters.pixelCountThr) {
			
			return 0;
		}
	}

	// increase image id
	imageParameters.imageId++;
	saveImageParametersToFram();
	
	/* ZDE KONÈÍ VYÈÍTÁNÍ OBRÁZKU */
	
	// BINING_32
	if ((imageParameters.outputForm & 0x08) > 0) {
		
		applyBinning(BINNING_32);
		imageParameters.chunkId = getNextChunkId(STORAGE_DATA_ID);
		saveImageParametersToFram();
		sendImageInfo(repplyTo, BINNING_32);
		sendPostProcessed(repplyTo, BINNING_32);
		vTaskDelay(30);
	}

	// BINING_16
	if ((imageParameters.outputForm & 0x04) > 0) {
	
		applyBinning(BINNING_16);
		imageParameters.chunkId = getNextChunkId(STORAGE_DATA_ID);
		saveImageParametersToFram();
		sendImageInfo(repplyTo, BINNING_16);
		sendPostProcessed(repplyTo, BINNING_16);
		vTaskDelay(30);
	}

	// BINING_8
	if ((imageParameters.outputForm & 0x02) > 0) {

		applyBinning(BINNING_8);
		imageParameters.chunkId = getNextChunkId(STORAGE_DATA_ID);
		saveImageParametersToFram();
		sendImageInfo(repplyTo, BINNING_8);
		sendPostProcessed(repplyTo, BINNING_8);
		vTaskDelay(30);
	}

	// HISTOGRAMY
	if ((imageParameters.outputForm & 0x10) > 0) {
	
		createHistograms(HISTOGRAMS);
		imageParameters.chunkId = getNextChunkId(STORAGE_DATA_ID);
		saveImageParametersToFram();
		sendImageInfo(repplyTo, HISTOGRAMS);
		sendPostProcessed(repplyTo, HISTOGRAMS);
		vTaskDelay(30);
	}

	// ENERGY_HISTOGRAM
	if ((imageParameters.outputForm & 0x20) > 0) {
	
		createEnergyHistogram();
		imageParameters.chunkId = getNextChunkId(STORAGE_DATA_ID);
		saveImageParametersToFram();
		sendImageInfo(repplyTo, ENERGY_HISTOGRAM);
		sendPostProcessed(repplyTo, ENERGY_HISTOGRAM);
		vTaskDelay(30);
	}
	
	// BINING_1
	if ((imageParameters.outputForm & 0x01) > 0) {
		
		applyBinning(BINNING_1);
		imageParameters.chunkId = getNextChunkId(STORAGE_DATA_ID);
		saveImageParametersToFram();
		sendImageInfo(repplyTo, BINNING_1);
		sendCompressed(1, repplyTo);
		vTaskDelay(30);
	}

	return 1;
}

void sendBootupMessage(uint8_t replyTo) {
	
	uint8_t i;
	char myChar;
	
	if (replyTo == OUTPUT_DIRECT) {
		
		/*
	
		for (i = 0; i < 64; i++) {
			
			myChar = spi_mem_read_byte(MEDIPIX_BOOTUP_MESSAGE+i);
			
			outcomingPacket->data[i] = myChar;
			
			if (myChar == '\0') {
				
				break;
			}
		}
		
		outcomingPacket->length = i;
		
		csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
		
		*/
		
	// DK response
	} else {
		
		dk_msg_store_ack_t * message = (dk_msg_store_ack_t *) outcomingPacket->data;
		
		message->parent.cmd = DKC_STORE_ACK;
		message->port = STORAGE_HK_ID;
		message->host = CSP_DK_MY_ADDRESS;
		
		for (i = 0; i < 64; i++) {
			
			myChar = spi_mem_read_byte(MEDIPIX_BOOTUP_MESSAGE+i);
			
			message->data[i] = myChar;
			
			if (myChar == '\0') {
				
				break;
			}
		}
		
		uint8_t k;
		for (k = 0; k < 3; k++) {
		
			outcomingPacket->length = sizeof(dk_msg_store_ack_t) + i;
			csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);
			
			if (waitForDkAck() == 1) {
				
				replyOk();
				break;				
			}
		}
		
		if (k == 3) {
			
			replyErr(ERROR_DATA_NOT_SAVED);
		}
	}
}

void sendTemperature(uint8_t outputTo) {
	
	if (outputTo == OUTPUT_DIRECT) {

		outcomingPacket->data[0] = adtTemp;
		outcomingPacket->length = 1;
		csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
	} 
}

void sendSensorsData() {
	
	sensors_t tempData;
	
	sensors_t * sensors_data = (sensors_t *) outcomingPacket->data;
	
	sensors_data->IR = csp_hton16(uv_ir_data.IR);
	sensors_data->TIR = csp_hton16(uv_ir_data.TIR);
	sensors_data->UV1 = csp_hton16(uv_ir_data.UV1);
	sensors_data->UV2 = csp_hton16(uv_ir_data.UV2);
	sensors_data->TIR_max = csp_hton16(uv_ir_data.TIR_max);
	sensors_data->TIR_min = csp_hton16(uv_ir_data.TIR_min);
	sensors_data->IR_max = csp_hton16(uv_ir_data.IR_max);
	sensors_data->IR_min = csp_hton16(uv_ir_data.IR_min);
	sensors_data->UV1_max = csp_hton16(uv_ir_data.UV1_max);
	sensors_data->UV1_min = csp_hton16(uv_ir_data.UV1_min);
	sensors_data->UV2_max = csp_hton16(uv_ir_data.UV2_max);
	sensors_data->UV2_min = csp_hton16(uv_ir_data.UV2_min);
	
	outcomingPacket->length = sizeof(sensors_t);
	
	csp_sendto(CSP_PRIO_NORM, dest_addr, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
}

/* -------------------------------------------------------------------- */
/*	The main task														*/
/* -------------------------------------------------------------------- */
void mainTask(void *p) {
	
	outcomingPacket = csp_buffer_get(CSP_PACKET_SIZE);
		
	uint8_t modeChanging;
	uint16_t i;
	
	loadImageParametersFromFram();
					
	// infinite while loop of the program 
	while (1) {
			
		if (xQueueReceive(xCSPEventQueue, &xReceivedEvent, 1)) {
			
			dest_p = ((csp_packet_t *) (xReceivedEvent.pvData))->id.sport;
			source_p = ((csp_packet_t *) (xReceivedEvent.pvData))->id.dport;
			dest_addr = ((csp_packet_t *) (xReceivedEvent.pvData))->id.src;
			
			uint8_t command = ((csp_packet_t *) xReceivedEvent.pvData)->data[0];
			uint8_t * packetPayload = ((csp_packet_t *) xReceivedEvent.pvData)->data+1;
		
			switch( xReceivedEvent.eEventType ) {
				
				case obcEvent :
					
					switch (command) {
				
						case MEDIPIX_GET_TEMPERATURE:
						
							sendTemperature(OUTPUT_DATAKEEPER);
						
						break;

						case MEDIPIX_GET_HOUSEKEEPING:

							houseKeeping(OUTPUT_DATAKEEPER);

						break;
					
						case MEDIPIX_GET_BOOTUP_MESSAGE:

							sendBootupMessage(OUTPUT_DATAKEEPER);

						break;
						
						case MEDIPIX_MEASURE:
						
							replyOk();
							measure(MEASURE_TURNOFF_YES, MEASURE_WITHOUT_DATA_NO, OUTPUT_DATAKEEPER, USE_PIXEL_TRESHOLD_NO);
						
						break;
						
						case MEDIPIX_MEASURE_UV:
						
							replyOk();
							loadImageParametersFromFram();
							
							for (i = 0; i < ADRENALIN_LENGTH; i++) {
								
								if (uv_ir_data.UV1 > imageParameters.uv1_treshold) {
									
									measure(MEASURE_TURNOFF_YES, MEASURE_WITHOUT_DATA_NO, OUTPUT_DATAKEEPER, USE_PIXEL_TRESHOLD_NO);
									break;
								}
								
								vTaskDelay(ADRENALIN_DELAY);
							}
						
						break;
						
						case MEDIPIX_MEASURE_WITH_PARAMETERS:
						
							replyOk();
							measure(MEASURE_TURNOFF_YES, MEASURE_WITHOUT_DATA_NO, OUTPUT_DATAKEEPER, USE_PIXEL_TRESHOLD_NO);

						break;

						case MEDIPIX_MEASURE_NO_TURNOFF:

							replyOk();	
							measure(MEASURE_TURNOFF_NO, MEASURE_WITHOUT_DATA_NO, OUTPUT_DATAKEEPER, USE_PIXEL_TRESHOLD_NO);

						break;
					
						/*
						case MEDIPIX_MEASURE_WITHOUT_DATA:

							replyOk();
							measure(MEASURE_TURNOFF_YES, MEASURE_WITHOUT_DATA_YES, OUTPUT_DATAKEEPER, USE_PIXEL_TRESHOLD_NO);

						break;
						
						case MEDIPIX_MEASURE_WITHOUT_DATA_NO_TURNOFF:

							replyOk();
							measure(MEASURE_TURNOFF_NO, MEASURE_WITHOUT_DATA_YES, OUTPUT_DATAKEEPER, USE_PIXEL_TRESHOLD_NO);

						break;
						*/
						
						case MEDIPIX_MEASURE_SCANNING_MODE:
						
							replyOk();
							measure(MEASURE_TURNOFF_YES, MEASURE_WITHOUT_DATA_NO, OUTPUT_DATAKEEPER, USE_PIXEL_TRESHOLD_YES);
						
						break;
						
						case MEDIPIX_MEASURE_SCANNING_MODE_NO_TURNOFF:
						
							replyOk();
							measure(MEASURE_TURNOFF_NO, MEASURE_WITHOUT_DATA_NO, OUTPUT_DATAKEEPER, USE_PIXEL_TRESHOLD_YES);
						
						break;
						
						/*	REMOVED TO PRESERVE A PROGRAM MEMORY... OBSOLETE 
						
						case MEDIPIX_SEND_ORIGINAL:
						
							replyOk();
							sendCompressed(0, OUTPUT_DATAKEEPER);
						
						break;
						
						case MEDIPIX_SEND_FILTERED:
						
							replyOk();
							sendCompressed(1, OUTPUT_DATAKEEPER);
						
						break;
						
						case MEDIPIX_SEND_BINNED:
						
							replyOk();
							sendPostProcessed(OUTPUT_DATAKEEPER);

						break;
						
						*/
					}
									
				break;
				
				// sends the info about the system
				case directEvent :
					
					switch (command) {
						
						case MEDIPIX_PWR_ON:
						
							if (medipixPowered() == 1) {
								
								replyErr(ERROR_MEDIPIX_ALREADY_POWERED);	
							
							} else {
							
								medipixInit();
							
								if (medipixPowered() == 1)
									replyOk();
								else
									replyErr(ERROR_MEDIPIX_NOT_POWERED);
							}
							
						break;
						
						case MEDIPIX_PWR_OFF:
						
							medipixStop();
						
							replyOk();
							
						break;
						
						case MEDIPIX_SET_ALL_PARAMS:
						
							loadImageParametersFromFram();
							
							newSettings_t * settings = (newSettings_t *) packetPayload;
						
							if (settings->mode != imageParameters.mode)
								modeChanging = 1;
							else
								modeChanging = 0;
							
							imageParameters.threshold = csp_ntoh16(settings->treshold);
							imageParameters.exposure = csp_ntoh16(settings->exposure);
							imageParameters.bias = settings->bias;
							imageParameters.filtering = settings->filtering;
							imageParameters.mode = settings->mode;
							imageParameters.outputForm = settings->outputForm;
							imageParameters.temperatureLimit = settings->temperatureLimit;
							imageParameters.pixelCountThr = csp_ntoh16(settings->pixelCountThr);
							imageParameters.uv1_treshold = csp_ntoh16(settings->uv1_treshold);
							
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

							imageParameters.threshold = csp_ntoh16(parseUint16(packetPayload));
							
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

							imageParameters.exposure = csp_ntoh16(parseUint16(packetPayload));
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;
						
						case MEDIPIX_SET_FILTERING:
						
							loadImageParametersFromFram();

							imageParameters.filtering = *packetPayload;
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;
						
						case MEDIPIX_SET_TEMPLIMIT:
						
							loadImageParametersFromFram();

							imageParameters.temperatureLimit = *packetPayload;
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;		
						
						case MEDIPIX_SET_PIXELCNTTHR:
						
							loadImageParametersFromFram();

							imageParameters.pixelCountThr = csp_ntoh16(parseUint16(packetPayload));
							
							saveImageParametersToFram();
							
							replyOk();
						
						break;			
						
						case MEDIPIX_SET_UV1THL:
						
							loadImageParametersFromFram();

							imageParameters.uv1_treshold = csp_ntoh16(parseUint16(packetPayload));
							
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
							
							measure(MEASURE_TURNOFF_YES, MEASURE_WITHOUT_DATA_NO, OUTPUT_DIRECT, USE_PIXEL_TRESHOLD_NO);
						
						break;
						
						case MEDIPIX_MEASURE_WITH_PARAMETERS:
							
							measure(MEASURE_TURNOFF_YES, MEASURE_WITHOUT_DATA_NO, OUTPUT_DIRECT, USE_PIXEL_TRESHOLD_NO);

						break;
						
						case MEDIPIX_MEASURE_NO_TURNOFF:
						
							measure(MEASURE_TURNOFF_NO, MEASURE_WITHOUT_DATA_NO, OUTPUT_DIRECT, USE_PIXEL_TRESHOLD_NO);
						
						break;
						
						/*
						case MEDIPIX_MEASURE_WITHOUT_DATA:
						
							measure(MEASURE_TURNOFF_YES, MEASURE_WITHOUT_DATA_YES, OUTPUT_DIRECT, USE_PIXEL_TRESHOLD_NO);
						
						break;
						
						case MEDIPIX_MEASURE_WITHOUT_DATA_NO_TURNOFF:
						
							measure(MEASURE_TURNOFF_NO, MEASURE_WITHOUT_DATA_YES, OUTPUT_DIRECT, USE_PIXEL_TRESHOLD_NO);
						
						break;
						*/
						
						case MEDIPIX_SEND_ORIGINAL:
						
							sendImageInfo(OUTPUT_DIRECT, BINNING_1);
							sendCompressed(0, OUTPUT_DIRECT);
						
						break;
						
						/*
						case MEDIPIX_SEND_FILTERED:
						
							sendImageInfo(OUTPUT_DIRECT, BINNING_1);
							sendCompressed(1, OUTPUT_DIRECT);
						
						break;
						*/
						
						/*
						case MEDIPIX_SEND_BINNED:
						
							sendImageInfo(OUTPUT_DIRECT);
							sendPostProcessed(OUTPUT_DIRECT);
						
						break;	
						*/
						
						case MEDIPIX_SEND_METADATA:
							
							loadImageParametersFromFram();
							sendImageInfo(OUTPUT_DIRECT, imageParameters.outputForm);
						
						break;
						
						/*						
						case MEDIPIX_GET_BOOTUP_MESSAGE:
						
							sendBootupMessage(OUTPUT_DIRECT);
						
						break;
						*/
						
						case MEDIPIX_GET_TEMPERATURE:
						
							sendTemperature(OUTPUT_DIRECT);
						
						break;
						
						case MEDIPIX_GET_HOUSEKEEPING:
						
							houseKeeping(OUTPUT_DIRECT);
						
						break;
						
						case XRAY_DK_CREATE_STORAGES:
						
							if (createStorages() == 1) {
							
								replyOk();
								
							} else {
								
								replyErr(ERROR_STORAGES_NOT_CREATED);
							}
						
						break;
						
						case MEDIPIX_SEND_SENSOR_DATA:
						
							sendSensorsData();
							
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