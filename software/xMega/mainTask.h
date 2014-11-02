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

void mainTask(void *p);

#endif /* MAINTASK_H_ */