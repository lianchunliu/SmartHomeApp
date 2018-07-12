#ifndef Coord_H
#define Coord_H

#include "OSAL.h"
#include "user_printf.h"

//DoorLight_ProcessEvent();
void Coord_HandleKeys(uint8 keys);

void Coord_HandleMsg(uint8* msg, uint16 srcAddr);

void Coord_Update(void);

void Coord_Init(void);

#endif /* Coord_H*/