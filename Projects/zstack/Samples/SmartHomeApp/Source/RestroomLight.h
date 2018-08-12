#ifndef RestroomLight_H
#define RestroomLight_H

#include "OSAL.h"
#include "user_printf.h"

//DoorLight_ProcessEvent();
void RestroomLight_HandleKeys(uint8 keys);

void RestroomLight_HandleMsg(uint8* msg, uint16 srcAddr);

void RestroomLight_Update(void);

void RestroomLight_Init(void);

#endif /* RestroomLight_H*/