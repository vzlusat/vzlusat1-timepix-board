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
#include "csp_endian.h"

uint8_t createStorage(uint8_t id, uint16_t size) {
	
	dk_msg_create_t * message = (dk_msg_create_t *) outcomingPacket->data;
	
	message->parent.cmd = DKC_CREATE;
	message->port = id;
	message->conf_siz = csp_hton16(size);
	
	uint8_t k;
	for (k = 0; k < 3; k++) {

		outcomingPacket->length = sizeof(dk_msg_create_t);

		csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);

		if (waitForDkAck() == 1) {
			
			return 1;
		}
	}
	
	return 0;
};

uint8_t removeStorage(uint8_t id) {
	
	dk_msg_storage_t * message = (dk_msg_storage_t *) outcomingPacket->data;
	
	message->parent.cmd = DKC_WIPE;
	message->port = id;
	message->host = CSP_DK_MY_ADDRESS;
	
	uint8_t k;
	for (k = 0; k < 3; k++) {

		outcomingPacket->length = sizeof(dk_msg_create_t);

		csp_sendto(CSP_PRIO_NORM, CSP_DK_ADDRESS, CSP_DK_PORT, 18, CSP_O_NONE, outcomingPacket, 1000);

		if (waitForDkAck() == 1) {
			
			return 1;
		}
	}
	
	return 0;
}

uint8_t clearStorage(uint8_t id) {
	
	if (removeStorage(id) == 1) {
		
		if (createStorage(id, DEFAULT_CONF_CHUNK_SIZE) == 1) {
			
			return 1;
		} else {
			
			return 0;
		}
	} else {
		
		return 0;
	}
}

uint8_t createStorages() {
	
	// clear all storages
	
	uint8_t i;
	uint8_t out = 1;
	
	for (i = 1; i <= NUMBER_OF_STORAGES; i++) {
		
		clearStorage(i);
		out *= createStorage(i, DEFAULT_CONF_CHUNK_SIZE);
	}
	
	return out;
}

uint32_t waitForTimeAck() {
	
	int32_t time;
	
	if (pdTRUE == xQueueReceive(xCSPTimeQueue, &time, 1000)) {
		
		return (uint32_t) time;
	}

	return 0;
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
			
			return csp_ntoh32(time);
		}
	}
	
	return 0;
}

uint8_t getAttitude(int16_t * attitude, int16_t * position) {
	
	timestamp_t * req_time = (timestamp_t *) &outcomingPacket->data;
	
	req_time->tv_sec = csp_hton32(imageParameters.time);
	req_time->tv_nsec = 0;
	
	adcs_att_t attitudeIn;
	uint8_t i;
		
	uint8_t k;
	for (k = 0; k < 3; k++) {
		
		outcomingPacket->length = sizeof(timestamp_t);
		csp_sendto(CSP_PRIO_NORM, CSP_OBC_ADDRESS, OBC_PORT_ADCS, 19, CSP_O_NONE, outcomingPacket, 1000);
		
		if (pdTRUE == xQueueReceive(xCSPAttitudeQueue, &attitudeIn, 300)) {
			
			for (i = 0; i < 7; i++) {
				
				attitude[i] = csp_ntoh16(attitudeIn.attitude[i]);
			}
			
			for (i = 0; i < 3; i++) {
				
				position[i] = csp_ntoh16(attitudeIn.position[i]);
			}
			
			return 1;
		}
	}
	
	for (i = 0; i < 7; i++) {
		
		attitude[i] = 0;
	}
	
	for (i = 0; i < 3; i++) {
		
		position[i] = 0;
	}
	
	return 0;
}