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
	unsigned long address;
	address = ((unsigned long) row)*256 + ((unsigned long) col) + RAW_IMAGE_START_ADDRESS;
	
	// write the data
	spi_mem_write_byte(address, value);
}

// set the value of the pixel in the filtered image
void setFilteredPixel(uint8_t row, uint8_t col, uint8_t value) {
	
	// prepare the adress of the pixel
	unsigned long address;
	address = ((unsigned long) row)*256 + ((unsigned long) col) + FILTERED_IMAGE_START_ADDRESS;
	
	// write the data
	spi_mem_write_byte(address, value);
}

// get the value of the pixel in the filtered image
uint8_t getRawPixel(uint8_t row, uint8_t col) {
	
	// prepare the adress of the pixel
	unsigned long address;
	address = ((unsigned long) row)*256 + ((unsigned long) col) + RAW_IMAGE_START_ADDRESS;
	
	return spi_mem_read_byte(address);
}

// get the value of the pixel in the filtered image
uint8_t getFiltered(uint8_t row, uint8_t col) {
	
	// prepare the adress of the pixel
	unsigned long address;
	address = ((unsigned long) row)*256 + ((unsigned long) col) + FILTERED_IMAGE_START_ADDRESS;
	
	// get the value
	return spi_mem_read_byte(address);
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
		
	if (*(previousRow + col) == 1)
		return 1;
		
	return 0;
}

uint8_t hasNeighbour(uint8_t col, uint8_t * currentRow, uint8_t * previousRow) {
	
	if (hasNeighboutLeft(col, currentRow) || hasNeighbourRight(col, currentRow) || hasNeighbourUp(col, currentRow, previousRow))
		return 1;
			
	return 0;
}

// copy the raw image into the filtered image
// remove the non-one pixel events
void filterOnePixelEvents() {
	
	uint16_t row, col;
	uint8_t tempPxl;
	
	imageParameters.nonZeroPixels = 0;
	imageParameters.minValue = 255;
	imageParameters.maxValue = 0;
	
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
				
				// if the value is nonzero
				if (tempPxl > 0) {
					
					imageParameters.nonZeroPixels++;
					
					// set the minValue
					if (tempPxl < imageParameters.minValue)
						imageParameters.minValue = tempPxl;
						
					// set the maxValue
					if (tempPxl > imageParameters.maxValue)
						imageParameters.maxValue = tempPxl;
				}
			}
		}
		
		// if the image is clean, set the min and max value to 0
		if (imageParameters.nonZeroPixels == 0) {
			
			imageParameters.minValue = 0;
			imageParameters.maxValue = 0;
		}
		
		spi_mem_write_blob(IMAGE_PARAMETERS_ADDRESS, (uint8_t *) (&imageParameters), sizeof(imageParameters_t));
		
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
			
			// delete the pixel in the currentRow that have a neighbour
			for (col = 0; col < 256; col++) {
				
				// if the pixel has a neighbour, delete it from the filtered image
				if (hasNeighbour(col, currentRow, previousRow)) {
					
					setFilteredPixel(row, col, 0);
				}
			}
			
			// after the business, swap the current row and the previous row
			tempPtr = previousRow;
			previousRow = currentRow;
			currentRow = tempPtr;
		}
	}
}

// apply binning
void applyBinning() {
	
	
}

// create histograms from the image
void createHistograms() {
	
	
}

void prepareOutput() {
	
	
}