#include "RestroomSensor.h"
#include "SmartHomeApp.h"
#include "hal_key.h"

#define LIGHT0_PIN P1_0
#define LIGHT1_PIN P1_1
#define LIGHT2_PIN P1_4

#define SENSOR_BUTTON_4 P0_4
#define SENSOR_BUTTON_PRESS 0

void RestroomSensor_Init()
{
  LIGHT0_PIN = 1;
  LIGHT1_PIN = 1;
  LIGHT2_PIN = 1;
}

void RestroomSensor_Update()
{
  static uint16 i = 0;
  i++;
  if (i % 20 == 0) {
    if (SENSOR_BUTTON_4 == SENSOR_BUTTON_PRESS) {
      SmartHome_SendCmd("TurnOffLater RestroomLight 0 60");
    }
  }
}

void RestroomSensor_HandleKeys(uint8 keys)
{
  printf("RestroomSensor_HandleKeys : %X\n", keys);
  
  if (keys & HAL_KEY_SW_4) {
    SmartHome_SendCmd("TurnOffLater RestroomLight 0 60");
  }

}


void RestroomSensor_HandleMsg(uint8* msg, uint16 srcAddr)
{
  
  printf("RestroomSensor_HandleMsg: %s\n", msg); 
}