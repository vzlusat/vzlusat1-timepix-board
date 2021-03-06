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
#include "twi_master_driver.h"
#include "twi_slave_driver.h"

// LEDs
#define	YELLOW	IOPORT_CREATE_PIN(PORTD, 4)
#define	RED	IOPORT_CREATE_PIN(PORTD, 0)

// Baudrate of the CSP i2c interface
#define CSP_I2C_BAUDRATE	400000
#define CSP_I2C_BAUDSETTING TWI_BAUD(F_CPU, CSP_I2C_BAUDRATE)

/* -------------------------------------------------------------------- */
/*	Macros for manipulating with LEDs									*/
/* -------------------------------------------------------------------- */
#define led_yellow_on()		ioport_set_pin_level(YELLOW, false)
#define led_yellow_off()	ioport_set_pin_level(YELLOW, true)
#define led_yellow_toggle()	ioport_toggle_pin_level(YELLOW)
#define led_red_on()		ioport_set_pin_level(RED, false)
#define led_red_off()		ioport_set_pin_level(RED, true)
#define led_red_toggle()	ioport_toggle_pin_level(RED)
	
// Uart
extern UsartBuffer * pc_usart_buffer;

// i2c
TWI_Master_t twi_csp_master;		/*!< TWI slave module. */
TWI_Slave_t twi_csp_slave;			/*!< TWI slave module. */
	
/* -------------------------------------------------------------------- */
/*	Initialize the xMega peripherals									*/
/* -------------------------------------------------------------------- */
void boardInit();

void TWI_CSP_SlaveProcessData(void);
int i2c_send(int8_t destination, uint8_t * buffer, int8_t data_len);

#endif /* SYSTEM_H_ */