#ifndef DoorLight_H
#define DoorLight_H

#include "OSAL.h"
#include "user_printf.h"

//DoorLight_ProcessEvent();
void DoorLight_HandleKeys(uint8 keys);

void DoorLight_HandleMsg(uint8* msg);

void DoorLight_Update(void);

void DoorLight_Init(void);

#endif /* DoorLight_H*/