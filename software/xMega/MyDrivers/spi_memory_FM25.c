#include "spi_memory_FM25.h"
#include "config.h"
//#include "spi.h"
#include "spi_master.h"
#include "avr/cpufunc.h"

uint8_t buffer_memory[68];		// buffer for saving and loading data from SPI memory

inline fram_select(void) {
	
	ioport_set_pin_level(FRAM_CS, false);
}

inline fram_deselect(void) {
	
	ioport_set_pin_level(FRAM_CS, true);
}

struct spi_device spi_device_conf = {
	.id = IOPORT_CREATE_PIN(PORTC, 4)		// change pin when is necessary (CS pin for memory)
};

void spi_mem_init(void)
{
	
	ioport_set_pin_dir(FRAM_WP,IOPORT_DIR_OUTPUT);					// Write protect
	ioport_set_pin_level(FRAM_WP, false);
	
	ioport_set_pin_dir(FRAM_CS,IOPORT_DIR_OUTPUT);		// CS
	ioport_set_pin_level(FRAM_CS, true);
	
	//ioport_configure_port_pin(&SPI_PORT, PIN4_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);		// CS
	ioport_configure_port_pin(&SPI_PORT, PIN5_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);		// MOSI
	ioport_configure_port_pin(&SPI_PORT, PIN6_bm, IOPORT_DIR_INPUT);							// MISO
	ioport_configure_port_pin(&SPI_PORT, PIN7_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);		// SCK
	
	spi_master_init(&SPI_MEM_INTERFACE);
	spi_master_setup_device(&SPI_MEM_INTERFACE, &spi_device_conf, SPI_MODE_0, 1000000, 0);
	spi_enable(&SPI_MEM_INTERFACE);

	led_red_toggle();
}

void spi_mem_write_byte(unsigned long address, uint8_t data) {
	
	spi_command command;
	memcpy(&command, &address, 4);
	
	buffer_memory[0] = SPI_WRITE;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;
	buffer_memory[4] = data;
	
	fram_unprotect();
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 5);
	fram_deselect();
	fram_protect();
	led_red_toggle();
	
	uint8_t i;
	for (i = 0; i < NUMBER_OF_NOOPS; i++) {
		
		_NOP();
	}
}

uint8_t spi_mem_read_byte(unsigned long address) {
	
	spi_command command;
	memcpy(&command, &address, 4);
	
	buffer_memory[0] = SPI_READ;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;
	
	fram_unprotect();
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	spi_read_packet(&SPI_MEM_INTERFACE, buffer_memory, 1);
	fram_deselect();
	fram_protect();
	
	led_yellow_toggle();
	
	uint8_t i;
	for (i = 0; i < NUMBER_OF_NOOPS; i++) {
		
		_NOP();
	}
	
	return buffer_memory[0];
}

void spi_mem_write_uint16(unsigned long address, uint16_t value) {
	
	uint8_t * tempPtr;
	tempPtr = (uint8_t *) (&value);
	
	// write the first byte of the uint16t
	spi_mem_write_byte(address, *tempPtr);
	
	// write the second byte of the uint16t
	spi_mem_write_byte(address+1, *(tempPtr + 1));	
}

uint16_t spi_mem_read_uint16t(unsigned long address) {
	
	// the return value
	uint16_t tempValue;
	
	uint8_t * tempPtr;
	tempPtr = (uint8_t *) (&tempValue);
	
	// read the first byte of the uint16t
	*tempPtr = spi_mem_read_byte(address);
	
	// read the second byte of the uint16t
	*(tempPtr + 1) = spi_mem_read_byte(address+1);
}

// save a blob of data (max 256B) to spi memory
void spi_mem_write_blob(unsigned long address, uint8_t * data, uint8_t size) {
	
	uint16_t i;
	
	for (i = 0; i < size; i++) {
		
		spi_mem_write_byte(address+i, *(data+i));
	}
}

// read the blob of data from the spi memory
void spi_mem_read_blob(unsigned long address, uint8_t * data, uint8_t size) {

	uint16_t i;
	
	for (i = 0; i < size; i++) {
		
		*(data + i) = spi_mem_read_byte(address+i);
	}
}

void spi_mem_write_command(uint8_t command){

	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, &command, 1);
	fram_deselect();
}

void fram_protect() {
	
	ioport_set_pin_level(FRAM_WP, false);
	spi_mem_write_command(SPI_WRDI);
}

void fram_unprotect() {
	
	ioport_set_pin_level(FRAM_WP, true);
	spi_mem_write_command(SPI_WREN);
}

void spi_mem_clear_all() {
	
	unsigned long i;
	
	for (i = 0; i < 1024*256*8; i++) {
		
		spi_mem_write_byte(i, 0);
	}
}