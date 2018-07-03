#include "DoorSensor.h"


void DoorSensor_HandleKeys(uint8 keys)
{
  printf("DoorSensor_HandleKeys : %X\n", keys);
  
}


void DoorSensor_HandleMsg(uint8* msg)
{
  
  printf("DoorSensor_HandleMsg: %s\n", msg); 
}