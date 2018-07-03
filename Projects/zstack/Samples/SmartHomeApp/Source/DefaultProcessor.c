#include "DefaultProcessor.h"


void DefaultProcessor_HandleKeys(uint8 keys)
{
  printf("DefaultProcessor_HandleKeys : %X\n", keys);
  
}


void DefaultProcessor_HandleMsg(uint8* msg)
{
  
  printf("DefaultProcessor_HandleMsg: %s\n", msg); 
}