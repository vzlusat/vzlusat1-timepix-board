/*
 * adtTask.c
 *
 * Created: 6.7.2015 13:49:04
 *  Author: klaxalk
 */ 

#include "adtTask.h"
#include "system.h"

int8_t adtTemp = 0;

/* -------------------------------------------------------------------- */
/*	The adt task														*/
/* -------------------------------------------------------------------- */
void adtTask(void *p) {
	
	/* -------------------------------------------------------------------- */
	/*	Initialize ADT sensor												*/
	/* -------------------------------------------------------------------- */
	ADT_init();

	while (true) {
		
		adtTemp = adt_convert_temperature(ADT_get_temperature());
		
		vTaskDelay(5000);
	}
}