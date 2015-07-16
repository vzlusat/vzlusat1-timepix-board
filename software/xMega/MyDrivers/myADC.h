/*
 * ADC.h
 *
 * Created: 30.5.2015 16:23:43
 *  Author: Ondra
 */ 

#include <avr/io.h>
#ifndef ADC_H_
#define ADC_H_

int16_t adc_read_ch0(void);		// TIR
int16_t adc_read_ch1(void);		// IR
int16_t adc_read_ch2(void);		// UV1
int16_t adc_read_ch3(void);		// UV2
void adc_init(void);

typedef struct __attribute__ ((packed)) {
	uint16_t TIR;
	uint16_t IR;
	uint16_t UV1;
	uint16_t UV2;
} sensors_t;

volatile sensors_t uv_ir_data;

#endif /* ADC_H_ */