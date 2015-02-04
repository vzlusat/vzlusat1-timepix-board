/*
 * medipixTask.h
 *
 * Created: 2.11.2014 19:01:51
 *  Author: Tomas Baca
 */ 

#ifndef MEDIPIXTASK_H_
#define MEDIPIXTASK_H_

enum medipixAction_t {PWR_ON, PWR_OFF, PWR_TOGGLE, MEASURE};

typedef struct {
	
	enum medipixAction_t action;
} medipixActionMessage_t;

void medipixTask(void *p);

#endif /* MEDIPIXTASK_H_ */