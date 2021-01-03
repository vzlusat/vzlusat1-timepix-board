/*
 * main.c
 *
 * Created: 24.8.2014 15:10:04
 *  Author: Tomas Baca
 * Flashed on 18.1.2017
 */ 

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "system.h"
#include "mainTask.h"
#include "cspTask.h"
#include "adtTask.h"

int main(void) {
	
	// initialize the xMega peripherals
	boardInit();
	
	// Initialize the CSP buffers
	csp_buffer_init(CSP_BUFFER_COUNT, CSP_BUFFER_SIZE);
			
	// Initialize the CSP
	csp_init(CSP_MY_ADDRESS);
	
	// Initialize the CSP I2C interface
	csp_i2c_init(CSP_I2C_SLAVE_ADDRESS, 0, CSP_I2C_BAUDSETTING);
					
	// Add route to OBC via i2c
	csp_route_set(CSP_OBC_ADDRESS, &csp_if_i2c, CSP_I2C_OBC_ADDRESS);
	csp_route_set(CSP_RADIO_ADDRESS, &csp_if_i2c, CSP_I2C_COM_ADRESS);
	csp_route_set(CSP_DEFAULT_ROUTE, &csp_if_i2c, CSP_I2C_COM_ADRESS);
	csp_route_set(CSP_DEBUG_ADRESS, &csp_if_i2c, CSP_I2C_OBC_ADDRESS);
					
	// Start router task
	csp_route_start_task(CSP_ROUTER_STACK, CSP_ROUTER_PRIORITY);
			
	/* -------------------------------------------------------------------- */
	/*	Starts task that handles incoming communication		 				*/
	/* -------------------------------------------------------------------- */
	xTaskCreate(cspTask, (signed char*) "cspTask", 400, NULL, configNORMAL_PRIORITY, NULL);
	
	/* -------------------------------------------------------------------- */
	/*	Starts task that handles outgoing communication		 				*/
	/* -------------------------------------------------------------------- */
	xTaskCreate(mainTask, (signed char*) "mainTask", 1600, NULL, configNORMAL_PRIORITY, NULL);
	
	/* -------------------------------------------------------------------- */
	/*	Starts task that handles outgoing communication		 				*/
	/* -------------------------------------------------------------------- */
	xTaskCreate(adtTask, (signed char*) "adtTask", 300, NULL, configNORMAL_PRIORITY, NULL);
	
	/* -------------------------------------------------------------------- */
	/*	Starts the scheduler and all previously created tasks				*/
	/* -------------------------------------------------------------------- */
	vTaskStartScheduler();
	
	return 0;
}