/*
 * mainTask.h
 *
 * Created: 11.10.2014 20:19:18
 *  Author: Tomas Baca
 */ 

#include "system.h"

#ifndef MAINTASK_H_
#define MAINTASK_H_

int sendFreeHeapSpace(csp_packet_t * inPacket);
int echoBack(csp_packet_t * inPacket);

void mainTask(void *p);

#endif /* MAINTASK_H_ */