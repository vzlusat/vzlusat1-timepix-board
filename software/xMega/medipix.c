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

// 1 if medipix is powered
// 0 if medipix is turned off
uint8_t medipixOnline = 0;

uint16_t DefaultDacValsTimepix[15] = {1, 100, 255, 127, 127, 0, 314, 7, 130, 128, 80, 85, 128, 128, 0};

volatile Mpx_DAC DAC;

volatile uint8_t medipixMode = MODE_MEDIPIX; 

volatile uint8_t ioBuffer[448];
volatile uint8_t tempBuffer[256];
volatile uint16_t dataBuffer[256];

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

	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
		
		#if DEBUG_OUTPUT == 1
		if (inChar == '\r')
		inChar = '<';
		
		if (inChar == '\n')
		inChar = '_';
		
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
		#endif
	}
	
	stream[0]='d';
	
	// Pak naplnit
	MpxDACstreamTimepix((uint8_t *) stream+1);
		
	// odeslat skuteèné DAC
	for (i = 0; i < 34; i++) {
		
		#if DEBUG_OUTPUT == 1
		bin2hex(stream[i], &outcomingPacket->data);
		outcomingPacket->data[2] = ' ';
		outcomingPacket->data[3] = 0;
		outcomingPacket->length = 4;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		#endif
		
		usartBufferPutByte(medipix_usart_buffer, stream[i], 1000);
		vTaskDelay(5);
	}
	
	#if DEBUG_OUTPUT == 1
	sendBlankLine(15, 16);
	#endif
		
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
		
		#if DEBUG_OUTPUT == 1
		if (inChar == '\r')
		inChar = '<';
		
		if (inChar == '\n')
		inChar = '_';
		
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
		#endif
	}
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
void MpxConvertValuesMXR(uint16_t * values){

	uint16_t i;
	
	for (i = 0; i < 256; i++) {
		
		if (*values < 16384) {
			
			*values = getRntRaw(*values);
				
		} else {
			*values = 0xFFFF;
		}
		values++;
	}
}

uint16_t getRntRaw(uint16_t idx) {
	
	uint16_t output;
	uint8_t * tempPtr = (uint8_t *) &output;
	
#if MEDIPIX_VERSION == FLIGHT
	
	if (idx < 8192) {
		
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudoCount1low) + idx);
		tempPtr++;
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudoCount1high) + idx);
		
	} else {
		
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudoCount2low) + idx - 8192);
		tempPtr++;
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudoCount2high) + idx - 8192);
	}
	
#else

	if (idx < 8192) {
		
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudo2Count1low) + idx);
		tempPtr++;
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudo2Count1high) + idx);
		
	} else {
		
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudo2Count2low) + idx - 8192);
		tempPtr++;
		*tempPtr = pgm_read_byte_far(GET_FAR_ADDRESS(pseudo2Count2high) + idx - 8192);
	}

#endif
	
	return output;
}

uint8_t getEqualizationRaw(uint16_t idx) {
	
#if MEDIPIX_VERSION == FLIGHT

	if (idx < 8192) {

		return pgm_read_byte(&(equalization1[idx]));
	} else if (idx < 2*8192) {

		return pgm_read_byte(&(equalization2[idx - 8192]));
	} else if (idx < 3*8192) {

		return pgm_read_byte(&(equalization3[idx - 2*8192]));
	} else if (idx < 4*8192) {

		return pgm_read_byte(&(equalization4[idx - 3*8192]));
	} else if (idx < 5*8192) {

		return pgm_read_byte(&(equalization5[idx - 4*8192]));
	} else if (idx < 6*8192) {

		return pgm_read_byte(&(equalization6[idx - 5*8192]));
	} else if (idx < 7*8192) {

		return pgm_read_byte(&(equalization7[idx - 6*8192]));
	} else {

		return pgm_read_byte(&(equalization8[idx - 7*8192]));
	}
		
#elif MEDIPIX_VERSION == EQM
	
	if (idx < 8192) {

		return pgm_read_byte(&(equalization21[idx]));
	} else if (idx < 2*8192) {

		return pgm_read_byte(&(equalization22[idx - 8192]));
	} else if (idx < 3*8192) {

		return pgm_read_byte(&(equalization23[idx - 2*8192]));
	} else if (idx < 4*8192) {

		return pgm_read_byte(&(equalization24[idx - 3*8192]));
	} else if (idx < 5*8192) {

		return pgm_read_byte(&(equalization25[idx - 4*8192]));
	} else if (idx < 6*8192) {

		return pgm_read_byte(&(equalization26[idx - 5*8192]));
	} else if (idx < 7*8192) {

		return pgm_read_byte(&(equalization27[idx - 6*8192]));
	} else {

		return pgm_read_byte(&(equalization28[idx - 7*8192]));
	}

#endif

}

