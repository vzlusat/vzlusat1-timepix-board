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
#include "imageProcessing.h"
#include "avr/cpufunc.h"
#include "fram_mapping.h"

// 1 if medipix is powered
// 0 if medipix is turned off
uint8_t medipixOnline = 0;

uint16_t DefaultDacValsTimepix[15] = {1, 100, 255, 127, 127, 0, 314, 7, 130, 128, 80, 85, 128, 128, 0};
	
volatile Mpx_DAC DAC;

volatile imageParameters_t imageParameters;

volatile uint8_t ioBuffer[448];
volatile uint8_t tempBuffer[256];
volatile uint16_t dataBuffer[256];

uint8_t medipixCheckStatus() {
	
	char message[] = "!Unit is on line. Medipix TPXXX chips: 1 Medipix chips: 1 Ready";
	
	uint8_t i;
	for (i = 0; i < strlen(message); i++) {
		
		if (((uint8_t) message[i]) != ((uint8_t) spi_mem_read_byte(MEDIPIX_BOOTUP_MESSAGE + i)))
			return 0;
	}
	
	return 1;
}

void bin2hex(uint8_t in, uint8_t * out) {
	
	unsigned char ch;
	
	// creates the first hex character
	ch = in;
	ch = ch & 240;
	ch = ch >> 4;
	if (ch >= 0 && ch <= 9)
	ch = ch + '0';
	else
	ch = ch + 'A' - 10;
	*out = (uint8_t) ch;

	out++;

	// creates the second hex character
	ch = in;
	ch = ch & 15;
	if (ch >= 0 && ch <= 9)
	ch = ch + '0';
	else
	ch = ch + 'A' - 10;
	*out = (uint8_t) ch;
}

// 'd' = Set all DACS, if numofchips is 0 the device default will be used
void MpxSetDACs() {
	
	char inChar;
	uint8_t i;

	#define MAX_DACS_STREAM_LEN 40 

	uint8_t stream[MAX_DACS_STREAM_LEN];
	
	stream[0] = 'd';

	for (i = 1; i<MAX_DACS_STREAM_LEN; i++)
		stream[i]=0xff;
	
	// nejdøív zapsat s jednièkama
	for (i = 0; i < 34; i++) {
		
		usartBufferPutByte(medipix_usart_buffer, stream[i], 1000);
		vTaskDelay(5);
	}

	// ceka na potvrzeni nastaveni
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 2000)) {
		
		// TODO uložit zprávu z medipixu do fram k pozdìjšímu pøeètení

		// ukonci cekani na odpoved pri prijmuti posledniho znaku
		
		if (inChar == '\r')
			break;
	}
	
	vTaskDelay(40);
	
	stream[0]='d';
	
	// Pak naplnit
	MpxDACstreamTimepix((uint8_t *) stream+1);
		
	// odeslat skuteèné DAC
	for (i = 0; i < 34; i++) {
		
		usartBufferPutByte(medipix_usart_buffer, stream[i], 1000);
		vTaskDelay(5);
	}
		
	// ceka na potvrzeni nastaveni
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 2000)) {
		
		// TODO uložit zprávu z medipixu do fram k pozdìjšímu pøeètení
		
		// ukonci cekani na odpoved pri prijmuti posledniho znaku
		if (inChar == '\r')
			break;
	}
	
	vTaskDelay(40);
}

