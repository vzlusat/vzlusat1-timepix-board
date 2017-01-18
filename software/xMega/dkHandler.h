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

#define NUMBER_OF_STORAGES		3

typedef enum {
	
	STORAGE_HK_ID				= 1,
	STORAGE_METADATA_ID			= 2,
	STORAGE_DATA_ID				= 3,
} DK_STORAGES_IDS;

#define DEFAULT_CONF_CHUNK_SIZE		20

/* obsolete, replaced on 18.1.2017
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
*/

typedef enum
{
	DKC_STORE = 0,       //Store a chunk of data.
	DKC_STORE_ACK = 1,   //Store a chunk of data and reply result
	DKC_GET_RAW = 2,     //Retrieve only a chunk of data
	DKC_GET_RICH = 3,    //Retrieve a data with all adational info
	DKC_GET_RANGE = 4,   //Retrive a range of chunks
	DKC_GET_NUM = 5,     //Retrive sepecified number of chunks
	DKC_FIND = 6,        //Retrive a chunk ID with certain store time
	DKC_INFO = 10,       //Information about one storage
	DKC_LIST = 11,       //List of all storages information
	DKC_INFO_RICH = 12,  //Rich information about one storage
	DKC_CREATE = 13,     //Create new storage
	DKC_WIPE = 14,       //Delete all data from storage
	DKC_CLEAN = 15,      //Clean chunks from storage
	DKC_STORE_FULL = 16, //Store a chunk with defined time and flags
	DKC_CREATE_FOR = 17, //Create new storage for host
	DKC_STRIP = 18,      //Delete 1/2 of storage data (oldest)
	DKC_DELETE = 19,     //Delete a storage with all it's data
	DKC_STATS = 20,      //Statistics about space and storages
	DKC_MAINTENANCE = 21,//Repair storages and actual remove cleaned chunks
	DKC_INIT = 22,       //Re-load cache from files
	DKC_BCK_ONOFF = 23,  //mount/umount backup and start/stop backuping
	DKC_BCK_SWAP = 24    //Swap main and backup storage
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

typedef struct __attribute__((packed))
{ //DKC_INFO, DKC_INFO_RICH, DKC_REMOVE, DKC_MAINTENANCE
	dk_msg_t parent;
	uint8_t host; //storage identification
	uint8_t port; //storage sub-identification
} dk_msg_storage_t;

/* old and obsolete, replaced on 18.1.2017
typedef struct __attribute__((packed))
{ //DKC_INFO, DKC_LIST
	uint8_t host; //storage identification
	uint8_t port; //storage sub-identification
	uint32_t chunks; //number of data chunks in a storage
	uint32_t write_tim; //since 01.01.2000 00:00:00 UTC
} dk_reply_info_t;
*/

typedef struct __attribute__((packed))
{ //DKC_INFO, DKC_LIST
	uint8_t host;        //storage identification
	uint8_t port;        //storage sub-identification
	uint8_t errors;      //number of read / write / open errors
	uint32_t newest_chunk_id;
	uint32_t write_tim;  //since 01.01.2000 00:00:00 UTC
	uint32_t oldest_chunk_id;
} dk_reply_info_t;

uint8_t createStorages();
uint32_t getTime();
uint8_t getAttitude(int16_t * attitude, int16_t * position);
uint8_t clearStorage(uint8_t id);
uint32_t waitForTimeAck();

#endif /* DKHANDLER_H_ */