void pwrOnMedipix() {
	
	ioport_set_pin_level(MEDIPIX_PWR, true);
	medipixOnline = 1;
	
	// pocka az nastartuje

	#if DEBUG_OUTPUT == 1
	sendBlankLine(15, 16);
	#endif
	
	// prijme uvitaci zpravu
	char inChar;
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 4000)) {
	
		#if DEBUG_OUTPUT == 1
		
		if (inChar == '\r')
		inChar = '<';
	
		if (inChar == '\n')
		inChar = '_';
	
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
		
		#endif
	}
}

void setBias(uint8_t bias) {
	
	usartBufferPutByte(medipix_usart_buffer, 'b', 1000);
	usartBufferPutByte(medipix_usart_buffer, bias, 1000);
	
	#if DEBUG_OUTPUT == 1
	sendBlankLine(15, 16);
	#endif
	
	// prijme uvitaci zpravu
	char inChar;
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 500)) {
		
		#if DEBUG_OUTPUT == 1
		
		if (inChar == '\r')
		inChar = '<';
		
		if (inChar == '\n')
		inChar = '_';
		
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
		
		#endif
	}
}

void pwrOffMedipix() {
	
	ioport_set_pin_level(MEDIPIX_PWR, false);
	medipixOnline = 0;
	sendString("Medipix OFF\r\n");
}

void pwrToggleMedipix() {
	
	if (medipixOnline == 1) {
		
		pwrOffMedipix();
		
	} else {
		
		pwrOnMedipix();	
	}
}

void openShutter() {
	
	usartBufferPutByte(medipix_usart_buffer, 'o', 1000);
	
	#if DEBUG_OUTPUT == 1
	sendBlankLine(15, 16);
	#endif
	
	// prijme uvitaci zpravu
	char inChar;
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 500)) {
		
		#if DEBUG_OUTPUT == 1
		if (inChar == '\r')
		inChar = '<';
		
		if (inChar == '\n')
		inChar = '_';
		
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
		#endif
	}
}

void closeShutter() {
	
	usartBufferPutByte(medipix_usart_buffer, 'c', 1000);
	
	#if DEBUG_OUTPUT == 1
	sendBlankLine(15, 16);
	#endif
	
	// prijme uvitaci zpravu
	char inChar;
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 500)) {
		
		#if DEBUG_OUTPUT == 1
		if (inChar == '\r')
		inChar = '<';
		
		if (inChar == '\n')
		inChar = '_';
		
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
		#endif
	}
}

