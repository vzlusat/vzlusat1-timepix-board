#include "spi_memory_FM25.h"
#include "config.h"
//#include "spi.h"
#include "spi_master.h"


char buffer_memory[68];		// buffer for saving and loading data from SPI memory

#define FRAM_CS		IOPORT_CREATE_PIN(PORTC, 4)

inline fram_select(void){
	ioport_set_pin_low(FRAM_CS);
}

inline fram_deselect(void){
	ioport_set_pin_high(FRAM_CS);

}
struct spi_device spi_device_conf = {
	.id = IOPORT_CREATE_PIN(PORTC, 4)		// change pin when is necessary (CS pin for memory)
};



void spi_mem_init(void)
{
	
	ioport_set_pin_dir(FRAM_WP,IOPORT_DIR_OUTPUT);					// Write protect
	ioport_set_pin_high(FRAM_WP);
	
	ioport_set_pin_dir(FRAM_CS,IOPORT_DIR_OUTPUT);		// CS
	ioport_set_pin_high(FRAM_CS);
	
	//ioport_configure_port_pin(&SPI_PORT, PIN4_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);		// CS
	ioport_configure_port_pin(&SPI_PORT, PIN5_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);		// MOSI
	ioport_configure_port_pin(&SPI_PORT, PIN6_bm, IOPORT_DIR_INPUT);							// MISO
	ioport_configure_port_pin(&SPI_PORT, PIN7_bm, IOPORT_INIT_HIGH | IOPORT_DIR_OUTPUT);		// SCK
	

	spi_master_init(&SPI_MEM_INTERFACE);
	spi_master_setup_device(&SPI_MEM_INTERFACE, &spi_device_conf, SPI_MODE_0, 1000000, 0);
	spi_enable(&SPI_MEM_INTERFACE);
	
	fram_unprotect();
	//leda_toggle();
	
}

void spi_mem_write_page(unsigned long address, char * data , unsigned int no_pages)
{
	spi_command command;
	memcpy(&command,&address,4);
	
	buffer_memory[0] = SPI_WRITE;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;

	memcpy(&buffer_memory[4], data,no_pages);
	
	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	memcpy(buffer_memory,&data,no_pages);
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, no_pages);
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_deselect();

}

void spi_mem_write_word(unsigned long address, unsigned int data)
{
	spi_command command;
	memcpy(&command,&address,4);
	
	buffer_memory[0] = SPI_WRITE;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;
	buffer_memory[4] = data>>8;
	buffer_memory[5] = data;
	
	
	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 6);
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_deselect();
}

void spi_mem_write_byte(unsigned long address, unsigned char data)
{
	spi_command command;
	memcpy(&command,&address,4);
	
	buffer_memory[0] = SPI_WRITE;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;
	buffer_memory[4] = data;
	
	fram_unprotect();
	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 5);
	fram_deselect();
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);

}

void spi_mem_write_complex(unsigned long address, complex data)
{
	spi_command command;
	memcpy(&command,&address,4);
	
	buffer_memory[0] = SPI_WRITE;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;
	
	
	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	memcpy(buffer_memory,&data,8);
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 8);
	fram_deselect();
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);

	
}

void spi_mem_write_long(unsigned long address, long data)
{
	spi_command command;
	memcpy(&command,&address,4);
	
	buffer_memory[0] = SPI_WRITE;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;


	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	memcpy(buffer_memory,&data,4);
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	fram_deselect();
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	

	
}

void spi_mem_read_page(unsigned long address, char * readed_data,  unsigned char no_pages)
{
	unsigned int data;
	spi_command command;
	memcpy(&command,&address,4);
	
	buffer_memory[0] = SPI_READ;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;
	
	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	spi_read_packet(&SPI_MEM_INTERFACE, buffer_memory, no_pages);
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_deselect();
	
	memcpy(readed_data, buffer_memory, no_pages);
}

unsigned int spi_mem_read_word(unsigned long address)
{
	unsigned int data;
	spi_command command;
	memcpy(&command,&address,4);
	
	buffer_memory[0] = SPI_READ;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;
	
	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	spi_read_packet(&SPI_MEM_INTERFACE, buffer_memory, 2);
	fram_deselect();
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	
	data = (int)(buffer_memory[0]<<8);
	data |= (int) buffer_memory[1];
	
	return data;
}

char spi_mem_read_byte(unsigned long address)
{
	spi_command command;
	memcpy(&command,&address,4);
	
	buffer_memory[0] = SPI_READ;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;
	
	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	spi_read_packet(&SPI_MEM_INTERFACE, buffer_memory, 1);
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_deselect();
	
	return buffer_memory[0];
}

complex spi_mem_read_complex(unsigned long address)
{
	complex data;
	spi_command command;
	memcpy(&command,&address,4);
	
	buffer_memory[0] = SPI_READ;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;
	
	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	spi_read_packet(&SPI_MEM_INTERFACE, buffer_memory, 8);
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_deselect();
	
	memcpy(&data,buffer_memory,8);
	
	return data;
}

long spi_mem_read_long(unsigned long address)
{
	long data;
	spi_command command;
	memcpy(&command,&address,4);
	
	buffer_memory[0] = SPI_READ;
	buffer_memory[1] = command.addr1;
	buffer_memory[2] = command.addr2;
	buffer_memory[3] = command.addr3;
	

	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	spi_read_packet(&SPI_MEM_INTERFACE, buffer_memory, 4);
	fram_deselect();
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	
	memcpy(&data,buffer_memory,4);
	
	return data;
}

void spi_mem_write_command(uint8_t command){
	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, &command, 1);
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_deselect();
}

void fram_protect(void){
	spi_mem_write_command(SPI_WRDI);
	ioport_set_pin_low(FRAM_WP);

}

void fram_unprotect(void){
	spi_mem_write_command(SPI_WREN);
	ioport_set_pin_high(FRAM_WP);
}

char spi_mem_read_status(void){
	buffer_memory[0] = SPI_RDSR;
	
	//spi_select_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_select();
	spi_write_packet(&SPI_MEM_INTERFACE, buffer_memory, 1);
	spi_read_packet(&SPI_MEM_INTERFACE, buffer_memory, 1);
	//spi_deselect_device(&SPI_MEM_INTERFACE, &spi_device_conf);
	fram_deselect();
	
	return buffer_memory[0];
}