// Shifts data in the buffer by "shift" bits to right
// if "shift" is negative shifts to left
void bitShift(uint8_t* data, uint16_t datalen, int8_t shift){
	uint16_t i;
	
	if (!shift)
		return;
		
	if (shift>0){
	
		data+=(datalen-1);
		for (i=1;i<datalen;i++,data--)
			*data=((*(data-1)<<(8-shift))&0xff) | (*data>>shift);
		
		(*data)>>=shift;
	
	} else {
	
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
	
	buff+=32;
}

void setDACs(uint16_t thr) {
	
	DefaultDacValsTimepix[6] = thr;

	memcpy(DAC.dacVals, DefaultDacValsTimepix, 15*sizeof(uint16_t));
	DAC.DACcode=11;
	DAC.Sense = 1;
	DAC.Bypass = 0;
	DAC.TestPulseConfig=0xff00ff00;
	
	MpxSetDACs();
}

uint8_t medipixPowered() {
	
	return medipixOnline;
}

// Deserialization of data stream from one MXR chip
void MpxBitStream2DataSingleMXR(uint8_t * byteStream, uint16_t * data) {
	
	int16_t j, k;
	int16_t byteIdx;
	uint16_t bitMsk, valMsk;
	memset(data, 0, 256*sizeof(uint16_t));

	for (j = 255; j >= 0; j--) {                                                 // Loop for 256 pixels of i-th row
		
		byteIdx = 32*13 + (j>>3);                                            // index of first byte in bytestream which contains data bits for [i,j] element in data matrix
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
	
	memset(byteStream, 0, 448*sizeof(uint8_t));

	for (j = 255; j >= 0; j--) {        
																					// Loop for 256 pixels of i-th row
		byteIdx = 32*13 + (j>>3);															// index of first byte in bytestream which contains data bits for [i,j] element in data matrix
		bitMsk = 1 << ((~j) & 0x7);                                               // mask of bit for [i,j] element in byte on byteIdx, bits are in reverse order (=> ~j instead of j)
		for (k=0, valMsk=1; k<14; k++, valMsk <<=1, byteIdx-=32)                  // Loop for 13 bits of j-th pixel in i-th row
		if ((*data) & valMsk)
			byteStream[byteIdx] |= bitMsk;
		
		data++;
	}
}

// Derandomization for MXR:
void MpxConvertValuesMXR(uint16_t * values) {

	uint16_t i;
	
	for (i = 0; i < 256; i++) {
		
		if (values[i] < 16384) {
			
			values[i] = getRntRaw(values[i]);
				
		} else {
			values[i] = 0xFFFF;
		}
	}
}

uint16_t getRntRaw(uint16_t idx) {
	
	uint16_t output;
	uint8_t * tempPtr = (uint8_t *) &output;

	if (idx < 8192) {
		
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudoCount1low) + idx);
		tempPtr++;
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudoCount1high) + idx);
		
	} else {
		
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudoCount2low) + idx - 8192);
		tempPtr++;
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudoCount2high) + idx - 8192);
	}
	
	return output;
}

uint8_t getEqualizationRaw(uint16_t idx) {
	
	uint16_t tempIdx;
	
	if (idx < 8192) {

		return pgm_read_byte(&(equalization1[idx]));
	} else if (idx < (uint16_t) 2*8192) {

		return pgm_read_byte(&(equalization2[idx - (uint16_t) 8192]));
	} else if (idx < (uint16_t) 3*8192) {

		return pgm_read_byte(&(equalization3[idx - (uint16_t) 2*8192]));
	} else if (idx < (uint16_t) 4*8192) {

		return pgm_read_byte(&(equalization4[idx - (uint16_t) 3*8192]));
	} else if (idx < (uint16_t) 5*8192) {

		return pgm_read_byte(&(equalization5[idx - (uint16_t) 4*8192]));
	} else if (idx < (uint16_t) 6*8192) {

		return pgm_read_byte(&(equalization6[idx - (uint16_t) 5*8192]));
	} else if (idx < (uint16_t) 7*8192) {

		return pgm_read_byte(&(equalization7[idx - (uint16_t) 6*8192]));
	} else {
		
		tempIdx = idx - 7*8192;

		if (((uint16_t ) GET_FAR_ADDRESS(equalization8) + tempIdx) >= (uint16_t) 65536)
			return pgm_read_byte_far(GET_FAR_ADDRESS(equalization8) + tempIdx);
		else
			return pgm_read_byte(&(equalization8[tempIdx]));
	}
}

