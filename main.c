/*
 * main.c
 *
 * Created: 24.8.2014 15:10:04
 *  Author: Tomas Baca
 */ 

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "system.h"
#include "mainTask.h"
#include "cspTask.h"

#include <string.h>

extern UsartBuffer * medipix_usart_buffer;

#if MEDIPIX_BOARD == 1

// Blinking RTOS task, just for debugging
void blink(void *p) {
	
	while (1) {
				
		led_red_toggle();
		
        vTaskDelay(500);
	}
}

#endif

int main(void) {
	
	// initialize the xMega peripherals
	boardInit();
	
	// Initialize the CSP buffers
	csp_buffer_init(CSP_BUFFER_COUNT, CSP_BUFFER_SIZE);
			
	// Initialize the CSP
	csp_init(CSP_MY_ADDRESS);
	
	// Initialize the CSP I2C interface
	csp_i2c_init(CSP_I2C_SLAVE_ADDRESS, 0, CSP_I2C_BAUDRATE);
					
	// Add route to OBC via i2c 
	csp_route_set(CSP_OBC_ADDRESS, &csp_if_i2c, CSP_I2C_OBC_ADDRESS);		
					
	// Start router task
	csp_route_start_task(CSP_ROUTER_STACK, CSP_ROUTER_PRIORITY);
				
	#if MEDIPIX_BOARD == 1
	
	/* -------------------------------------------------------------------- */
	/*	Starts blinking task - only for debug								*/
	/* -------------------------------------------------------------------- */
	// xTaskCreate(blink, (signed char*) "blink", 64, NULL, configNORMAL_PRIORITY, NULL);
	
	#endif // MEDIPIX_BOARD == 1
	
	/* -------------------------------------------------------------------- */
	/*	Starts task that handles incoming communication		 				*/
	/* -------------------------------------------------------------------- */
	xTaskCreate(cspTask, (signed char*) "cspTask", 64, NULL, configNORMAL_PRIORITY, NULL);
	
	/* -------------------------------------------------------------------- */
	/*	Starts task that handles outgoing communication		 				*/
	/* -------------------------------------------------------------------- */
	xTaskCreate(mainTask, (signed char*) "mainTask", 2048, NULL, configNORMAL_PRIORITY, NULL);
	
	/* -------------------------------------------------------------------- */
	/*	Starts the scheduler and all previously created tasks				*/
	/* -------------------------------------------------------------------- */
	vTaskStartScheduler();
	
	return 0;
}