/*
 * ADC.h
 *
 * Created: 30.5.2015 16:23:43
 *  Author: Ondra
 */ 

#include <avr/io.h>
#include "opticalSensors.h"

#ifndef ADC_H_
#define ADC_H_

int16_t adc_read_ch0(void);		// TIR
int16_t adc_read_ch1(void);		// IR
int16_t adc_read_ch2(void);		// UV1
int16_t adc_read_ch3(void);		// UV2
void adc_init(void);

#endif /* ADC_H_ */