/*
 * system.c
 *
 * Created: 11.10.2014 18:40:47
 *  Author: Tomas Baca 
 */

 #include "sysclk.h"
 #include "system.h"

 csp_packet_t * outcomingPacket;
 
 #if MEDIPIX_BOARD == 1
 
 UsartBuffer * medipix_usart_buffer;
 
 #endif // MEDIPIX_BOARD == 1

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
		
	#if MEDIPIX_BOARD == 1
	
	ioport_set_pin_dir(RED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(YELLOW, IOPORT_DIR_OUTPUT);
	
	led_yellow_off();
	led_red_off();
	
	medipix_usart_buffer = usartBufferInitialize(&MPX_USART, MPX_USART_BAUDRATE, MPX_USART_BUFFERSIZE);
	
	#endif // MEDIPIX_BOARD == 1
}