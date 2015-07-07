/*
 * mainTask.h
 *
 * Created: 11.10.2014 20:19:18
 *  Author: Tomas Baca
 */ 

#ifndef MAINTASK_H_
#define MAINTASK_H_

#include "system.h"

xQueueHandle * xCSPEventQueue;
xQueueHandle * xCSPAckQueue;

csp_packet_t * outcomingPacket;

unsigned int dest_p;
unsigned int source_p;

void mainTask(void *p);
uint8_t waitForDkAck();

// board commands
typedef enum {
	MEDIPIX_PWR_ON = 0,									// initialize medipix
	MEDIPIX_PWR_OFF = 1,								// power off medipix
	MEDIPIX_SET_ALL_PARAMS = 2,							// set all aquisition parameters (save to fram)
	MEDIPIX_SET_THRESHOLD = 3,							// set new treshold
	MEDIPIX_SET_BIAS = 4,								// set new bias
	MEDIPIX_SET_EXPOSURE = 5,							// set new exposure
	MEDIPIX_SET_FILTERING = 6,							// set new filtering mode
	MEDIPIX_SET_MODE = 7,								// set new aquisition mode
	MEDIPIX_SET_OUTPUT_FORM = 8,						// set new output form
	MEDIPIX_MEASURE = 9,								// take a shot
	MEDIPIX_MEASURE_WITH_PARAMETERS = 10,				// take a shot with all parameters specified in the command
	MEDIPIX_MEASURE_WITHOUT_DATA = 11,					// take a shot but only send metadata
	MEDIPIX_SEND_ORIGINAL = 12,							// send the original image
	MEDIPIX_SEND_FILTERED = 13,							// send the filtered image
	MEDIPIX_SEND_BINNED = 14,							// send binned/histogram image
	MEDIPIX_SEND_METADATA = 15,							// send only the image metadata
	MEDIPIX_MEASURE_NO_TURNOFF = 16,					// dont turn off medipix after measurement
	MEDIPIX_GET_BOOTUP_MESSAGE = 17,					// return the medipix's bootup message from the last boot
	MEDIPIX_GET_TEMPERATURE = 18,						// return the medipix's bootup message from the last boot
	MEDIPIX_GET_HOUSKEEPING = 19,						// return the main houskeeping information
	MEDIPIX_MEASURE_WITHOUT_DATA_NO_TURNOFF = 20,		// return the main houskeeping information
	XRAY_DK_CREATE_STORAGES = 21,						// create all storages in the data keeper
} MPX_COMMANDS;	

typedef struct {
	
	uint16_t bootCount;
	uint16_t imagesTaken;
	uint8_t temperature;
	uint8_t framStatus;
	uint8_t medipixStatus;
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} hk_data_t;

#define MEASURE_TURNOFF_YES		1
#define MEASURE_TURNOFF_NO		0

#define MEASURE_WITHOUT_DATA_YES	1
#define MEASURE_WITHOUT_DATA_NO		0

#define OUTPUT_DATAKEEPER	1
#define OUTPUT_DIRECT		0

#endif /* MAINTASK_H_ */