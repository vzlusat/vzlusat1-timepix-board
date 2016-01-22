/*
 * ADC.c
 *
 * Created: 30.5.2015 16:23:23
 *  Author: Ondra
 */ 

#include <../asf.h>
#include "myADC.h"

void adc_init(void) {
	
	PORTA.DIR = 0;	 // configure PORTA as input
	ADCA.CTRLA |= 0x1;	 // enable adc
	ADCA.CTRLB =  ADC_CONMODE_bm | ADC_RESOLUTION_12BIT_gc;	 // ADC_CONMODE_bm |  12 bit conversion, conversion mode - signed - diff
	
	ADCA.REFCTRL = 0x12;	 // internal Vcc/1.6 (3,3/1,6=2,0625V)
	ADCA.PRESCALER = ADC_PRESCALER_DIV256_gc;	 // peripheral clk/256 (32 MHz ... 125 kHz)
	ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;		// single ended - TIR
	ADCA.CH1.CTRL = ADC_CH_INPUTMODE_DIFF_gc;				// differential mode - IR
	ADCA.CH2.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;		// single ended - UV1
	ADCA.CH3.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;		// single ended - UV2
	ADCA.EVCTRL = ADC_EVACT_CH0123_gc|ADC_SWEEP_0123_gc | ADC_EVSEL_0123_gc;		// 0xC4
	//ADCA.EVCTRL = 0x01;
	
	// in differential mode ****************************
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc ;	//
	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc | ADC_CH_MUXNEG_PIN2_gc ;	//
	ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN3_gc ;	//
	ADCA.CH3.MUXCTRL = ADC_CH_MUXPOS_PIN4_gc ;	//
	
	uv_ir_data.TIR_max = -32000;
	uv_ir_data.TIR_min = 32000;
	uv_ir_data.IR_max = -32000;
	uv_ir_data.IR_min = 32000;
	uv_ir_data.UV1_max = -32000;
	uv_ir_data.UV1_min = 32000;
	uv_ir_data.UV2_max = -32000;
	uv_ir_data.UV2_min = 32000;
}

int16_t adc_read_ch0(void) {
	
	int16_t adc_sum;
	uint8_t i;

	adc_sum=0;
	for(i=0;i<4;++i) {
		
		ADCA.CH0.CTRL |= ADC_CH_START_bm; // start conversion on channel 0
		while(!ADCA.CH0.INTFLAGS);
			adc_sum += ADCA.CH0RES;
	}
	
	adc_sum = adc_sum >> 2;
	return adc_sum;
}

int16_t adc_read_ch1(void) {
	
	int16_t adc_sum;
	uint8_t i;

	adc_sum=0;
	for(i=0;i<4;++i) {
		
		ADCA.CH1.CTRL |= ADC_CH_START_bm; // start conversion on channel 0
		while(!ADCA.CH1.INTFLAGS);
			adc_sum += ADCA.CH1RES;
	}
	
	adc_sum = adc_sum >> 2;
	return adc_sum;	
}

int16_t adc_read_ch2(void) {
	
	int16_t adc_sum;
	uint8_t i;

	adc_sum=0;
	for(i=0;i<4;++i) {
		
		ADCA.CH2.CTRL |= ADC_CH_START_bm; // start conversion on channel 0
		while(!ADCA.CH2.INTFLAGS);
			adc_sum += ADCA.CH2RES;
	}
	
	adc_sum = adc_sum >> 2;
	return adc_sum;
}

int16_t adc_read_ch3(void) {
	
	int16_t adc_sum;
	uint8_t i;

	adc_sum=0;
	for(i=0;i<4;++i) {
		
		ADCA.CH3.CTRL |= ADC_CH_START_bm; // start conversion on channel 0
		while(!ADCA.CH3.INTFLAGS);
			adc_sum += ADCA.CH3RES;
	}
	
	adc_sum = adc_sum >> 2;
	return adc_sum;
}