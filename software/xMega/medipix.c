/*
 * medipix.c
 *
 * Created: 25.1.2015 20:11:33
 *  Author: klaxalk
 */ 

#include "system.h"
#include "medipix.h"
#include "equalization.h"
#include "mainTask.h"
#include "pseudoTables.h"

// 1 if medipix is powered
// 0 if medipix is turned off
char medipixOnline;

const uint16_t DefaultDacValsTimepix[15] = {1  ,127,255,127,127,  0,450,7,130,128, 80, 85,128,128,  0};

volatile Mpx_DAC DAC;

volatile uint8_t ioBuffer[448];
volatile uint8_t tempBuffer[256];
volatile uint16_t dataBuffer[256];

// 'd' = Set all DACS, if numofchips is 0 the device default will be used
void MpxSetDACs() {
	
	char inChar;
	uint8_t i;

	#define MAX_DACS_STREAM_LEN 40 

	uint8_t stream[MAX_DACS_STREAM_LEN];
	
	stream[0] = 'd';

	for (i = 1; i<MAX_DACS_STREAM_LEN; i++)
		stream[i]=0xff;
	
	// nejdøív zapsat s jednièkama tady poslat první byte ze streamu
	for (i = 0; i < 34; i++) {
		usartBufferPutByte(medipix_usart_buffer, stream[i], 1000);
		if (i == 0)
			vTaskDelay(5);
	}

	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
		
		if (inChar == '\r')
		inChar = '<';
		
		if (inChar == '\n')
		inChar = '_';
		
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
	}
	
	stream[0]='d';
	
	// Pak naplnit
	MpxDACstreamTimepix((uint8_t *) stream+1);
	
	// nejdøív zapsat s jednièkama tady poslat první byte ze streamu
	for (i = 0; i < 34; i++) {
		usartBufferPutByte(medipix_usart_buffer, stream[i], 1000);
		if (i == 0)
			vTaskDelay(5);
	}
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 2000)) {
		
		if (inChar == '\r')
		inChar = '<';
		
		if (inChar == '\n')
		inChar = '_';
		
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
	}
}

// Shifts data in the buffer by "shift" bits to right
// if "shift" is negative shifts to left
void bitShift(uint8_t* data, uint16_t datalen, uint16_t shift){
	uint16_t i;
	
	if (!shift)
	return;
	if (shift>0){
		data+=(datalen-1);
		for (i=1;i<datalen;i++,data--)
		*data=((*(data-1)<<(8-shift))&0xff) | (*data>>shift);
		(*data)>>=shift;
		}else{
		shift=-shift;
		for (i=0;i<datalen-1;i++,data++)
		*data=(*(data+1)>>(8-shift)) | ((*data<<shift)&0xff);
		(*data)<<=shift;
	}
}

void MpxDACstreamTimepix(uint8_t * buff) {
	// Single dummy byte:
	buff[0]=0xff;

	// Prepare buffer (fill it with zeroes):
	buff++;
	memset(buff, 0, 32);
	
	DACsTimepix *single;

	// Fill by dac values chip by chip:
	single=(DACsTimepix*) buff;

	// DACs:
	single->ikrum       = DAC.dacVals[MPX_DAC_TPX_IKRUM];
	single->disc        = DAC.dacVals[MPX_DAC_TPX_DISC];
	single->preamp      = DAC.dacVals[MPX_DAC_TPX_PREAMP];
	single->buffALow4b  = DAC.dacVals[MPX_DAC_TPX_BUFFA] & 0x000f;
	single->buffAHigh4b = DAC.dacVals[MPX_DAC_TPX_BUFFA] >> 4;
	single->buffBLow5b  = DAC.dacVals[MPX_DAC_TPX_BUFFB] & 0x001f;
	single->buffBHigh3b = DAC.dacVals[MPX_DAC_TPX_BUFFB] >> 5;
	single->hist        = DAC.dacVals[MPX_DAC_TPX_HIST];
	single->thlFine     = DAC.dacVals[MPX_DAC_TPX_THLFINE];
	single->thlCoarse   = DAC.dacVals[MPX_DAC_TPX_THLCOARSE];
	single->vCas        = DAC.dacVals[MPX_DAC_TPX_VCAS];
	single->fbkLow1b    = DAC.dacVals[MPX_DAC_TPX_FBK] & 0x0001;
	single->fbkHigh7b   = DAC.dacVals[MPX_DAC_TPX_FBK] >> 1;
	single->gnd         = DAC.dacVals[MPX_DAC_TPX_GND];
	single->ths         = DAC.dacVals[MPX_DAC_TPX_THS];
	single->biasLvds    = DAC.dacVals[MPX_DAC_TPX_BIASLVDS];
	single->refLvds     = DAC.dacVals[MPX_DAC_TPX_REFLVDS];

	// Test pulse:
	single->ctprLow17b  = DAC.TestPulseConfig;
	single->ctprHigh15b = DAC.TestPulseConfig >> 17;

	single->codeLow2b   = DAC.DACcode & 0x03;
	single->codeHigh2b  = DAC.DACcode >> 2;
	single->senseDac    = DAC.Sense;
	single->extDac      = DAC.Bypass;

	// It is in reverse order !!!
	char pom;
	for (int i=0;i<16;i++) {
		pom=buff[i];
		buff[i]=buff[31-i];
		buff[31-i]=pom;
	}
	// Try to shift by 1 bit:
	bitShift(buff, 32, -1);
}

