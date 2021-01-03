#ifndef XRAY_H
#define XRAY_H

#include <stdint.h>

/** Default node address */
#define NODE_XRAY				4

/** Port numbers */
#define XRAY_PORT_DIRECT			16
#define XRAY_PORT_DK				17

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
	MEDIPIX_GET_HOUSEKEEPING = 19,						// return the main houskeeping information
	MEDIPIX_MEASURE_WITHOUT_DATA_NO_TURNOFF = 20,		// return the main houskeeping information
	XRAY_DK_CREATE_STORAGES = 21,						// create all storages in the data keeper
	MEDIPIX_SET_PIXELCNTTHR = 22,						// pixel count threshold for scanning mode
	MEDIPIX_SET_TEMPLIMIT = 23,							// temperature limit for medipix
	MEDIPIX_MEASURE_SCANNING_MODE = 24,					// similar as "MEDIPIX_MEASURE", but saves the data only if number of pixels (after filtration) exceeds a treshold
	MEDIPIX_MEASURE_SCANNING_MODE_NO_TURNOFF = 25,		// -||- but not turnoff
	MEDIPIX_SEND_SENSOR_DATA = 26,						// get data from the IR and UV sensors
	MEDIPIX_MEASURE_UV = 27,							// measuring triggered by UV1 sensor
	MEDIPIX_SET_UV1THL = 28,							// set the UV1 treshold
} MPX_COMMANDS;	

#define THRESHOLD_UPPER_BOUND	500

// this structure is send via CSP to set image parameters
typedef struct __attribute__((packed)) {
	
	uint16_t treshold;
	
	uint16_t exposure;
	
	uint8_t bias;
	
	// 0 -> off
	// 1 -> on
	uint8_t filtering;
	
	// 0 -> TPX
	// 1 -> TOT
	uint8_t mode;
	
	// 0 -> 1 binning
	// 1 -> 8 binning
	// 2 -> 16 binning
	// 3 -> 32 binning
	// 4 -> histograms
	uint8_t outputForm;
	
	// temperature limit for measuring
	int8_t temperatureLimit;
	
	// pixel count threshold for image catcher mode
	uint16_t pixelCountThr;
	
	// UV1 treshold
	uint16_t uv1_treshold;
	
} newSettings_t;

typedef struct __attribute__((packed))
{
  uint8_t cmd;
} xray_msg_cmd_t;

typedef struct __attribute__((packed))
{
  uint8_t cmd;
  uint8_t data;
} xray_msg_cmd_1byte_t;

typedef struct __attribute__((packed))
{
  uint8_t cmd;
  int8_t data;
} xray_msg_cmd_int8_t;

typedef struct __attribute__((packed))
{
  uint8_t cmd;
  uint16_t data;
} xray_msg_cmd_1uin16_t;

typedef struct __attribute__((packed))
{
  uint8_t cmd;
  int16_t data;
} xray_msg_cmd_1in16_t;

typedef struct __attribute__((packed))
{
  uint8_t cmd;
  newSettings_t data;
} xray_msg_cmd_params_t;

typedef struct __attribute__((packed))
{
  int8_t temp;       
} xray_temperature_t;

typedef struct __attribute__((packed)) {
	
	uint8_t packetType;
	uint16_t bootCount;
	uint16_t imagesTaken;
	uint8_t temperature;
	uint8_t framStatus;
	uint8_t medipixStatus;
	uint32_t seconds;
	uint16_t TIR_max;
	uint16_t TIR_min;
	uint16_t IR_max;
	uint16_t IR_min;
	uint16_t UV1_max;
	uint16_t UV1_min;
	uint16_t UV2_max;
	uint16_t UV2_min;
	uint8_t temperature_max;
	uint8_t temperature_min;
} hk_data_t;

// structure that hold all parameters of the measurement
typedef struct __attribute__((packed)) {
	
	uint8_t packetType;

	// 0 -> 1 binning
	// 1 -> 8 binning
	// 2 -> 16 binning
	// 3 -> 32 binning
	// 4 -> histograms
	uint8_t outputForm;
	
	// 0 -> 65535
	uint16_t imageId;
	
	// 0 -> TPX
	// 1 -> TOT
	uint8_t mode;
	
	uint16_t threshold;
	
	uint8_t bias;
	
	uint16_t exposure;
	
	// 0 -> off
	// 1 -> on
	uint8_t filtering;
	
	// number of non-zero pixels in the output image
	uint16_t nonZeroPixelsFiltered;
	
	// number of non-zerou pixel in the original image
	uint16_t nonZeroPixelsOriginal;
	
	// minimal (nonzero) and maximal value of the pixel in the original image
	uint8_t minValueOriginal;
	uint8_t maxValueOriginal;

	// minimal (nonzero) and maximal value of the pixel in the filtered image
	uint8_t minValueFiltered;
	uint8_t maxValueFiltered;
	
	// temperature from the ADT sensor
	int8_t temperature;
	
	// temperature limit for measuring
	int8_t temperatureLimit;
	
	// pixel count threshold for image catcher mode
	uint16_t pixelCountThr;
	
	// UV1 treshold
	uint16_t uv1_treshold;
	
	// id of the chunk in the storage
	uint32_t chunkId;
	
	// position from ADCS
	int16_t attitude[7];
	
	// position from ADCS
	int16_t position[3];
	
	// time from OBC.. unix time?
	uint32_t time;
	
} imageParameters_t;

typedef struct __attribute__ ((packed)) {
	int16_t TIR;
	int16_t IR;
	int16_t UV1;
	int16_t UV2;
} sensors_t;

#endif // XRAY_H
