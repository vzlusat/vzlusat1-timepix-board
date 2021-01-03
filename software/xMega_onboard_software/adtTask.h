/*
 * adtTask.h
 *
 * Created: 6.7.2015 13:49:14
 *  Author: klaxalk
 */ 

#ifndef ADTTASK_H_
#define ADTTASK_H_

#include "system.h"

volatile int8_t adtTemp;
volatile int8_t adtTemp_max;
volatile int8_t adtTemp_min;

void adtTask();

#endif /* ADTTASK_H_ */