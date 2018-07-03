#ifndef DoorSensor_H
#define DoorSensor_H

#include "OSAL.h"
#include "user_printf.h"

void DoorSensor_HandleKeys(uint8 keys);

void DoorSensor_HandleMsg(uint8* msg);

#endif /* DoorSensor_H*/