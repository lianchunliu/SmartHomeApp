#include "DoorLight.h"
#include "hal_mcu.h"
#include "Utils.h"

#define LIGHT_TOTAL_NUM 3
#define LIGHT0_PIN P1_0
#define LIGHT1_PIN P1_1
#define LIGHT2_PIN P1_4

#define LIHGT_ON_STATE 0
#define LIGHT_OFF_STATE 1


static uint16 light_timer[LIGHT_TOTAL_NUM] = {0xFFFF,0xFFFF,0xFFFF};

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

void DoorLight_HandleKeys(uint8 keys)
{
  printf("DoorLight_HandleKeys : %X\n", keys);
  
}


void DoorLight_HandleMsg(uint8* msg)
{
  uint16 lightNum;
  uint16 timeout;
  printf("DoorLight_HandleMsg: %s\n", msg);
  
  if (Util_StartWith(msg, "TurnOn DoorLight")) {
    if (Util_StartWith(msg, "TurnOn DoorLight ")) {
      lightNum = Util_Str2Dec(msg + 17);
    } else {
      lightNum = 0;
    }
    
    turnOnLight(lightNum);
  }
  
  if (Util_StartWith(msg, "TurnOff DoorLight")) {
    if (Util_StartWith(msg, "TurnOff DoorLight ")) {
      lightNum = Util_Str2Dec(msg + 18);
    } else {
      lightNum = 0;
    }
    
    turnOffLight(lightNum);
  }
  
  
  if (Util_StartWith(msg, "TurnOffLater DoorLight ")) {
    lightNum = Util_Str2Dec(msg + 23);
    timeout = Util_Str2Dec(msg + 25);
    if (lightNum > 2 || timeout > 3000) {
      printf("Invalid args: lightNum=%d, timeout=%d\n");
      return; 
    }
    turnOnLight(lightNum);
    light_timer[lightNum] = timeout;
  }
}

void DoorLight_Update(void)
{
  int i;
  for (i = 0; i < LIGHT_TOTAL_NUM; i++) {
    if (light_timer[i] == 0) {
      turnOffLight(i);
      light_timer[i] = 0xFFFF;
    } else  if (light_timer[i] == 0xFFFF) {
     // ignore 
    } else {
      light_timer[i]--;
    }
  }
}




