/*
 * opticalSensors.h
 *
 * Created: 17.8.2015 14:58:03
 *  Author: klaxalk
 */ 


#ifndef OPTICALSENSORS_H_
#define OPTICALSENSORS_H_

typedef struct __attribute__ ((packed)) {
	uint16_t TIR;
	uint16_t IR;
	uint16_t UV1;
	uint16_t UV2;
	uint16_t TIR_max;
	uint16_t TIR_min;
	uint16_t IR_max;
	uint16_t IR_min;
	uint16_t UV1_max;
	uint16_t UV1_min;
	uint16_t UV2_max;
	uint16_t UV2_min;
} sensors_t;

volatile sensors_t uv_ir_data;

#endif /* OPTICALSENSORS_H_ */