/*
 * main.c
 *
 * Created: 24.8.2014 15:10:04
 *  Author: klaxalk
 */ 


#include <avr/io.h>
#include <util/delay.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "sysclk.h"
#include "usart_driver_RTOS.h"
#include "ioport.h"

#include <string.h>
#include "csp.h"
#include "csp_thread.h"
#include "csp_clock.h"
#include "csp_if_i2c.h"

/** Example defines */
#define MY_ADDRESS  1			// Address of local CSP node
#define MY_PORT		10			// Port to send test traffic to

/*! Defining an example slave address. */
#define SLAVE_ADDRESS_1    0x56
#define SLAVE_ADDRESS_2    0

#define BAUDRATE	100000

// YELLOW LED on PORTA-0
#define	YELLOW	IOPORT_CREATE_PIN(PORTA, 0)

// RED LED on PORTA-4
#define	RED	IOPORT_CREATE_PIN(PORTA, 4)

// define the usart port
#define PC_USART	 USARTC0

/*! Defining an example slave address. */
#define SLAVE_ADDRESS    0x55

csp_packet_t *packet;

// handles communication between csp and commtask
volatile int8_t sendStatus = 0;

void blink1(void *p) {
	
	while (1) {
				
		ioport_toggle_pin_level(RED);

        vTaskDelay(1000);
	}
}

void uartLoopBack(void *p) {
	
	unsigned char znak;
	
	UsartBuffer * pc_usart_buffer = usartBufferInitialize(&PC_USART, BAUD19200, 128);
	usartBufferPutString(pc_usart_buffer, "\n\n\rXMEGA ready", 10);
	
	while (1) {
		
		if (usartBufferGetByte(pc_usart_buffer, &znak, 0)) {
			
			usartBufferPutByte(pc_usart_buffer, znak, 10);
		}
	}
}

/* User functions required */
void clock_get_time(csp_timestamp_t * time) {
	
	
}

extern void clock_set_time(csp_timestamp_t * time) {
	
	
}

int send_packet() {

	/* Get packet buffer for data */

	/* Connect to host HOST, port PORT with regular UDP-like protocol and 1000 ms timeout */
	csp_conn_t *conn = csp_connect(CSP_PRIO_NORM, 1, 15, 1000, CSP_O_NONE);
	if (conn == NULL) {
		/* Connect failed */
		printf("Connection failed\\n");
		return -1;
	}
	
	char * msg = "MMM";
	
	/* Copy message to packet */
	strcpy(packet->data, msg);

	/* Set packet length */
	packet->length = strlen(msg);

	/* Send packet */
	if (!csp_send(conn, packet, 1000)) {
		
		/* Send failed */
		printf("Send failed\\n");
	} else {
	}

	/* Close connection */
	csp_close(conn);

	return 0;
}

void csp_receive(void *p) {
	
	/* Create socket without any socket options */
	csp_socket_t *sock = csp_socket(CSP_SO_NONE);

	/* Bind all ports to socket */
	csp_bind(sock, CSP_ANY);

	/* Create 10 connections backlog queue */
	csp_listen(sock, 10);

	/* Pointer to current connection and packet */
	csp_conn_t *conn;
	csp_packet_t *packet;

	/* Process incoming connections */
	while (1) {
		
		/* Wait for connection, 10000 ms timeout */
		if ((conn = csp_accept(sock, 10000)) == NULL)
		continue;

		/* Read packets. Timout is 1000 ms */
		while ((packet = csp_read(conn, 1000)) != NULL) {
			switch (csp_conn_dport(conn)) {
				
				/* Port 15 packet received */
				case 15:
				
					sendStatus = 1;
					
				/* Process packet here */	
				default:
				
				/* Let the service handler reply pings, buffer use, etc. */
				csp_service_handler(conn, packet);
				break;
			}
		}

		/* Close current connection, and handle next */
		csp_close(conn);
	}
}

void communication_task(void *p) {
	
	while (1) {
	
		if (sendStatus == 1) {
								
			ioport_toggle_pin_level(YELLOW);
			send_packet();
			sendStatus = 0;
			
		} else if (sendStatus == 2) {
				
			send_packet();
		}
	}
}

int main(void) {

	// prepare the i/o for LEDs
	ioport_init();
	ioport_set_pin_dir(RED, IOPORT_DIR_OUTPUT);
	ioport_set_pin_dir(YELLOW, IOPORT_DIR_OUTPUT);
			
	// clock init & enable system clock to all peripheral modules
	sysclk_init();
	sysclk_enable_module(SYSCLK_PORT_GEN, 0xff);
	sysclk_enable_module(SYSCLK_PORT_A, 0xff);
	sysclk_enable_module(SYSCLK_PORT_B, 0xff);
	sysclk_enable_module(SYSCLK_PORT_C, 0xff);
	sysclk_enable_module(SYSCLK_PORT_D, 0xff);
	sysclk_enable_module(SYSCLK_PORT_E, 0xff);
	sysclk_enable_module(SYSCLK_PORT_F, 0xff);
	
	ioport_set_pin_level(YELLOW, false);
	ioport_set_pin_level(RED, false);
	
	csp_buffer_init(2, 100);
			
	// Init CSP with address MY_ADDRESS
	csp_init(2);
	
	csp_i2c_init(0x55, 0, BAUDRATE);
					
	// Add route to address 1 via i2c with 0x56 i2c address
	csp_route_set(1, &csp_if_i2c, 0x56);		
					
	// Start router task with 500 word stack, OS task priority 1
	csp_route_start_task(500, 2);
	
	packet = csp_buffer_get(50);
				
	// start tasks
	xTaskCreate(blink1, (signed char*) "blink1", 512, NULL, 2, NULL);
	xTaskCreate(csp_receive, (signed char*) "casp_task", 512, NULL, 2, NULL);
	xTaskCreate(communication_task, (signed char*) "commTask", 512, NULL, 2, NULL);
	
	// xTaskCreate(uartLoopBack, (signed char*) "uart1", 1024, NULL, 2, NULL);
	
	vTaskStartScheduler();
	
	return 0;
}