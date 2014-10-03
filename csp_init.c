/*
 * csp_init.c
 *
 * Created: 29.9.2014 21:39:45
 *  Author: klaxalk
 */

#include "i2c.h"
#include "twi_master_driver.h"
#include "twi_slave_driver.h"
#include "ioport.h"

#define	YELLOW	IOPORT_CREATE_PIN(PORTA, 0)

/*! CPU speed 2MHz, BAUDRATE 100kHz and Baudrate Register Settings */
#define CPU_SPEED	32000000UL
#define BAUDRATE	100000
#define TWI_BAUDSETTING TWI_BAUD(CPU_SPEED, BAUDRATE)

TWI_Master_t twiMaster;      /*!< TWI slave module. */
TWI_Slave_t twiSlave;      /*!< TWI slave module. */
i2c_callback_t callbackFunction;

// this function processes received data on the I2C Slave line
// It is call by the I2C driver
void TWIE_SlaveProcessData(void) {
	
	i2c_frame_t newFrame;
	
	if (twiSlave.bytesReceived > 0) {
		
		// set the data len
		newFrame.len = twiSlave.bytesReceived;
		
		// copy the data
		int i;
		for (i = 0; i < twiSlave.bytesReceived; i++) {
			
			newFrame.data[i] = twiSlave.receivedData[i];
		}
	}
	
	callbackFunction(&newFrame, false);
}

int i2c_init(int handle, int mode, uint8_t addr, uint16_t speed, int queue_len_tx, int queue_len_rx, i2c_callback_t callback) {
	
	// Initialize TWI master on PORTC
	TWI_MasterInit(&twiMaster, &TWIE, TWI_MASTER_INTLVL_LO_gc, TWI_BAUDSETTING);
			
	// Initialize TWI slave on PORTE
	TWI_SlaveInitializeDriver(&twiSlave, &TWIE, TWIE_SlaveProcessData);
	TWI_SlaveInitializeModule(&twiSlave, 0x56, TWI_SLAVE_INTLVL_LO_gc);
	
	callbackFunction = callback;
	
	//!!! pøepsat return
	return 1;
}

/**
 * Send I2C frame via the selected device
 *
 * @param handle Handle to the device
 * @param frame Pointer to I2C frame
 * @param timeout Ticks to wait
 * @return Error code
 */
int i2c_send(int handle, i2c_frame_t * frame, uint16_t timeout) {
	
	if (TWI_MasterWrite(&twiMaster, 0x55, (uint8_t *) &(*frame).data, (*frame).len)) {
		
		return E_NO_ERR;
	} else {
		
		return 1;
	}
}

/*! TWIC Master Interrupt vector. */
ISR(TWIE_TWIM_vect)
{
	TWI_MasterInterruptHandler(&twiMaster);
}

/*! TWIE Slave Interrupt vector. */
ISR(TWIE_TWIS_vect)
{
	TWI_SlaveInterruptHandler(&twiSlave);
}