void pwrOnMedipix() {
	
	char timeOutMessage[] = "Timed out";
	
	// clean the buffer before doing anything
	char inChar;
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1)) {}
	
	ioport_set_pin_level(MEDIPIX_PWR, true);
	medipixOnline = 1;
	
	uint8_t timedOut = 1;
	
	uint8_t i = 0;
	
	// prijme uvitaci zpravu
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 8000)) {
		
		// TODO uložit zprávu z medipixu do fram k pozdìjšímu pøeètení

		// ukonci cekani na odpoved pri prijmuti posledniho znaku

		if (inChar == '\r') {
			
			timedOut = 0;
			break;
		}
	
		spi_mem_write_byte(MEDIPIX_BOOTUP_MESSAGE + i++, inChar);
	}
	
	// if the bootup timed out, save it into the message
	if (timedOut == 1) {
		
		for (i = 0; i < strlen(timeOutMessage)+1; i++) {
			
			spi_mem_write_byte(MEDIPIX_BOOTUP_MESSAGE+i, timeOutMessage[i]);
		}
	}
	
	if (medipixCheckStatus() == 0) {
		
		ioport_set_pin_level(MEDIPIX_PWR, false);
		medipixOnline = 0;
	}
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1)) {}
	
	vTaskDelay(40);
}

void setBias(uint8_t bias) {
	
	usartBufferPutByte(medipix_usart_buffer, 'b', 1000);	
	usartBufferPutByte(medipix_usart_buffer, bias, 1000);
	
	// prijme potvrzeni
	char inChar;
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 4000)) {
	
		// TODO uložit zprávu z medipixu do fram k pozdìjšímu pøeètení
		
		// ukonci cekani na odpoved pri prijmuti posledniho znaku
		if (inChar == '\r')
			break;
	}
	
	vTaskDelay(50);
}

void pwrOffMedipix() {
	
	ioport_set_pin_level(MEDIPIX_PWR, false);
	medipixOnline = 0;
}

void openShutter() {
	
	#if PLOT_TEST_PATTERN == 0
	
	usartBufferPutByte(medipix_usart_buffer, 'o', 1000);
	
	// prijme odpoved z open shutter
	char inChar;
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 4000)) {
		
		// TODO uložit zprávu z medipixu do fram k pozdìjšímu pøeètení

		// ukonci cekani na odpoved pri prijmuti posledniho znaku
		if (inChar == '\r')
			break;
	}
	
	#endif
}

void closeShutter() {
	
	#if PLOT_TEST_PATTERN == 0
	
	usartBufferPutByte(medipix_usart_buffer, 'c', 1000);
	
	// prijme odpoved z close shutter
	char inChar;
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 4000)) {
		
		// TODO uložit zprávu z medipixu do fram k pozdìjšímu pøeètení

		// ukonci cekani na odpoved pri prijmuti posledniho znaku
		if (inChar == '\r')
			break;
	}

	vTaskDelay(40);

	#endif
}
	
void eraseMatrix() {
	
	#if PLOT_TEST_PATTERN == 0

	usartBufferPutByte(medipix_usart_buffer, 'e', 1000);
	
	// prijme odpoved z close shutter
	char inChar;
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 4000)) {
		
		// TODO uložit zprávu z medipixu do fram k pozdìjšímu pøeètení

		// ukonci cekani na odpoved pri prijmuti posledniho znaku
		if (inChar == '\r')
			break;
	}
	
	vTaskDelay(40);
	
	#endif
}

