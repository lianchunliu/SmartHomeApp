#include "DoorSensor.h"
#include "SmartHomeApp.h"
#include "hal_key.h"


void DoorSensor_Init()
{
  
}

void DoorSensor_Update()
{
  
}

void DoorSensor_HandleKeys(uint8 keys)
{
  printf("DoorSensor_HandleKeys : %X\n", keys);
  
  if (keys & HAL_KEY_SW_4) {
    SmartHome_SendCmd("TurnOffLater DoorLight 0 60");
  }
  
  if (keys & HAL_KEY_SW_5) {
    SmartHome_SendCmd("TurnOffLater DoorLight 1 30");
  }
}


void DoorSensor_HandleMsg(uint8* msg)
{
  
  printf("DoorSensor_HandleMsg: %s\n", msg); 
}