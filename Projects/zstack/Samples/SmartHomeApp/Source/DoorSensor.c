#include "DoorSensor.h"
#include "SmartHomeApp.h"
#include "hal_key.h"

#define LIGHT0_PIN P1_0
#define LIGHT1_PIN P1_1
#define LIGHT2_PIN P1_4


void DoorSensor_Init()
{
  LIGHT0_PIN = 1;
  LIGHT1_PIN = 1;
  LIGHT2_PIN = 1;
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
    SmartHome_SendCmd("TurnOffLater DoorLight 1 60");
  }
}


void DoorSensor_HandleMsg(uint8* msg, uint16 srcAddr)
{
  
  printf("DoorSensor_HandleMsg: %s\n", msg); 
}