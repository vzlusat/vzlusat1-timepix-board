/*
 * imageProcessing.h
 *
 *  Author: klaxalk
 */ 

#ifndef IMAGEPROCESSING_H_
#define IMAGEPROCESSING_H_

#include "system.h"

// increase the image ID and return the new value
uint16_t increaseImageID();

// return the image ID value
uint16_t getImageID();

// set the value of the pixel in the raw image
void setRawPixel(uint8_t row, uint8_t col, uint8_t value);

// set the value of the pixel in the filtered image
void setFilteredPixel(uint8_t row, uint8_t col, uint8_t value);

// get the value of the pixel in the filtered image
uint8_t getRawPixel(uint8_t row, uint8_t col);

// get the value of the pixel in the filtered image
uint8_t getFilteredPixel(uint8_t row, uint8_t col);

// set histogram1 value
void setHistogram1(uint8_t idx, uint8_t value);

// set histogram2 value
void setHistogram2(uint8_t idx, uint8_t value);

// get histogram1 value
uint8_t getHistogram1(uint8_t idx);

// get histogram2 value
uint8_t getHistogram2(uint8_t idx);

// get pixel in the binned image
uint8_t getBinnedPixel(uint8_t row, uint8_t col);

// copy the raw image into the filtered image
// remove the non-one pixel events
void filterOnePixelEvents();

// apply binning
void applyBinning();

// create histograms from the image
void createHistograms();

void prepareOutput();

// count number of active pixel in the filtered/output image
void computeImageStatistics();

void loadImageParametersFromFram();

void saveImageParametersToFram();

#endif /* IMAGEPROCESSING_H_ */