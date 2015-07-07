/*
 * dkHandler.c
 *
 * Created: 7.7.2015 14:36:58
 *  Author: klaxalk
 */ 

#include "system.h"
#include "dkHandler.h"
#include "mainTask.h"

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