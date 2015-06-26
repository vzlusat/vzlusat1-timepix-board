/*
 * imageProcessing.c
 *
 *  Author: klaxalk
 */ 

#include "imageProcessing.h"
#include "medipix.h"
#include "fram_mapping.h"
#include "spi_memory_FM25.h"

uint8_t * buffer256 = (uint8_t *) &tempBuffer;
uint8_t * buffer448 = (uint8_t *) &ioBuffer;
uint8_t * buffer512 = (uint8_t *) &dataBuffer;

// increase the image ID and return the new value
uint16_t increaseImageID() {
	
	uint16_t tempInt;
	
	tempInt = spi_mem_read_uint16t(IMAGE_ID_ADDRESS);
	tempInt++;
	
	spi_mem_write_uint16(IMAGE_ID_ADDRESS, tempInt);
	
	return tempInt;
}

// return the image ID value
uint16_t getImageID() {
	
	return spi_mem_read_uint16t(IMAGE_ID_ADDRESS);
}

// set the value of the pixel in the raw image
void setRawPixel(uint8_t row, uint8_t col, uint8_t value) {
	
	// prepare the adress of the pixel
	unsigned long address = RAW_IMAGE_START_ADDRESS;
	address += ((unsigned long) row)*256 + ((unsigned long) col);
	
	// write the data
	spi_mem_write_byte(address, value);
}

// set the value of the pixel in the filtered image
void setFilteredPixel(uint8_t row, uint8_t col, uint8_t value) {
	
	// prepare the adress of the pixel
	unsigned long address = FILTERED_IMAGE_START_ADDRESS;
	address += ((unsigned long) row)*256 + ((unsigned long) col);
	
	// write the data
	spi_mem_write_byte(address, value);
}

// get the value of the pixel in the filtered image
uint8_t getRawPixel(uint8_t row, uint8_t col) {
	
	// prepare the adress of the pixel
	unsigned long address = RAW_IMAGE_START_ADDRESS;
	address += ((unsigned long) row)*256 + ((unsigned long) col);
	
	return spi_mem_read_byte(address);
}

// get the value of the pixel in the filtered image
uint8_t getFilteredPixel(uint8_t row, uint8_t col) {
	
	// prepare the adress of the pixel
	unsigned long address = FILTERED_IMAGE_START_ADDRESS;
	address += ((unsigned long) row)*256 + ((unsigned long) col);
	
	// get the value
	return spi_mem_read_byte(address);
}

// get the value of the pixel in the downscaled image
uint8_t getBinnedPixel(uint8_t row, uint8_t col) {
	
	unsigned long address = WORKING_SPACE_START_ADDRESS;
	
	switch (imageParameters.outputForm) {
		
		case BINNING_8:
			address += ((unsigned long) row)*8 + ((unsigned long) col);
		break;
		
		case BINNING_16:
			address += ((unsigned long) row)*16 + ((unsigned long) col);
		break;
		
		case BINNING_32:
			address += ((unsigned long) row)*32 + ((unsigned long) col);
		break;
	}
	
	return spi_mem_read_byte(address);
}

// set the value in the pixel of the downscaled image
uint8_t setBinnedPixel(uint8_t row, uint8_t col, uint8_t value) {
	
	unsigned long address = WORKING_SPACE_START_ADDRESS;
	
	switch (imageParameters.outputForm) {
		
		case BINNING_8:
			address += ((unsigned long) row)*8 + ((unsigned long) col);
		break;
		
		case BINNING_16:
			address += ((unsigned long) row)*16 + ((unsigned long) col);
		break;
		
		case BINNING_32:
			address += ((unsigned long) row)*32 + ((unsigned long) col);
		break;
	}
	
	spi_mem_write_byte(address, value);
}

// returns 1 if the pixel is active and has a nonzero left neighbour
uint8_t hasNeighboutLeft(uint8_t col, uint8_t * row) {
	
	if (col == 0)
		return 0;

	if (*(row + col) == 0)
		return 0;

	if (*(row + col - 1) > 0)
		return 1;
	
	return 0;
}

// returns 1 if the pixel is active and has a nonzero right neighbour
uint8_t hasNeighbourRight(uint8_t col, uint8_t * row) {
	
	if (col == 255)
		return 0;

	if (*(row + col) == 0)
		return 0;

	if (*(row + col + 1) > 0)
		return 1;
	
	return 0;
}

// returns 1 if the pixel is active and has a nonzero up neighbour
uint8_t hasNeighbourUp(uint8_t col, uint8_t * currentRow, uint8_t * previousRow) {
	
	if (*(currentRow + col) == 0)
		return 0;
		
	if (*(previousRow + col) > 0)
		return 1;
		
	return 0;
}

