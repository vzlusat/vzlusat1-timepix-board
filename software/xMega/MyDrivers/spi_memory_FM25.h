#include "system.h"
#include "config.h"
#include "spi.h"

#ifndef SPI_MEMORY
#define SPI_MEMORY

// #define FRAM_CS		IOPORT_CREATE_PIN(PORTD, 4)		// letovy
#define FRAM_CS		IOPORT_CREATE_PIN(PORTC, 4)			// prototyp

#define	FRAM_WP		IOPORT_CREATE_PIN(PORTB, 1)

#define NUMBER_OF_NOOPS	1

typedef struct               // one point is 8 bytes large
{
	long real;
	long imag;
} complex;

typedef struct
{
	char addr3;
	char addr2;
	char addr1;
	char cmd;
	
} spi_command;

void spi_mem_write_command(uint8_t command);

uint8_t spi_mem_read_byte(unsigned long address);
void spi_mem_write_byte(unsigned long address, uint8_t data);

void spi_mem_write_uint16(unsigned long address, uint16_t value);
uint16_t spi_mem_read_uint16t(unsigned long address);

void spi_mem_write_blob(unsigned long address, uint8_t * data, uint8_t size);
void spi_mem_read_blob(unsigned long address, uint8_t * data, uint8_t size);

void spi_mem_init(void);
void fram_unprotect(void);
void fram_protect(void);
void memory_unprotect(void);

void spi_mem_clear_all();

// definition of opcodes for SPI memory
#define SPI_READ 0x03		// read data from memory
#define SPI_WRITE 0x02		// write data to memory
#define SPI_WREN 0x06		// set the write enable latch - enable write operations
#define SPI_WRDI 0x04		// reset the write enable latch - disable write operations
#define SPI_RDSR 0x05		// read status register
#define SPI_WRSR 0x01		// write status register

#endif