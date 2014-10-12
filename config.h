/*
 * config.h
 *
 * Created: 11.10.2014 18:31:37
 *  Author: Tomas Baca
 */ 

#ifndef CONFIG_H_
#define CONFIG_H_

// If the board is the Medipix board, compile some board specific stuff
#define MEDIPIX_BOARD	1

// Address of the local CSP node
#define CSP_MY_ADDRESS	2

// Address of the OBC CSP node
#define CSP_OBC_ADDRESS	1	

// Actual position of CSP i2c on the xMega
// Replace "TWIE" with adequate port of your board
#define CSP_I2C_INTERFACE	TWIE
#define CSP_I2C_TWIM		TWIE_TWIM_vect
#define CSP_I2C_TWIS		TWIE_TWIS_vect

// Slave address of the CSP i2c interface
#define CSP_I2C_SLAVE_ADDRESS	0x55

// Slave address of the CSP i2c interface
#define CSP_I2C_OBC_ADDRESS	0x56

// Number of CSP buffer to allocate
#define CSP_BUFFER_COUNT	1

// CSP Buffer size in bytes
#define CSP_BUFFER_SIZE	86

// Maximum size of CSP packet, payload + header
#define CSP_PACKET_SIZE	72

// Size of CSP router stack in words
#define CSP_ROUTER_STACK	500

// Do not ever change!!!
#define CSP_ROUTER_PRIORITY	0

// Baudrate of the CSP i2c interface
#define CSP_I2C_BAUDRATE	400000

#if MEDIPIX_BOARD == 1

// define the usart port for MEDIPIX
#define MPX_USART	USARTC0

#define MPX_USART_BAUDRATE	BAUD57600

#define MPX_USART_BUFFERSIZE	128	

#endif

#endif /* CONFIG_H_ */