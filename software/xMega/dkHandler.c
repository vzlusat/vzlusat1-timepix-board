/*
 * dkHandler.c
 *
 * Created: 7.7.2015 14:36:58
 *  Author: klaxalk
 */ 

#include "system.h"
#include "dkHandler.h"
#include "mainTask.h"
#include "medipix.h"

uint8_t createStorage(uint8_t id, uint16_t size) {
	
	dk_msg_create_t * message = (dk_msg_create_t *) outcomingPacket->data;
	
	message->parent.cmd = DKC_CREATE;
	message->port = id;
	message->conf_siz = csp_hton16(size);
	
	outcomingPacket->length = sizeof(dk_msg_create_t);
	
	csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);
	
	return waitForDkAck();
};

uint8_t createStorages() {
	
	uint8_t out = 1;
	
	// create storage for settings
	out *= createStorage(STORAGE_SETTINGS_ID, DEFAULT_CONF_CHUNK_SIZE);
	
	// create storage for houskeeping data
	out *= createStorage(STORAGE_HK_ID, DEFAULT_CONF_CHUNK_SIZE);
	
	// create storage for image metadata
	out *= createStorage(STORAGE_METADATA_ID, DEFAULT_CONF_CHUNK_SIZE);
	
	// create storage for images with binning = 32
	out *= createStorage(STORAGE_BINNED32_ID, DEFAULT_CONF_CHUNK_SIZE);
	
	// create storage for images with binning = 16
	out *= createStorage(STORAGE_BINNED16_ID, DEFAULT_CONF_CHUNK_SIZE);
	
	// create storage for images with binning = 8
	out *= createStorage(STORAGE_BINNED8_ID, DEFAULT_CONF_CHUNK_SIZE);
	
	// create storage for images histograms
	out *= createStorage(STORAGE_HISTOGRAMS_ID, DEFAULT_CONF_CHUNK_SIZE);
	
	// create storage for raw images
	out *= createStorage(STORAGE_RAW_ID, DEFAULT_CONF_CHUNK_SIZE);
	
	// create storage for bootup message
	out *= createStorage(STORAGE_BOOTUP_MESSAGE_ID, DEFAULT_CONF_CHUNK_SIZE);
	
	return out;
}

uint32_t waitForTimeAck() {
	
	int32_t time;
	
	if (pdTRUE == xQueueReceive(xCSPTimeQueue, &time, 100)) {
		
		return (uint32_t) time;
	}

	return 0;
}

uint32_t my_ntho32(uint32_t in) {
	
	uint32_t out;
	uint8_t * tempPtr = (uint8_t *) &in;
	uint8_t * tempPtr2 = (uint8_t *) &out;
	uint8_t l;
	
	for (l = 0; l < 4; l++) {
		
		*(tempPtr2 + l) = *(tempPtr + 3 - l);
	}
	
	return out;
}

uint32_t getTime() {
	
	csp_cmp_msg_t * msg = (csp_cmp_msg_t *) outcomingPacket->data;
	msg->type = 0; //CSP_CMP_REQUEST;
	msg->code = 6; //CSP_CMP_CLOCK;
	msg->tv_sec = 0; //MUST be set to 0; clock>0 will modify OBC time!
	msg->tv_nsec = 0;
		
	uint32_t time;
	
	uint8_t k;
	for (k = 0; k < 3; k++) {
				
		outcomingPacket->length = sizeof(csp_cmp_msg_t);
		csp_sendto(CSP_PRIO_NORM, CSP_OBC_ADDRESS, CSP_CMP, 20, CSP_O_NONE, outcomingPacket, 1000);
				
		if (pdTRUE == xQueueReceive(xCSPTimeQueue, &time, 100)) {
			
			return my_ntho32(time);
		}
	}
	
	return 0;
}

void getAttitude(adcs_att_t * attitude) {
	
	timestamp_t * req_time = (timestamp_t *) &outcomingPacket->data;
	
	req_time->tv_sec = my_ntho32(imageParameters.time) - 946684800;
	req_time->tv_nsec = 0;
		
	uint8_t k;
	for (k = 0; k < 3; k++) {
		
		outcomingPacket->length = sizeof(timestamp_t);
		csp_sendto(CSP_PRIO_NORM, CSP_OBC_ADDRESS, OBC_PORT_ADCS, 19, CSP_O_NONE, outcomingPacket, 1000);
		
		if (pdTRUE == xQueueReceive(xCSPAttitudeQueue, &attitude, 100)) {
			
			uint8_t i;
			for (i = 0; i < 6; i++) {
				
				attitude->attitude[i] = csp_ntoh16(attitude->attitude);
			}
			
			for (i = 0; i < 3; i++) {
				
				attitude->position[i] = csp_ntoh16(attitude->position);
			}
			
			return 1;
		}
	}
	
	return 0;
}