void eraseMatrix() {
	
	char inChar;
	
	int16_t actIncomingPosition = 0;
	
	char measuringProceeding = 0;
	
	char temp[50];
	
	int16_t rowByteIdx = 0;
	
	int16_t bytesInBuffer = 0;
	int16_t bytesInOverBuffer = 0;
	uint8_t bufferFull = 0;
	int16_t receivedBytes = 0;
	uint16_t rowsReceived = 0;
	int16_t i;
	
	memset(ioBuffer, 0, 448);
	memset(tempBuffer, 0, 256);
	memset(dataBuffer, 0, 512);

	#if DEBUG_OUTPUT == 1
	sendBlankLine(15, 16);
	#endif
	
	usartBufferPutByte(medipix_usart_buffer, 'm', 1000);
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 4000)) {break;}
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
		
		receivedBytes++; // poèet bajtù pøijatých "tento øádek"
		
		if (bufferFull == 0) { // pokud nemáme plný ioBuffer
			
			bytesInBuffer++; // pøidá pøijatý bajt do ioBuffer
			
			if (bytesInBuffer == 448) // pokud jsme tímto znakem naplnili buffer
				bufferFull = 1;
			
		} else {
			bytesInOverBuffer++;
		}
		
		// pokud došel celý datagram ale nemám naplnìný buffer, zažádám o další
		if (receivedBytes == 256 && bufferFull == 0) {
			
			receivedBytes = 0;
			usartBufferPutByte(medipix_usart_buffer, 'i', 1000);
		}

		// receive the dummy line
		if (rowsReceived == 256) {
			
			#if DEBUG_OUTPUT == 1
			
			outcomingPacket->data[0] = inChar;
			outcomingPacket->data[1] = 0;
			outcomingPacket->length = 2;
			csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
			vTaskDelay(5);
			
			#endif
		}
		
		// pokud mám plný buffer a pøišel další datagram, zpracuju øádek
		if (receivedBytes == 256 & bufferFull == 1) {
		
			bytesInBuffer = bytesInOverBuffer;
			receivedBytes = 0;
			bytesInOverBuffer = 0;
			rowsReceived++;
			bufferFull = 0;
			
			usartBufferPutByte(medipix_usart_buffer, 'i', 1000);
		}
		
	}
}

uint8_t loadEqualization(uint16_t * data, uint8_t * outputBitStream) {
	
	uint16_t i, j;
	uint16_t k;
	PixelCfg val;
	
	uint8_t * valPoint = (uint8_t *) &val;
	
	uint16_t * Mask;

	#if DEBUG_OUTPUT == 1
	sendBlankLine(15, 16);
	#endif
	
	usartBufferPutByte(medipix_usart_buffer, 'S', 1000);
	
	vTaskDelay(10);
	
	for (i = 0; i < 256; i++) {
		
		Mask = dataBuffer;
		
		for (j = 0; j < 256; j++) {
			
			*valPoint = getEqualizationRaw(256*i + j);
			
			*(Mask+j) = val.maskBit | ((!val.testBit) << 9) | ((val.lowTh  & 0x01) << 7) | ((val.lowTh & 0x02) << 5) | ((val.lowTh  & 0x04) << 6) | ((val.highTh  & 0x01) << 12) | ((val.highTh  & 0x02) << 9) | ((val.highTh  & 0x04) << 9);
		
			// set the pixel mode			
			*(Mask+j) = (*(Mask+j) & 0x3dbf) | (medipixMode << 6) | (medipixMode << 9);
			
			/*
			// save test pattern
			if ((i % 2) == 0)
				*(Mask+j) = j;
			else
				*(Mask+j) = 255-j;
			*/
		}
		
		MpxData2BitStreamSingleMXR(&dataBuffer, &ioBuffer);
		
		#if DEBUG_OUTPUT == 1
		
		outcomingPacket->data[0] = 'X';
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		
		#endif
						
		for (k = 0; k < 448; k++) {
			
			usartBufferPutByte(medipix_usart_buffer, ioBuffer[k], 1000);	
		}
	}
	
	// send dummy 33 bytes
	for (k = 0; k < 33; k++) {
		usartBufferPutByte(medipix_usart_buffer, 'A', 1000);
	}
	
	char inChar;
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
		
		#if DEBUG_OUTPUT == 1
		
		if (inChar == '\r')
		inChar = '<';
		
		if (inChar == '\n')
		inChar = '_';
		
		outcomingPacket->data[0] = inChar;
		outcomingPacket->data[1] = 0;
		outcomingPacket->length = 2;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(20);
		
		#endif
	}
	
	return 0;
}

