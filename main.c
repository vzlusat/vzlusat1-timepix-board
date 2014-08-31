/*
 * main.c
 *
 * Created: 24.8.2014 15:10:04
 *  Author: klaxalk
 */ 


#include <avr/io.h>
#include <util/delay.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ioport.h"
#include "usart_driver_RTOS.h"

// YELLOW LED on PORTA-0
#define	YELLOW	IOPORT_CREATE_PIN(PORTA, 0)

// RED LED on PORTA-4
#define	RED	IOPORT_CREATE_PIN(PORTA, 4)

// define the usart port
#define PC_USART	 USARTC0

void EnableInter32MhzOsc() {
	
	CCP = CCP_IOREG_gc;// disable register security for oscillator update
	OSC.CTRL = OSC_RC32MEN_bm; // enable 32MHz oscillator
	while(!(OSC.STATUS & OSC_RC32MRDY_bm)); // wait for oscillator to be ready
	CCP = CCP_IOREG_gc; //disable register security for clock update
	CLK.CTRL = CLK_SCLKSEL_RC32M_gc; // switch to 32MHz clock
}

void blink1(void *p) {
	
	while (1) {
		
		ioport_toggle_pin_level(RED);
        vTaskDelay(1000);
	}
}

void blink2(void *p) {
	
	while (1) {
		
		ioport_toggle_pin_level(YELLOW);
		vTaskDelay(100);
	}
}

void uartLoopBack(void *p) {
	
	unsigned char znak;
	
	UsartBuffer * pc_usart_buffer = usartBufferInitialize(&PC_USART, BAUD19200, 128);
	usartBufferPutString(pc_usart_buffer, "\n\n\rXMEGA ready", 10);
	
	while (1) {
		
		if (usartBufferGetByte(pc_usart_buffer, &znak, 0)) {
			
			usartBufferPutByte(pc_usart_buffer, znak, 10);
		}
	}
}

int main(void)
{	
	
	EnableInter32MhzOsc();
	
	// prepare the i/o for LEDs
	ioport_init();
	ioport_set_pin_dir(RED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(YELLOW, IOPORT_DIR_OUTPUT);
			
	// start tasks
	xTaskCreate(blink1, (signed char*) "blink1", 1024, NULL, 2, NULL);
	xTaskCreate(blink2, (signed char*) "blink2", 1024, NULL, 2, NULL);
	xTaskCreate(uartLoopBack, (signed char*) "uart1", 1024, NULL, 2, NULL);
	
	vTaskStartScheduler();
	
	return 0;
}