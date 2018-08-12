#ifndef RestroomSensor_H
#define RestroomSensor_H

#include "OSAL.h"
#include "user_printf.h"

void RestroomSensor_HandleKeys(uint8 keys);

void RestroomSensor_HandleMsg(uint8* msg, uint16 srcAddr);

void RestroomSensor_Init(void);

void RestroomSensor_Update(void);

#endif /* RestroomSensor_H*/