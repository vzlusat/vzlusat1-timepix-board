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
uint8_t getBinnedPixel(uint8_t row, uint8_t col, uint8_t outputForm) {
	
	unsigned long address = WORKING_SPACE_START_ADDRESS;
	
	switch (outputForm) {
		
		case BINNING_8:
			address += ((unsigned long) row)*32 + ((unsigned long) col);
		break;
		
		case BINNING_16:
			address += ((unsigned long) row)*16 + ((unsigned long) col);
		break;
		
		case BINNING_32:
			address += ((unsigned long) row)*8 + ((unsigned long) col);
		break;
	}
	
	return spi_mem_read_byte(address);
}

// set the value in the pixel of the downscaled image
void setBinnedPixel(uint8_t row, uint8_t col, uint8_t value, uint8_t outputForm) {
	
	unsigned long address = WORKING_SPACE_START_ADDRESS;
	
	switch (outputForm) {
		
		case BINNING_8:
			address += ((unsigned long) row)*32 + ((unsigned long) col);
		break;
		
		case BINNING_16:
			address += ((unsigned long) row)*16 + ((unsigned long) col);
		break;
		
		case BINNING_32:
			address += ((unsigned long) row)*8 + ((unsigned long) col);
		break;
	}
	
	spi_mem_write_byte(address, value);
}

// set histogram1 value
void setHistogram1(uint8_t idx, uint8_t value) {
	
	unsigned long address = WORKING_SPACE_START_ADDRESS+idx;
	
	spi_mem_write_byte(address, value);
}

// set histogram2 value
void setHistogram2(uint8_t idx, uint8_t value) {
	
	unsigned long address = WORKING_SPACE_START_ADDRESS+256+idx;
	
	spi_mem_write_byte(address, value);
}

// get histogram1 value
uint8_t getHistogram1(uint8_t idx) {
	
	unsigned long address = WORKING_SPACE_START_ADDRESS+idx;
	
	return spi_mem_read_byte(address);
}

// set energy histogram value
void setEnergyHistogram(uint8_t idx, uint16_t value) {
	
	unsigned long address = ENERGY_HISTOGRAM_ADRESS + 2*idx;
	
	spi_mem_write_uint16(address, value);
}

// get value of the energy histogram
uint16_t getEnergyHistogram(uint8_t idx) {
	
	unsigned long address = ENERGY_HISTOGRAM_ADRESS + 2*idx;
	
	return spi_mem_read_uint16t(address);
}

// get histogram2 value
uint8_t getHistogram2(uint8_t idx) {
	
	unsigned long address = WORKING_SPACE_START_ADDRESS+256+idx;
	
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
	
	spi_mem_read_blob(IMAGE_PARAMETERS_ADDRESS, (uint8_t *) (&imageParameters), sizeof(imageParameters_t));	
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
	}
}

// count number of active pixel in the filtered/output image
void computeImageStatistics() {
	
	uint16_t i, j;
	
	uint8_t tempPixel;
	
	imageParameters.nonZeroPixelsFiltered = 0;
	imageParameters.minValueFiltered = 255;
	imageParameters.maxValueFiltered = 0;
	
	for (i = 0; i < 256; i++) {
		
		for (j = 0; j < 256; j++) {
			
			tempPixel = getFilteredPixel(i, j);
			
			if (tempPixel > 0) {
				imageParameters.nonZeroPixelsFiltered++;
			
				if (tempPixel < imageParameters.minValueFiltered)
					imageParameters.minValueFiltered = tempPixel;
				
				if (tempPixel > imageParameters.maxValueFiltered)
					imageParameters.maxValueFiltered = tempPixel;
			}
		}
	}
}

// apply binning
void applyBinning(uint8_t outputForm) {
	
	uint16_t i, j, x, y, sum;
	
	uint8_t numPerLine = 0;
	uint8_t numInBin = 0;
	uint8_t tempPixel;
	
	switch (outputForm) {
		
		case BINNING_8:
			numPerLine = 32;
			numInBin = 8;
		break;
		
		case BINNING_16:
			numPerLine = 16;
			numInBin = 16;
		break;
		
		case BINNING_32:
			numPerLine = 8;
			numInBin = 32;
		break;
	}

	if (outputForm > BINNING_1) {
		
		// go through the binned image
		for (i = 0; i < numPerLine; i++) {
		
			for (j = 0; j < numPerLine; j++) {
			
				sum = 0;
			
				// go through all subpixels of the bin
				for (x = i*numInBin; x < (i+1)*numInBin; x++) {
				
					for (y = j*numInBin; y < (j+1)*numInBin; y++) {
					
						tempPixel = getFilteredPixel(x, y);
					
						if (tempPixel > 0) {
							
							sum++;
						}
					}
				}
				
				// pokud binujeme po 32, tak downscale
				if (outputForm == BINNING_32) {
					
					sum = (uint16_t) ceil(((float) sum / (float) 4));
				}
				
				if (sum >= 256)
					sum = 255;
			
				// create the averege
				// sum = sum / (numPerLine*numPerLine);
			
				// print the test pattern
				// setBinnedPixel(i, j, i*numPerLine + j);
			
				setBinnedPixel(i, j, (uint8_t) sum, outputForm);
			}
		}
	}
}

// create histograms from the image
void createHistograms() {
	
	uint16_t i, j, sum;
	
	for (i = 0; i < 256; i++) {
		
		// create historgram 1
		sum = 0;
		
		for (j = 0; j < 256; j++) {
			
			if (getFilteredPixel(i, j) > 0)
				sum++;
		}
		
		if (sum > 255)
			sum = 255;
		
		setHistogram1(i, (uint8_t) sum);
		
		// create histogram 2
		sum = 0;
		
		for (j = 0; j < 256; j++) {
			
			if (getFilteredPixel(j, i) > 0)
				sum++;
		}
		
		if (sum > 255)
			sum = 255;
		
		setHistogram2(i, (uint8_t) sum);
	}
}

// create the energy histogram
void createEnergyHistogram() {
	
	uint16_t i, j, sum;
	
	uint8_t pixelValue;
	
	// clear the memory at first
	for (i = 0; i < 16; i++)
		setEnergyHistogram(i, 0);
	
	// compute the histogram
	for (i = 0; i < 256; i++) {
		
		for (j = 0; j < 256; j++) {
			
			pixelValue = getFilteredPixel(i, j);
			
			if ((pixelValue > 0) && (pixelValue < 128)) {
				
				setEnergyHistogram(floor(pixelValue/8.0), getEnergyHistogram(floor(pixelValue/8.0))+1);	
			} else {
				
				setEnergyHistogram(15, getEnergyHistogram(15)+1);
			}
		}
	}
}