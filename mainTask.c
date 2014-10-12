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

/* -------------------------------------------------------------------- */
/*	Reply the heap free space in human readable form					*/
/* -------------------------------------------------------------------- */
int sendFreeHeapSpace(csp_packet_t * inPacket) {

	/* Get packet buffer for data */

	/* Connect to host HOST, port PORT with regular UDP-like protocol and 1000 ms timeout */
	/* priority, destination, destination port, timeout, opts */
	csp_conn_t *conn = csp_connect(CSP_PRIO_NORM, inPacket->id.src, 15, 1000, CSP_O_CRC32);
	if (conn == NULL) {
		/* Connect failed */
		printf("Connection failed\\n");
		return -1;
	}
	
	char msg[20];
	itoa(xPortGetFreeHeapSize(), msg, 10);
	
	/* Copy message to packet */
	strcpy(outcomingPacket->data, msg);
	
	outcomingPacket->length = strlen(msg);

	/* Send packet */
	if (!csp_send(conn, outcomingPacket, 1000)) {
		
		/* Send failed */
		printf("Send failed\\n");
	} else {
		/* Send succeeded */
	}

	/* Close connection */
	csp_close(conn);

	return 0;
}

/* -------------------------------------------------------------------- */
/*	Reply with some status info message									*/
/* -------------------------------------------------------------------- */
int systemStatus(csp_packet_t * inPacket) {

	/* Get packet buffer for data */

	/* Connect to host HOST, port PORT with regular UDP-like protocol and 1000 ms timeout */
	/* priority, destination, destination port, timeout, opts */
	csp_conn_t *conn = csp_connect(CSP_PRIO_NORM, inPacket->id.src, 15, 1000, CSP_O_CRC32);
	if (conn == NULL) {
		/* Connect failed */
		printf("Connection failed\\n");
		return -1;
	}
	
	// the info message
	char msg[] = "\n\rMedipix board\n\rSoftware v1.0\n\r";
	strcpy(outcomingPacket->data, msg);
	outcomingPacket->length = strlen(msg);

	/* Send packet */
	if (!csp_send(conn, outcomingPacket, 1000)) {
		
		/* Send failed */
		printf("Send failed\\n");
		} else {
			/* Send succeeded */
		}

	/* Close connection */
	csp_close(conn);

	return 0;
}

/* -------------------------------------------------------------------- */
/*	Sends back the incoming packet										*/
/* -------------------------------------------------------------------- */
int echoBack(csp_packet_t * inPacket) {

	/* Get packet buffer for data */

	/* Connect to host HOST, port PORT with regular UDP-like protocol and 1000 ms timeout */
	/* priority, destination, destination port, timeout, opts */
	csp_conn_t *conn = csp_connect(CSP_PRIO_NORM, inPacket->id.src, inPacket->id.dport, 1000, CSP_O_CRC32);
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
		/* Send succeeded */
	}

	/* Close connection */
	csp_close(conn);

	return 0;
}

/* -------------------------------------------------------------------- */
/*	The main task														*/
/* -------------------------------------------------------------------- */
void mainTask(void *p) {
	
	// packet used to handle incoming communication
	outcomingPacket = csp_buffer_get(CSP_PACKET_SIZE);
	
	// infinite while loop of the program 
	while (1) {
		
		// the queue between cspTask and the main task
		xQueueReceive(xCSPEventQueue, &xReceivedEvent, portMAX_DELAY);
		
		switch( xReceivedEvent.eEventType )
		{
			// Reply with RTOS free heap space
			// replies in Human Readable form
			case freeHeapEvent :
			
				sendFreeHeapSpace(xReceivedEvent.pvData);
			
			break;
			
			// Echo back the whole packet
			// incoming port => outcoming
			case echoBackEvent :
			
				echoBack(xReceivedEvent.pvData);
			
			break;
			
			// sends the info about the system
			case statusEvent :
			
				systemStatus(xReceivedEvent.pvData);
			
			break;

			default :
			/* Should not get here. */
			break;
		}
	}
}