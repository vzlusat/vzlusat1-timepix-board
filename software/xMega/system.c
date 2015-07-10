/*
 * system.c
 *
 * Created: 11.10.2014 18:40:47
 *  Author: Tomas Baca
 */

 #include "sysclk.h"
 #include "system.h"
 #include "TC_driver.h"
 #include "medipixTask.h"
 #include "ADT7420.h"
 #include "spi_memory_FM25.h"
 #include "fram_mapping.h"
 
 volatile uint16_t milisecondsTimer;
 volatile uint32_t secondsTimer;
 
 // UART handler
 UsartBuffer * medipix_usart_buffer;
 
 // queue for command for medipix
 xQueueHandle * medipixActionQueue;

/* -------------------------------------------------------------------- */
/*	Initialize the xMega peripherals									*/
/* -------------------------------------------------------------------- */
void boardInit() {
	
	// prepare the i/o for LEDs
	ioport_init();
	
	// clock init & enable system clock to all peripheral modules
	sysclk_init();
	
	sysclk_enable_module(SYSCLK_PORT_GEN, 0xff);
	sysclk_enable_module(SYSCLK_PORT_A, 0xff);
	sysclk_enable_module(SYSCLK_PORT_B, 0xff);
	sysclk_enable_module(SYSCLK_PORT_C, 0xff);
	sysclk_enable_module(SYSCLK_PORT_D, 0xff);
	sysclk_enable_module(SYSCLK_PORT_E, 0xff);
	sysclk_enable_module(SYSCLK_PORT_F, 0xff);
		
	/* -------------------------------------------------------------------- */
	/*	Timer for RTC														*/
	/* -------------------------------------------------------------------- */
	
	// select the clock source and pre-scaler by 8
	TC1_ConfigClockSource(&TCC1, TC_CLKSEL_DIV64_gc);
	
	TC1_SetOverflowIntLevel(&TCC1, TC_OVFINTLVL_LO_gc);
	
	TC_SetPeriod(&TCC1, 499);
	
	milisecondsTimer = 0;
	secondsTimer = 0;
	
	/* -------------------------------------------------------------------- */
	/*	Setup LEDs															*/
	/* -------------------------------------------------------------------- */
	
	ioport_set_pin_dir(YELLOW, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(RED, IOPORT_DIR_OUTPUT);
	
	led_yellow_off();
	led_red_off();
		
	/* -------------------------------------------------------------------- */
	/*	Setup Medipix Enabler												*/
	/* -------------------------------------------------------------------- */
	
	ioport_set_pin_dir(MEDIPIX_PWR, IOPORT_DIR_OUTPUT);
	pwrOffMedipix();
		
	/* -------------------------------------------------------------------- */
	/*	Setup UART															*/
	/* -------------------------------------------------------------------- */
	medipix_usart_buffer = usartBufferInitialize(&MPX_USART, MPX_USART_BAUDRATE, MPX_USART_BUFFERSIZE);

	/* -------------------------------------------------------------------- */
	/*	Initialize FRAM memory												*/
	/* -------------------------------------------------------------------- */
	spi_mem_init();

	/* -------------------------------------------------------------------- */
	/*	Increment the boot count											*/
	/* -------------------------------------------------------------------- */
	increaseBootCount();
}

// return the number of boots
uint16_t getBootCount() {
	
	return spi_mem_read_uint16t(BOOT_COUNT_ADDRESS);
}

// increase the number of boots
void increaseBootCount() {
	
	uint16_t tempInt;
	
	tempInt = spi_mem_read_uint16t(BOOT_COUNT_ADDRESS);
	tempInt++;
	
	spi_mem_write_uint16(BOOT_COUNT_ADDRESS, tempInt);
}

/* -------------------------------------------------------------------- */
/*	Interrupt for timing the RTC										*/
/* -------------------------------------------------------------------- */
ISR(TCC1_OVF_vect) {
	
	// shut down the output PPM pulse

	if (milisecondsTimer++ == 1000) {
		
		milisecondsTimer = 0;
		
		secondsTimer++;
	}
}