uint8_t loadEqualization(uint16_t * data, uint8_t * outputBitStream) {
	
	uint16_t i, j, k;
	
	uint8_t pixelCfg;
	
	uint16_t * Mask;
	
	uint16_t th0, th1, th2, th3, msk, tst;

	usartBufferPutByte(medipix_usart_buffer, 'S', 1000);
	
	vTaskDelay(1);
	
	// dummy byte
	usartBufferPutByte(medipix_usart_buffer, 'd', 1000);
	
	vTaskDelay(10);
	
	for (i = 0; i < 256; i++) {
		
		Mask = &dataBuffer;
		
		for (j = 0; j < 256; j++) {
			
			// load the pixel coniguration from the equalization matrix
			pixelCfg = getEqualizationRaw(256*i + j);
			
			/*
			// vymaskovat roh
			if (((i < 64) && (j < 64)) || ((i > 192) && (j < 64)) || ((i > 192) && (j > 192)) || ((i < 64) && (j > 192))) {
				
				pixelCfg = pixelCfg & (~(0x01));
			}
			*/
			
			// initialize the pixel with testbit preset on 1
			*(Mask+j) = 0;

			msk = (((uint16_t) pixelCfg) & ((uint16_t) 0x01));
			tst = (((uint16_t) pixelCfg) & ((uint16_t) 0x02)) >> 1;
			th0 = (((uint16_t) pixelCfg) & ((uint16_t) 0x04)) >> 2;
			th1 = (((uint16_t) pixelCfg) & ((uint16_t) 0x08)) >> 3;
			th2 = (((uint16_t) pixelCfg) & ((uint16_t) 0x10)) >> 4;
			th3 = (((uint16_t) pixelCfg) & ((uint16_t) 0x20)) >> 5;
		
			// varianta odnejmensiho
			// set the pixels mask
			*(Mask+j) = *(Mask+j) | (msk << 7) | (tst << 13) | (th0 << 8) | (th1 << 12) | (th2 << 10) | (th3 << 11);
					
			// set the pixel mode			
			if (imageParameters.mode == MODE_MEDIPIX)
				*(Mask+j) = (*(Mask+j) & 0x3dbf) | (0 << 6) | (0 << 9);
			else
				*(Mask+j) = (*(Mask+j) & 0x3dbf) | (0 << 6) | (1 << 9);
			
			#if PLOT_TEST_PATTERN == 1
			
				// better test pattern
				if (i == j)
					*(Mask + j) = 255;
				else if (i == 128)
					*(Mask + j) = 128;
				else if (j == 128)
					*(Mask + j) = 64;
				else
					*(Mask + j) = 0;
				
				if (i == 0) {
					*(Mask + j) = j;
				} 
				
				if (i == 255) {
					*(Mask + j) = 255-j;
				}
				
				if (j == 0) {
					*(Mask + j) = i;
				}
				
				if (j == 255) {
					*(Mask + j) = 255-i;
				}
				
			#endif
		}
		
		MpxData2BitStreamSingleMXR(&dataBuffer, &ioBuffer);
						
		for (k = 0; k < 448; k++) {
			
			usartBufferPutByte(medipix_usart_buffer, ioBuffer[k], 1000);	
		}
	}
	
	// send dummy 33 bytes
	for (k = 0; k < 32; k++) {
		usartBufferPutByte(medipix_usart_buffer, 'A', 1000);
	}

	char inChar;
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 2000)) {
	
		// TODO uložit zprávu z medipixu do fram k pozdìjšímu pøeètení

		// ukonci cekani na odpoved pri prijmuti posledniho znaku
		if (inChar == '\r')
			break;
	}
	
	vTaskDelay(40);
	
	return 0;
}

void saveLine(uint8_t row, uint16_t * data) {
	
	uint16_t i;
	uint8_t newPixelValue;
	
	for (i = 0; i < 256; i++) {
		
		if (imageParameters.mode == MODE_TIMEPIX) {

			*(data + i) = *(data + i) / 46;	
		}

		// saturace na byte
		if (*(data + i) > 255) {
				
			newPixelValue = 255;
		} else {
				
			newPixelValue = (uint8_t) (*(data + i));
		}
		
		// erase the last two lines
		#if (ERASE_LAST_TWO_LINE == 1) && (PLOT_TEST_PATTERN == 0)
			if (row >= 254)
				newPixelValue = 0;
		#endif
		
		// count statistic of the original image
		if (newPixelValue > 0) {
			
			imageParameters.nonZeroPixelsOriginal++;
			if (newPixelValue < imageParameters.minValueOriginal)
				imageParameters.minValueOriginal = newPixelValue;
			
			if (newPixelValue > imageParameters.maxValueOriginal)
				imageParameters.maxValueOriginal = newPixelValue;
		}
		
		setRawPixel(row, (uint8_t) i, newPixelValue);
	}
}

