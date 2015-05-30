/*
 * mainTask.h
 *
 * Created: 11.10.2014 20:19:18
 *  Author: Tomas Baca
 */ 

#ifndef MAINTASK_H_
#define MAINTASK_H_

#include "system.h"

extern xQueueHandle * xCSPEventQueue;
csp_packet_t * outcomingPacket;

unsigned int dest_p;
unsigned int source_p;

void mainTask(void *p);

#endif /* MAINTASK_H_ */