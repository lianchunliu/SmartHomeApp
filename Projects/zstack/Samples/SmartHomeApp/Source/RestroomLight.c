#include "RestroomLight.h"
#include "hal_mcu.h"
#include "Utils.h"

#include "DefaultLight.h"
#include "hal_key.h"
#include "SmartHomeApp.h"

#define LIGHT_TOTAL_NUM 3
#define LIGHT0_PIN P1_0
#define LIGHT1_PIN P1_1
#define LIGHT2_PIN P1_4

#define LIHGT_ON_STATE 1
#define LIGHT_OFF_STATE 0

static void toggleLight(uint16 lightNum)
{
  printf("toggle light %d\n", lightNum);
  
  switch(lightNum) {
  case 1:
    LIGHT1_PIN ^= 1;
    break;
  case 2:
    LIGHT2_PIN ^= 1;
    break;
  default:
    LIGHT0_PIN ^= 1;
  }
}

static uint8 lightStatus(uint16 lightNum)
{
  
  uint8 ret = 0;
  switch(lightNum) {
  case 1:
    ret = (LIGHT1_PIN == LIHGT_ON_STATE);
    break;
  case 2:
    ret = (LIGHT2_PIN == LIHGT_ON_STATE);
    break;
  default:
    ret = (LIGHT0_PIN == LIHGT_ON_STATE);
  }
  printf("lightStatus %d ret=%d\n", lightNum, ret);
  return ret;
}


static void turnOnLight(uint16 lightNum)
{
  printf("TurnOn light %d\n", lightNum);
  
  switch(lightNum) {
  case 1:
    LIGHT1_PIN = LIHGT_ON_STATE;
    break;
  case 2:
    LIGHT2_PIN = LIHGT_ON_STATE;
    break;
  default:
    LIGHT0_PIN = LIHGT_ON_STATE;
  }
  
}

static void turnOffLight(uint16 lightNum)
{
  printf("TurnOff light %d\n", lightNum);
  switch(lightNum) {
  case 1:
    LIGHT1_PIN = LIGHT_OFF_STATE;
    break;
  case 2:
    LIGHT2_PIN = LIGHT_OFF_STATE;
    break;
  default:
    LIGHT0_PIN = LIGHT_OFF_STATE;
  }
  
}

void RestroomLight_Init()
{ 
  DefaultLight_Init(&turnOnLight, &turnOffLight, &toggleLight, &lightStatus);
}

void RestroomLight_HandleKeys(uint8 keys)
{
  printf("RestroomLight_HandleKeys : %X\n", keys);
  
  if (keys & HAL_KEY_SW_4) {
    SmartHome_SendCmd("TurnOffLater DoorSensor 0 60");
  }
  
  
}


void RestroomLight_HandleMsg(uint8* msg, uint16 srcAddr)
{
  DefaultLight_HandleMsg(msg, "RestroomLight", srcAddr);
}


// update every second
void RestroomLight_Update(void)
{
  DefaultLight_Update();
}