void medipixInit() {

	memcpy(DAC.dacVals, DefaultDacValsTimepix, 15*sizeof(uint16_t));
	DAC.DACcode=11;
	DAC.Sense = 1;
	DAC.Bypass = 0;
	DAC.TestPulseConfig=0xff00ff00;
	
	MpxSetDACs();
}

char medipixPowered() {
	
	return medipixOnline;
}

// Number of bytes in stream containing one MXR row
#define MPXMXR_ROW_BYTES (32*14)

// Deserialization of data stream from one MXR chip
// No dummy byte is expected:

void MpxBitStream2DataSingleMXR(uint8_t * byteStream, uint16_t * data) {
	
	int16_t j, k;
	int16_t byteIdx;
	uint16_t bitMsk, valMsk;

	for (j = 255; j >= 0; j--) {                                                 // Loop for 256 pixels of i-th row
		
		byteIdx = (j>>3);                                            // index of first byte in bytestream which contains data bits for [i,j] element in data matrix
		bitMsk = 1 << ((~j) & 0x7);                                                // mask of bit for [i,j] element in byte on byteIdx, bits are in reverse order (=> ~j instead of j)
		for (k=0, *data=0, valMsk=1; k<14; k++, valMsk <<=1, byteIdx-=32)         // Loop for 13 bits of j-th pixel in i-th row
		if (*(byteStream + byteIdx) & bitMsk)
			(*data) |= valMsk;
		
		data++;
	}
}

void MpxData2BitStreamSingleMXR(uint16_t * data, uint8_t * byteStream) {
	
	int16_t j, k;
	int16_t byteIdx;
	uint16_t bitMsk, valMsk;
	
	memset(byteStream, 0, 448);

	for (j = 255; j >= 0; j--){        
		                                         // Loop for 256 pixels of i-th row
		byteIdx = (j>>3);                                            // index of first byte in bytestream which contains data bits for [i,j] element in data matrix
		bitMsk = 1 << ((~j) & 0x7);                                               // mask of bit for [i,j] element in byte on byteIdx, bits are in reverse order (=> ~j instead of j)
		for (k=0, valMsk=1; k<14; k++, valMsk <<=1, byteIdx-=32)                  // Loop for 13 bits of j-th pixel in i-th row
		if ((*data) & valMsk)
			*(byteStream + byteIdx) |= bitMsk;
		
		data++;
	}
}

// Derandomization for MXR:
void MpxConvertValuesMXR(uint16_t * values){

	int i;
	int16_t sum = 0;
	
	for (i = 0; i < 256; i++) {
		
		if (*values < 16384) {
			
			if (*values < 8192)
				*values = pgm_read_word(&(pseudo2Count1[*values]));
			else
				*values = pgm_read_word(&(pseudo2Count2[*values - 8192]));
			
				sum += *values;
			} else {
				*values = 0xFFFF;
		}
		
		values++;
	}
}

void pwrOnMedipix() {
	
	ioport_set_pin_level(MEDIPIX_PWR, true);
	medipixOnline = 1;
	
	// pocka az nastartuje

	sendBlankLine(15, 16);
	
	char inChar;
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 4000)) {
	
		if (inChar == '\r')
		inChar = '<';
	
		if (inChar == '\n')
		inChar = '_';
	
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
	}

	sendBlankLine(15, 16);

	medipixInit();
	
	sendBlankLine(15, 16);
	
	loadEqualization(&dataBuffer, &ioBuffer);
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 4000)) {
		
		if (inChar == '\r')
		inChar = '<';
		
		if (inChar == '\n')
		inChar = '_';
		
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
	}
	
	sendBlankLine(15, 16);
	
	/*
	
	loadEqualization(&dataBuffer, &ioBuffer);
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
		
		if (inChar == '\r')
		inChar = '<';
		
		if (inChar == '\n')
		inChar = '_';
		
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
	}
	
	*/

	readMatrix();
}

void pwrOffMedipix() {
	
	ioport_set_pin_level(MEDIPIX_PWR, false);
	medipixOnline = 0;
}

void pwrToggleMedipix() {
	
	if (medipixOnline == 1) {
		
		pwrOffMedipix();
		
	} else {
		
		pwrOnMedipix();	
	}
}

