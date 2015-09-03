/*
 * adtTask.c
 *
 * Created: 6.7.2015 13:49:04
 *  Author: klaxalk
 */ 

#include "adtTask.h"
#include "system.h"
#include "medipix.h"
#include "mainTask.h"
#include "ADT7420.h"
#include "imageProcessing.h"
#include "opticalSensors.h"

volatile int8_t adtTemp = 0;
volatile int8_t adtTemp_max = 0;
volatile int8_t adtTemp_min = 0;

/* -------------------------------------------------------------------- */
/*	The adt task														*/
/* -------------------------------------------------------------------- */
void adtTask(void *p) {
	
	/* -------------------------------------------------------------------- */
	/*	Initialize ADT sensor												*/
	/* -------------------------------------------------------------------- */
	ADT_init();
	
	vTaskDelay(3000);
	
	adtTemp = adt_convert_temperature(ADT_get_temperature());
	
	adtTemp_max = INT8_MIN;
	adtTemp_min = INT8_MAX;

	while (true) {
		
		adtTemp = adt_convert_temperature(ADT_get_temperature());

		if (adtTemp > imageParameters.temperatureLimit) {

			pwrOffMedipix();
		}
		
		if (adtTemp > adtTemp_max)
			adtTemp_max = adtTemp;
		
		if (adtTemp < adtTemp_min)
			adtTemp_min = adtTemp;

		if ((int16_t) uv_ir_data.TIR > (int16_t) uv_ir_data.TIR_max)
			uv_ir_data.TIR_max = uv_ir_data.TIR;

		if ((int16_t) uv_ir_data.TIR < (int16_t) uv_ir_data.TIR_min)
			uv_ir_data.TIR_min = uv_ir_data.TIR;

		if ((int16_t) uv_ir_data.IR > (int16_t) uv_ir_data.IR_max)
			uv_ir_data.IR_max = uv_ir_data.IR;

		if ((int16_t) uv_ir_data.IR < (int16_t) uv_ir_data.IR_min)
			uv_ir_data.IR_min = uv_ir_data.IR;

		if ((int16_t) uv_ir_data.UV1 > (int16_t) uv_ir_data.UV1_max)
			uv_ir_data.UV1_max = uv_ir_data.UV1;

		if ((int16_t) uv_ir_data.UV1 < (int16_t) uv_ir_data.UV1_min)
			uv_ir_data.UV1_min = uv_ir_data.UV1;

		if ((int16_t) uv_ir_data.UV2 > (int16_t) uv_ir_data.UV2_max)
			uv_ir_data.UV2_max = uv_ir_data.UV2;

		if ((int16_t) uv_ir_data.UV2 < (int16_t) uv_ir_data.UV2_min)
			uv_ir_data.UV2_min = uv_ir_data.UV2;
	}
}