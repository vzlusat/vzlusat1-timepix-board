/*
 * medipix.h
 *
 * Created: 25.1.2015 20:11:49
 *  Author: klaxalk
 */ 


#ifndef MEDIPIX_H_
#define MEDIPIX_H_

char medipixPowered();
void pwrOffMedipix();
void pwrOnMedipix();
void pwrToggleMedipix();

void eraseMatrix();
void closeShutter();
void openShutter();

void medipixInit();

uint16_t getRntRaw(uint16_t idx);
uint8_t loadEqualization(uint16_t * data, uint8_t * outputBitStream);

void readMatrix();

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

// Pixel configuration as it is stored in binary file:
typedef struct {
	uint8_t lowTh: 3;          // low threshold (3 bits, low (0) is ACTIVE)
	uint8_t highTh: 3;         // high threshold (3 bits, low (0) is ACTIVE)
	uint8_t testBit: 1;        // test bit (1 bit, low (0) is ACTIVE)
	uint8_t maskBit: 1;        // mask bit (1 bit, low (0) is ACTIVE)
} PixelCfg;

volatile uint8_t ioBuffer[448];
volatile uint8_t tempBuffer[256];
volatile uint16_t dataBuffer[256];

#endif /* MEDIPIX_H_ */