void proceedeLine(uint16_t * data) {
	
	uint8_t pCounter;
	uint8_t i;
	
	// vytvoøí 8 packetù z celého øádku
	for (pCounter = 0; pCounter < 8; pCounter++) {
		
		for (i = 0; i < 32; i++) {
			
			if (medipixMode == MODE_TIMEPIX) {

				*(data + ((pCounter*32) + i)) = *(data + ((pCounter*32) + i)) / 256;
			}

			// saturace na byte
			if (*(data + ((pCounter*32) + i)) > 255) {
	
				outcomingPacket->data[i] = 255;
			} else {
	
				outcomingPacket->data[i] = (uint8_t) (*(data + ((pCounter*32) + i)));
			}
		}
		
		outcomingPacket->data[32] = '\r';
		outcomingPacket->data[33] = '\n';
		outcomingPacket->length = 34;
		csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
		vTaskDelay(7);
	}
}

void readMatrix() {
	
	char inChar;
		
	int16_t actIncomingPosition = 0;
	
	char measuringProceeding = 0;
	
	char temp[50];
	
	int16_t rowByteIdx = 0;
		
	int16_t bytesInBuffer = 0;
	int16_t bytesInOverBuffer = 0;
	uint8_t bufferFull = 0;
	int16_t receivedBytes = 0;
	uint16_t rowsReceived = 0;
	int16_t i;
	
	memset(ioBuffer, 0, 448);
	memset(tempBuffer, 0, 256);
	memset(dataBuffer, 0, 512);

	#if DEBUG_OUTPUT == 1
	sendBlankLine(15, 16);
	#endif
	
	usartBufferPutByte(medipix_usart_buffer, 'm', 1000);
	
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 4000)) {break;}
			
	while (usartBufferGetByte(medipix_usart_buffer, &inChar, 1000)) {
				
		receivedBytes++; // poèet bajtù pøijatých "tento øádek"
			 
		if (bufferFull == 0) { // pokud nemáme plný ioBuffer
			
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
			
			#if DEBUG_OUTPUT == 1
				outcomingPacket->data[0] = inChar;
				outcomingPacket->data[1] = 0;
				outcomingPacket->length = 2;
				csp_sendto(CSP_PRIO_NORM, 1, 15, 16, CSP_O_NONE, outcomingPacket, 1000);
				vTaskDelay(5);
			#endif
		}
		
		// pokud mám plný buffer a pøišel další datagram, zpracuju øádek
		if (receivedBytes == 256 & bufferFull == 1) {
			
			MpxBitStream2DataSingleMXR(&ioBuffer, &dataBuffer);
			
			MpxConvertValuesMXR(&dataBuffer);
			
			// pøemístí ukazovátko na správné místo v tempBufferu
			
			#if DEBUG_OUTPUT == 1
			// výpis
			sprintf(temp, "R%d: %d %d %d %d %d %d\n\r", rowsReceived, dataBuffer[0], dataBuffer[1], dataBuffer[2], dataBuffer[3], dataBuffer[4], dataBuffer[5]);
			strcpy(outcomingPacket->data, temp);
			outcomingPacket->length = strlen(temp);
			csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);

			sprintf(temp, "\n\r");
			strcpy(outcomingPacket->data, temp);
			outcomingPacket->length = strlen(temp);
			csp_sendto(CSP_PRIO_NORM, 1, dest_p, source_p, CSP_O_NONE, outcomingPacket, 1000);
			#endif
			
			/* 
			// plot out test pattern
			int q;
			for (q = 0; q < 256; q++) {
				
				if ((rowsReceived % 2) == 0)
					dataBuffer[q] = q;
				else
					dataBuffer[q] = 255-q;
			}
			*/
			
			#if MATLAB_OUTPUT
			proceedeLine(&dataBuffer);
			#endif
			
			// skompíruje overBuffer do ioBufferu
			memcpy(ioBuffer, tempBuffer, bytesInOverBuffer*sizeof(uint8_t));
		
			bytesInBuffer = bytesInOverBuffer;
			receivedBytes = 0;
			bytesInOverBuffer = 0;
			rowsReceived++;
			bufferFull = 0;
			
			usartBufferPutByte(medipix_usart_buffer, 'i', 1000);
		}
		
	}
}