uint8_t hasNeighbour(uint8_t col, uint8_t * currentRow, uint8_t * previousRow) {
	
	if (hasNeighboutLeft(col, currentRow) || hasNeighbourRight(col, currentRow) || hasNeighbourUp(col, currentRow, previousRow))
		return 1;
			
	return 0;
}

void loadImageParametersFromFram() {
	
	spi_mem_write_blob(IMAGE_PARAMETERS_ADDRESS, (uint8_t *) &imageParameters, sizeof(imageParameters_t));	
}

void saveImageParametersToFram() {
	
	spi_mem_write_blob(IMAGE_PARAMETERS_ADDRESS, (uint8_t *) (&imageParameters), sizeof(imageParameters_t));
}

// copy the raw image into the filtered image
// remove the non-one pixel events
void filterOnePixelEvents() {
	
	uint16_t row, col;
	uint8_t tempPxl;
	
	imageParameters.nonZeroPixelsFiltered = 0;
	imageParameters.minValueFiltered = 255;
	imageParameters.maxValueFiltered = 0;
	
	// no filtering, just copy the matrix and count the number of events
	if (imageParameters.filtering == 0) {
		
		// for all rows in the image
		for (row = 0; row < 256; row++) {
			
			// for all columns in the image
			for (col = 0; col < 256; col++) {
				
				// get the pixel from the raw image
				tempPxl = getRawPixel(row, col);
				
				// set it in the "filtered image"
				setFilteredPixel(row, col, tempPxl);
			}
		}
		
		saveImageParametersToFram();
		
	// filtering on, copy, filter and count the number of events
	} else if (imageParameters.filtering == 1) {
		
		uint8_t * currentRow = buffer256;
		uint8_t * previousRow = buffer448;
		uint8_t * tempPtr;
		
		memset(currentRow, 0, 256*sizeof(uint8_t));
		memset(previousRow, 0, 256*sizeof(uint8_t));
		
		// for all rows
		for (row = 0; row < 256; row++) {

			// get the current row to RAM
			// and copy the current row to the "filtered image" unchanged
			for (col = 0; col < 256; col++) {
				
				*(currentRow + col) = getRawPixel(row, col);
				
				setFilteredPixel(row, col, *(currentRow + col));
			}
			
			// delete the pixel in the currentRow that has a neighbour
			for (col = 0; col < 256; col++) {
				
				// if the pixel has a neighbour, delete it from the filtered image
				if (hasNeighbour(col, currentRow, previousRow)) {
					
					// if the neighbour is above, delete it
					if (hasNeighbourUp(col, currentRow, previousRow)) {
						
						setFilteredPixel(row-1, col, 0);
					}
					
					setFilteredPixel(row, col, 0);
				}
			}
			
			// after the business, swap the current row and the previous row
			tempPtr = previousRow;
			previousRow = currentRow;
			currentRow = tempPtr;
		}
		
		saveImageParametersToFram();
	}
}

// count number of active pixel in the filtered/output image
void computeImageStatistics() {
	
	uint16_t i, j, numPerLine;
	
	uint8_t tempPixel;
	
	loadImageParametersFromFram();
	
	imageParameters.nonZeroPixelsFiltered = 0;
	imageParameters.minValueOriginal = 256;
	
	uint8_t (*fce)(uint8_t, uint8_t);
	
	switch(imageParameters.outputForm) {
	
		case BINNING_1:
			numPerLine = 256;
			fce = &getFilteredPixel;
		break;
		
		case BINNING_8:
			numPerLine = 32;
			fce = &getBinnedPixel;
		break;
		
		case BINNING_16:
			numPerLine = 16;
			fce = &getBinnedPixel;
		break;
		
		case BINNING_32:
			numPerLine = 8;
			fce = &getBinnedPixel;
		break;
	}
	
	for (i = 0; i < numPerLine; i++) {
		
		for (j = 0; j < numPerLine; j++) {
			
			tempPixel = (*fce)(i, j);
			
			if (tempPixel > 0) {
				imageParameters.nonZeroPixelsFiltered++;
			
				if (tempPixel < imageParameters.minValueFiltered)
					imageParameters.minValueFiltered = tempPixel;
				
				if (tempPixel > imageParameters.maxValueFiltered)
					imageParameters.maxValueFiltered = tempPixel;
			}
		}
	}
	
	saveImageParametersToFram();
}

// apply binning
void applyBinning() {
	
	
}

// create histograms from the image
void createHistograms() {
	
	
}

void prepareOutput() {
	
	
}