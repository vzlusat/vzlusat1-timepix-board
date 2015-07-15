/*
 * cspTask.c
 *
 * Created: 11.10.2014 20:33:09
 *  Author: Tomas Baca
 */ 

#include "cspTask.h"
#include "mainTask.h"
#include "dkHandler.h"

/* -------------------------------------------------------------------- */
/*	Task that handles CSP incoming packets								*/
/* -------------------------------------------------------------------- */
void cspTask(void *p) {
	
	/* Create socket without any socket options */
	csp_socket_t * sock = csp_socket(CSP_SO_NONE);

	/* Bind all ports to socket */
	csp_bind(sock, CSP_ANY);

	/* Create 10 connections backlog queue */
	csp_listen(sock, 10);

	/* Pointer to current connection and packet */
	csp_conn_t * conn;
	csp_packet_t * packet;
	
	xCSPEventQueue = xQueueCreate(10, (portBASE_TYPE) sizeof(xCSPStackEvent_t));
	xCSPAckQueue = xQueueCreate(1, 4);
	xCSPTimeQueue = xQueueCreate(1, 4);
	xCSPAttitudeQueue =  xQueueCreate(1, sizeof(adcs_att_t));
	
	int32_t tempInt;
	uint32_t tempUint;
	adcs_att_t attitudeData;
	
	xCSPStackEvent_t * newEvent;

	/* Process incoming connections */
	while (1) {
		
		/* Wait for connection, 10000 ms timeout */
		if ((conn = csp_accept(sock, 10000)) == NULL)
		continue;

		/* Read packets. Timout is 1000 ms */
		while ((packet = csp_read(conn, 10)) != NULL) {
			switch (csp_conn_dport(conn)) {
					
				// direct event
				case 16:
				
					newEvent->eEventType = directEvent;
					newEvent->pvData = packet;
					xQueueSend(xCSPEventQueue, newEvent, 10);	
					
				break;

				// datakeeper saving event
				case 17:
				
					newEvent->eEventType = obcEvent;
					newEvent->pvData = packet;
					xQueueSend(xCSPEventQueue, newEvent, 10);
				
				break;
				
				// ack from DK
				case 18:
					
					tempInt = ((dk_reply_common_t *) packet->data)->err_no;
					xQueueSend(xCSPAckQueue, &tempInt, 10);		
							
				break;
				
				// message with attitude
				case 19:
				
					memcpy(&attitudeData, packet->data, sizeof(adcs_att_t));
					xQueueSend(xCSPAttitudeQueue, &attitudeData, 10);	
				
				break;
				
				// message with time
				case 20:
				
					tempUint = ((csp_cmp_msg_t *) packet->data)->tv_sec;
					xQueueSend(xCSPTimeQueue, &tempUint, 10);		
				
				break;
				
				// message with chunk ID
				case 21:
				
					tempUint = ((dk_reply_info_t *) packet->data)->chunks;
					xQueueSend(xCSPTimeQueue, &tempUint, 10);
				
				break;
				
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