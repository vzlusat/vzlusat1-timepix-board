/*
 * mainTask.c
 *
 * Created: 11.10.2014 20:18:00
 *  Author: Tomas Baca
 */

#include "mainTask.h"
#include "cspTask.h"

extern csp_packet_t * outcomingPacket;
extern xQueueHandle xCSPEventQueue;

/* The variable used to receive from the queue. */
xCSPStackEvent_t xReceivedEvent;

extern 

int send_packet(csp_packet_t * inPacket) {

	/* Get packet buffer for data */

	/* Connect to host HOST, port PORT with regular UDP-like protocol and 1000 ms timeout */
	/* priority, destination, destination port, timeout, opts */
	csp_conn_t *conn = csp_connect(CSP_PRIO_NORM, inPacket->id.src, 15, 1000, CSP_O_NONE);
	if (conn == NULL) {
		/* Connect failed */
		printf("Connection failed\\n");
		return -1;
	}
	
	char msg[20];
	
	// itoa(xPortGetFreeHeapSize(), msg, 10);
	// itoa(inPacket->length, msg, 10);
	
	/* Copy message to packet */
	// strcpy(outcomingPacket->data, msg);
	outcomingPacket->data[0] = inPacket->data[108];
	outcomingPacket->data[1] = inPacket->data[109];
	
	outcomingPacket->length = 2;

	/* Send packet */
	if (!csp_send(conn, outcomingPacket, 1000)) {
		
		/* Send failed */
		printf("Send failed\\n");
	} else {
		// do something
		
		led_yellow_toggle();
	}

	/* Close connection */
	csp_close(conn);

	return 0;
}

int echoBack(csp_packet_t * inPacket) {

	/* Get packet buffer for data */

	/* Connect to host HOST, port PORT with regular UDP-like protocol and 1000 ms timeout */
	/* priority, destination, destination port, timeout, opts */
	csp_conn_t *conn = csp_connect(CSP_PRIO_NORM, inPacket->id.src, inPacket->id.dport, 1000, CSP_O_NONE);
	if (conn == NULL) {
		/* Connect failed */
		printf("Connection failed\\n");
		return -1;
	}
		
	/* Send the same data back */
	strcpy(outcomingPacket->data, inPacket->data);
	outcomingPacket->length = inPacket->length;

	/* Send packet */
	if (!csp_send(conn, outcomingPacket, 1000)) {
		
		/* Send failed */
		printf("Send failed\\n");
	} else {
		// do something
		led_red_toggle();
	}

	/* Close connection */
	csp_close(conn);

	return 0;
}

void mainTask(void *p) {
	
	// packet used to handle incoming comunication
	outcomingPacket = csp_buffer_get(256);
	
	// infinite while loop of the program 
	while (1) {
		
		xQueueReceive(xCSPEventQueue, &xReceivedEvent, portMAX_DELAY);
		
		switch( xReceivedEvent.eEventType )
		{
			case freeHeapEvent :
			
				send_packet(xReceivedEvent.pvData);
			
			break;
			
			case echoBackEvent :
			
				echoBack(xReceivedEvent.pvData);
			
			break;

			default :
			/* Should not get here. */
			break;
		}
	}
}