void readMatrix() {
	
	char inChar;
		
	uint16_t bytesInIoBuffer = 0;
	uint16_t bytesInTempBuffer = 0;
	uint16_t bytesReceived = 1;
	uint16_t rowsReceived = 0;
	uint8_t bufferFull = 0;
	
	unsigned long totalBytesReceived = 0;
	
	memset(ioBuffer, 0, 448);
	memset(tempBuffer, 0, 256);
	memset(dataBuffer, 0, 512);
	
	imageParameters.nonZeroPixelsOriginal = 0;
	imageParameters.minValueOriginal = 255;
	imageParameters.maxValueOriginal = 0;
	
	usartBufferPutByte(medipix_usart_buffer, 'm', 1000);
	
	// receive the response from command
	usartBufferGetByte(medipix_usart_buffer, &inChar, 4000);
	
	// receive the dummy byte
	usartBufferGetByte(medipix_usart_buffer, &inChar, 4000);
		
	while (true) {
	
		// I should receive some data from medipix
		if (bytesReceived < 256) {
			
			usartBufferGetByte(medipix_usart_buffer, &inChar, 5000);
			bytesReceived++;
			totalBytesReceived++;
		}
		
		// when the IO buffer is not full, put it there
		if (bufferFull == 0) {
			
			ioBuffer[bytesInIoBuffer] = inChar;
			bytesInIoBuffer++;
		// otherwise put in into the tempBuffer
		} else {
			
			tempBuffer[bytesInTempBuffer] = inChar;
			bytesInTempBuffer++;
		}
		
		// if there are 448 bytes in the IO buffer, make it full
		if (bytesInIoBuffer == 448) {
			
			bufferFull = 1;
		}
		
		// we received the block of 256 bytes from the medipix
		if (bytesReceived == 256) {
			
			// if the buffer is full, we can process one line of the image
			if (bufferFull == 1) {
				
				// deserialize the line
				MpxBitStream2DataSingleMXR(&ioBuffer, &dataBuffer);
			
				#if PLOT_TEST_PATTERN == 0
				// derandomize the line
				MpxConvertValuesMXR(&dataBuffer);
				#endif
			
				// save the line to fram
				saveLine(rowsReceived, (uint16_t *) &dataBuffer);
				
				// move the data from tempBuffer to IO buffer
				memcpy(ioBuffer, tempBuffer, bytesInTempBuffer*sizeof(uint8_t));
				
				// set the correct number of data in the IO buffer
				bytesInIoBuffer = bytesInTempBuffer;
				
				// reset the number of bytes in temp buffer
				bytesInTempBuffer = 0;
				
				// increment the number of received rows
				rowsReceived++;
					
				// the IO buffer should not be full now
				bufferFull = 0;
			}
			
			bytesReceived = 0;
			
			if (rowsReceived < 256)
				usartBufferPutByte(medipix_usart_buffer, 'i', 1000);
		}
		
		// special case, the last line
		if ((rowsReceived == 255) && (bufferFull == 1) && (bytesReceived == 36)) {
			
			// deserialize the line
			MpxBitStream2DataSingleMXR(&ioBuffer, &dataBuffer);
			
			#if PLOT_TEST_PATTERN == 0
			// derandomize the line
			MpxConvertValuesMXR(&dataBuffer);
			#endif

			// save the line to fram
			saveLine(rowsReceived, (uint16_t *) &dataBuffer);
			
			break;
		}
	}
}