// Derandomization for MXR:
uint8_t getEqualizationRaw(uint16_t idx){

	if (idx < 8192) {
		
		return pgm_read_word(&(equalization1[idx]));
	} else if (idx < 2*8192) {

		return pgm_read_word(&(equalization2[idx]));
	} else if (idx < 3*8192) {

		return pgm_read_word(&(equalization3[idx]));
	} else if (idx < 4*8192) {

		return pgm_read_word(&(equalization4[idx]));
	} else if (idx < 5*8192) {

		return pgm_read_word(&(equalization5[idx]));
	} else if (idx < 6*8192) {

		return pgm_read_word(&(equalization6[idx]));
	} else if (idx < 7*8192) {

		return pgm_read_word(&(equalization7[idx]));
	} else if (idx < 8*8192) {

		return pgm_read_word(&(equalization8[idx]));
	}
	
	return 1;
}

uint8_t loadEqualization(uint16_t * data, uint8_t * outputBitStream) {
	
	uint16_t i, j, k;
	PixelCfg val;
	
	uint8_t * valPoint = (uint8_t *) &val;
	
	uint16_t * Mask;
	
	usartBufferPutByte(medipix_usart_buffer, 'S', 1000);
	
	vTaskDelay(10);
	
	for (i = 0; i < 256; i++) {
		
		Mask = data;
		
		for (j = 0; j < 256; j++) {
			
			*valPoint = getEqualizationRaw(256*i+j);
			
			//*Mask = val.maskBit | ((!val.testBit) << 9) | ((val.lowTh  & 0x01) << 7) | ((val.lowTh & 0x02) << 5) | ((val.lowTh  & 0x04) << 6) | ((val.highTh  & 0x01) << 12) | ((val.highTh  & 0x02) << 9) | ((val.highTh  & 0x04) << 9);
			
			*Mask = 0xEEEE;
			
			Mask++;
		}
		
		// MpxData2BitStreamSingleMXR(&dataBuffer, &ioBuffer);
		
		outcomingPacket->data[0] = 'X';
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(5);
				
		for (k = 0; k < 448; k++) {
			
			usartBufferPutByte(medipix_usart_buffer, 0xDD, 1000);
		}
	}
	
	// send dummy 33 bytes
	for (k = 0; k < 33; k++) {
		usartBufferPutByte(medipix_usart_buffer, 'A', 1000);
	}
	
	vTaskDelay(100);
	
	return 0;
}

void readMatrix() {
	
	char inChar;
		
	int16_t actIncomingPosition = 0;
	
	char measuringProceeding = 0;
	
	char temp[40];
	
	int16_t rowByteIdx = 0;
		
	int16_t bytesInBuffer = 0;
	int16_t bytesInOverBuffer = 0;
	uint8_t bufferFull = 0;
	int16_t receivedBytes = 0;
	int16_t rowsReceived = 0;
	
	memset(ioBuffer, 0, 448);
	memset(dataBuffer, 0, 512);
	memset(tempBuffer, 0, 256);
	
	usartBufferPutByte(medipix_usart_buffer, 'm', 1000);
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 4000)) {break;}
			
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
				
		receivedBytes++; // poèet bajtù pøijatých "tento øádek"
			 
		if (!bufferFull) { // pokud nemáme plný ioBuffer
			
			ioBuffer[bytesInBuffer++] = inChar; // pøidá pøijatý bajt do ioBuffer
					
			if (bytesInBuffer == 448) // pokud jsme tímto znakem naplnili buffer
				bufferFull = 1;
					
		} else {
			tempBuffer[bytesInOverBuffer++] = inChar; // pokud je plný buffer, dávám data to tempBufferu
		}
		
		// pokud došel celý datagram ale nemám naplnìný buffer, zažádám o další
		if (receivedBytes == 256 && bufferFull == 0) {
			
			receivedBytes = 0;
			usartBufferPutByte(medipix_usart_buffer, 'i', 1000);
		}

		// receive the dummy line
		if (rowsReceived == 256) {
			
				outcomingPacket->data[0] = inChar;
				outcomingPacket->data[1] = 0;
				outcomingPacket->length = 2;
				csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
				vTaskDelay(5);
		}
		
		// pokud mám plný buffer a pøišel další datagram, zpracuju øádek
		if (receivedBytes == 256 & bufferFull == 1) {
			
			MpxBitStream2DataSingleMXR(&ioBuffer, &dataBuffer);
			
			MpxConvertValuesMXR(&dataBuffer);
			
			// pøenese pøevis z tempBufferu do ioBufferu
			memcpy(ioBuffer, tempBuffer, bytesInOverBuffer);
			
			// pøemístí ukazovátko na správné místo v tempBufferu
			bytesInBuffer = bytesInOverBuffer;
			receivedBytes = 0;
			bytesInOverBuffer = 0;
			rowsReceived++;
			bufferFull = 0;
			
			// výpis
			sprintf(temp, "Row %d, %d %d %d\n\r", rowsReceived, ioBuffer[0], ioBuffer[1], ioBuffer[2]);
			strcpy(outcomingPacket->data, temp);
			outcomingPacket->length = strlen(temp);
			csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
			
			usartBufferPutByte(medipix_usart_buffer, 'i', 1000);
		}
		
	}
}