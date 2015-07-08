/*
 * dkHandler.h
 *
 * Created: 7.7.2015 14:37:09
 *  Author: klaxalk
 */ 


#ifndef DKHANDLER_H_
#define DKHANDLER_H_

#define CSP_DK_ADDRESS			1
#define CSP_DK_PORT				33

#define CSP_LOG_ADDRESS			1
#define CSP_LOG_STORE_PORT		36

#define CSP_DK_MY_ADDRESS		CSP_MY_ADDRESS

#define OBC_PORT_ADCS			20

typedef enum {
	
	STORAGE_SETTINGS_ID			= 1,
	STORAGE_HK_ID				= 2,
	STORAGE_METADATA_ID			= 3,
	STORAGE_BINNED32_ID			= 4,
	STORAGE_BINNED16_ID			= 5,
	STORAGE_BINNED8_ID			= 6,
	STORAGE_HISTOGRAMS_ID		= 7,
	STORAGE_RAW_ID				= 8,
	STORAGE_BOOTUP_MESSAGE_ID	= 9,
} DK_STORAGES_IDS;

#define DEFAULT_CONF_CHUNK_SIZE		20

typedef enum {
	DKC_STORE = 0,			//Store a chunk of data.
	DKC_STORE_ACK = 1,		//Store a chunk of data and reply result
	DKC_GET_RAW = 2,		//Retrieve only a chunk of data
	DKC_GET_RICH = 3,		//Retrieve only a chunk of data
	DKC_GET_RANGE = 4,		//Retrive a range of chunks
	DKC_INFO = 10,			//Information about one storage
	DKC_LIST = 11,			//List of all storages information
	DKC_INFO_RICH = 12,		//Rich information about one storage
	DKC_CREATE = 13,		//Create new storage
	DKC_REMOVE = 14,		//Delete a storage with all it's data
	DKC_CLEAN = 15,			//Clean chunks from storage
	DKC_STATS = 20,			//Statistics about space and storages
	DKC_MAINTENANCE = 21	//Repair storages and actual remove cleaned chunks

} DK_COMMAND_ENUM;

typedef struct __attribute__((packed))
{ //DKC_LIST, DKC_STATS,
	uint8_t cmd; //DK_COMMAND_ENUM
} dk_msg_t;

typedef struct __attribute__((packed))
{ //DKC_CREATE
	dk_msg_t parent;
	uint8_t port; //storage sub-identification
	uint16_t conf_siz; //size to reserve for configuration
} dk_msg_create_t;

typedef struct __attribute__((packed))
{ //DKC_STORE_ACK
	dk_msg_t parent;
	uint8_t host; //storage identification
	uint8_t port; //identify where data to be stored (sub_id)
	//bits 6-0 is storage number from 0 to 128.
	//bit 7 identify first data chunk - configurations
	uint8_t data[0]; //array of data
} dk_msg_store_ack_t;

typedef struct __attribute__((packed))
{ //DKC_STORE_ACK, DKC_CREATE, DKC_REMOVE, DKC_CLEAN
	uint8_t host; //storage identification
	uint8_t port; //storage sub-identification
	int err_no; //internal error number or 0 when success
} dk_reply_common_t;

typedef struct __attribute__((packed))
{ //DKC_STORE
	dk_msg_t parent;
	uint8_t port; //identify where data to be stored (sub_id)
	//bits 6-0 is storage number from 0 to 128.
	//bit 7 identify first data chunk - configurations
	uint8_t data[0]; //array of data
} dk_msg_store_t;

typedef struct __attribute__ ((packed)) {
	uint8_t type;
	uint8_t code;
	uint32_t tv_sec;
	uint32_t tv_nsec;
} csp_cmp_msg_t;

typedef struct {
	uint32_t tv_sec;
	uint32_t tv_nsec;
} timestamp_t;

typedef struct __attribute__((packed)) {
	uint32_t tv_sec;
	uint32_t tv_nsec;
	int16_t attitude[7];
	int16_t position[3];
} adcs_att_t;

uint8_t createStorages();
uint32_t getTime();

#endif /* DKHANDLER_H_ */