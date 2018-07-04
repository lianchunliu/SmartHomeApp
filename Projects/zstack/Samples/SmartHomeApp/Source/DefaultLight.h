#ifndef DefaultLight_H
#define DefaultLight_H

#include "OSAL.h"
#include "user_printf.h"


void DefaultLight_HandleMsg(uint8* msg, uint8* name, uint16 srcAddr);

void DefaultLight_Update(void);

void DefaultLight_Init(void (*TurnOnFunc)(uint16),void (*TurnOffFunc)(uint16),void (*ToggleFunc)(uint16), uint8 (*LightStatusFunc)(uint16)); 


#endif /* DefaultLight_H */