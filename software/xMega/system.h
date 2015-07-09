/*
 * system.h
 *
 * Created: 11.10.2014 18:40:01
 *  Author: Tomas Baca
 */ 

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <avr/io.h>
#include "config.h"
#include "ioport.h"
#include "usart_driver_RTOS.h"
#include "csp.h"
#include "csp_thread.h"
#include "csp_clock.h"
#include "csp_if_i2c.h"

// LEDs
#define	YELLOW			IOPORT_CREATE_PIN(PORTA, 7)
#define	RED				IOPORT_CREATE_PIN(PORTA, 6)
#define	MEDIPIX_PWR		IOPORT_CREATE_PIN(PORTD, 1)
	
/* -------------------------------------------------------------------- */
/*	Macros for manipulating with LEDs									*/
/* -------------------------------------------------------------------- */
#define led_yellow_on()		ioport_set_pin_level(YELLOW, false)
#define led_yellow_off()	ioport_set_pin_level(YELLOW, true)
#define led_yellow_toggle()	ioport_toggle_pin_level(YELLOW)
#define led_red_on()		ioport_set_pin_level(RED, true)
#define led_red_off()		ioport_set_pin_level(RED, false)
#define led_red_toggle()	ioport_toggle_pin_level(RED)
 
// UART handler
UsartBuffer * medipix_usart_buffer;

/* -------------------------------------------------------------------- */
/*	RTC																	*/
/* -------------------------------------------------------------------- */
volatile uint16_t milisecondsTimer;
volatile uint32_t secondsTimer;

/* -------------------------------------------------------------------- */
/*	Additional quues													*/
/* -------------------------------------------------------------------- */

xQueueHandle * medipixActionQueue;

/* -------------------------------------------------------------------- */
/*	Initialize the xMega peripherals									*/
/* -------------------------------------------------------------------- */
void boardInit();

uint16_t getBootCount();

#endif /* SYSTEM_H_ */