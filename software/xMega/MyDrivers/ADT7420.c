/*
 * ADT7420.c
 *
 * Created: 22/02/2015 17:29:32
 *  Author: Ondra
 */ 

#include "twi_master_driver.h"
#include "ADT7420.h"
#include "config.h"
#include <math.h>
#include "medipix.h"
#include "system.h"

TWI_Master_t twi_adt_master;		/*!< TWI slave module. */
char adt_write_buffer[8];
	
void ADT_init(void)
{
	// Initialize TWI master for CSP
	TWI_MasterInit(&twi_adt_master, &ADT_I2C_INTERFACE, TWI_MASTER_INTLVL_LO_gc, ADT_I2C_BAUDSETTING);
	adt_write_buffer[0] = ADT_REG_CONFIGURATION;
	adt_write_buffer[1] = 0xC0;						// 16 bit, one shot (240ms), ...
	TWI_MasterWriteRead(&twi_adt_master,ADT_I2C_ADDRESS,&adt_write_buffer,2,0);
}

int ADT_get_temperature(void) {

	adt_write_buffer[0] = ADT_REG_TEMPERATURE;
	TWI_MasterWriteRead(&twi_adt_master,ADT_I2C_ADDRESS,&adt_write_buffer,1,2);		// 240 ms needed to convert temperature
	
	vTaskDelay(250);
	
	adt_write_buffer[0] = ADT_REG_TEMPERATURE;
	TWI_MasterWriteRead(&twi_adt_master,ADT_I2C_ADDRESS,&adt_write_buffer,1,2);		// 240 ms needed to convert temperature

	return (twi_adt_master.readData[0]<<8) + (twi_adt_master.readData[1]);
}


/*!  Master Interrupt vector for ADT7420. */
ISR(ADT_I2C_TWIM)
{
	TWI_MasterInterruptHandler(&twi_adt_master);
}