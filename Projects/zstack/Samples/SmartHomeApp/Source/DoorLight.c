#include "DoorLight.h"


void DoorLight_HandleKeys(uint8 keys)
{
  printf("DoorLight_HandleKeys : %X\n", keys);
  
}


void DoorLight_HandleMsg(uint8* msg)
{
  
  printf("DoorLight_HandleMsg: %s\n", msg); 
}