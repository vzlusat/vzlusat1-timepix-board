/*
 * medipixHeaders.h
 *
 * Created: 28.6.2015 10:46:22
 *  Author: klaxalk
 */ 


#ifndef MEDIPIXHEADERS_H_
#define MEDIPIXHEADERS_H_

#define FILTERING_ON	1
#define FILTERING_OFF	0

#define MODE_MEDIPIX	0
#define MODE_TIMEPIX	1

#define	BINNING_1	0
#define	BINNING_8	1
#define	BINNING_16	2
#define	BINNING_32	3
#define HISTOGRAMS	4

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

#endif /* MEDIPIXHEADERS_H_ */