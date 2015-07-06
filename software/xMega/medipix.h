/*
 * medipix.h
 *
 * Created: 25.1.2015 20:11:49
 *  Author: klaxalk
 */ 

#ifndef MEDIPIX_H_
#define MEDIPIX_H_

uint8_t medipixPowered();
void pwrOffMedipix();
void pwrOnMedipix();
void pwrToggleMedipix();

void eraseMatrix();
void closeShutter();
void openShutter();

void MpxDACstreamTimepix(uint8_t * buff);
void setDACs();

uint16_t getRntRaw(uint16_t idx);
uint8_t loadEqualization(uint16_t * data, uint8_t * outputBitStream);

void readMatrix();

#define FILTERING_ON	1
#define FILTERING_OFF	0

#define MODE_MEDIPIX	0
#define MODE_TIMEPIX	1

// Order of Timepix DACs
enum MPX_DACS_ORDER_TIMEPIX {
	MPX_DAC_TPX_IKRUM     = 0,
	MPX_DAC_TPX_DISC      = 1,
	MPX_DAC_TPX_PREAMP    = 2,
	MPX_DAC_TPX_BUFFA     = 3,
	MPX_DAC_TPX_BUFFB     = 4,
	MPX_DAC_TPX_HIST      = 5,
	MPX_DAC_TPX_THLFINE   = 6,
	MPX_DAC_TPX_THLCOARSE = 7,
	MPX_DAC_TPX_VCAS      = 8,
	MPX_DAC_TPX_FBK       = 9,
	MPX_DAC_TPX_GND       = 10,
	MPX_DAC_TPX_THS       = 11,
	MPX_DAC_TPX_BIASLVDS  = 12,
	MPX_DAC_TPX_REFLVDS   = 13
};

typedef struct {
	int16_t dacVals[15];       // DAC values. Order is given by enumerations MPX_DACS_ORDER or MPX_DACS_ORDER_MXR or MPX_DACS_ORDER_TIMEPIX
	uint8_t   DACcode;           // Code of sense or bypassed DAC
	uint8_t   Sense;             // Should be DACcode sensed?
	uint8_t Bypass;            // Should be DACcode bypassed?
	uint32_t   TestPulseConfig;   // Test pulse configuration register: Valid just for MXR chips
} Mpx_DAC;

typedef struct {
	// first dword  = bit 0-31, or 255-224 ?
	uint32_t unused0:3; uint32_t ikrum:8; uint32_t disc:8; uint32_t preamp:8; uint32_t unused1:5;

	// second dword = bit 32-63, or 223-192 ?
	uint32_t unused2:5; uint32_t codeLow2b:2; uint32_t unused3:1; uint32_t codeHigh2b:2; uint32_t senseDac:1;
	uint32_t extDac:1; uint32_t unused4:1; uint32_t buffALow4b:4; uint32_t unused5:6; uint32_t buffAHigh4b:4; uint32_t buffBLow5b:5;

	// third dword = bit 64-95, or 191-160 ?
	uint32_t buffBHigh3b:3; uint32_t unused6:18; uint32_t hist:8; uint32_t unused7:3;

	// fourth dword = bit 96-127, or 159-128 ?
	uint32_t unused8:3; uint32_t thlFine:10; uint32_t thlCoarse:4; uint32_t unused9:6; uint32_t vCas:8; uint32_t fbkLow1b:1;

	// fifth dword = bit 128-159, or 127-96 ?
	uint32_t fbkHigh7b:7; uint32_t gnd:8; uint32_t ctprLow17b:17;

	// sixth dword = bit 160-191, or 95-64 ?
	uint32_t ctprHigh15b:15; uint32_t unused10:5; uint32_t ths:8; uint32_t unused11:4;

	// seventh dword = bit 192-223, or 63-32 ?
	uint32_t unused12:32;

	// eigth dword = bit 224-255, or 31-0 ?
	uint32_t unused13:2; uint32_t biasLvds:8; uint32_t refLvds:8; uint32_t unused14:14;
} DACsTimepix;

typedef enum {
	BINNING_1 = 0,
	BINNING_8 = 1,
	BINNING_16 = 2,
	BINNING_32 = 3,
	HISTOGRAMS = 4,
} OUTPUT_FORMS;

// structure that hold all parameters of the measurement
typedef struct {
	
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
	
	// 0 -> 1 binning
	// 1 -> 8 binning
	// 2 -> 16 binning
	// 3 -> 32 binning
	// 4 -> histograms
	uint8_t outputForm;
	
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
	
	// TBD attitude
	
	// TBD timestamp
	
} imageParameters_t;

// this structure is send via CSP to set image parameters
typedef struct {
	
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
	
} newSettings_t;

// board commands
typedef enum {
	MEDIPIX_PWR_ON = 0,						// initialize medipix
	MEDIPIX_PWR_OFF = 1,					// power off medipix
	MEDIPIX_SET_ALL_PARAMS = 2,				// set all aquisition parameters (save to fram)
	MEDIPIX_SET_THRESHOLD = 3,				// set new treshold
	MEDIPIX_SET_BIAS = 4,					// set new bias
	MEDIPIX_SET_EXPOSURE = 5,				// set new exposure
	MEDIPIX_SET_FILTERING = 6,				// set new filtering mode
	MEDIPIX_SET_MODE = 7,					// set new aquisition mode
	MEDIPIX_SET_OUTPUT_FORM = 8,			// set new output form
	MEDIPIX_MEASURE = 9,					// take a shot
	MEDIPIX_MEASURE_WITH_PARAMETERS = 10,	// take a shot with all parameters specified in the command
	MEDIPIX_MEASURE_WITHOUT_DATA = 11,		// take a shot but only send metadata
	MEDIPIX_SEND_ORIGINAL = 12,				// send the original image
	MEDIPIX_SEND_FILTERED = 13,				// send the filtered image
	MEDIPIX_SEND_BINNED = 14,				// send binned/histogram image
	MEDIPIX_SEND_METADATA = 15,				// send only the image metadata
	MEDIPIX_MEASURE_NO_TURNOFF = 16,		// dont turn off medipix after measurement
	MEDIPIX_GET_BOOTUP_MESSAGE = 17,		// return the medipix's bootup message from the last boot
	MEDIPIX_GET_TEMPERATURE = 18,		// return the medipix's bootup message from the last boot
} MPX_MATLAB_COMMANDS;

volatile uint8_t ioBuffer[448];
volatile uint8_t tempBuffer[256];
volatile uint16_t dataBuffer[256];

volatile imageParameters_t imageParameters;

#endif /* MEDIPIX_H_ */