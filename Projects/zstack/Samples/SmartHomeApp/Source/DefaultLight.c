#include "DefaultLight.h"
#include "hal_mcu.h"
#include "Utils.h"

#include "SmartHomeApp.h"


#define LIGHT_TOTAL_NUM 3

static uint16 light_timer[LIGHT_TOTAL_NUM] = {0xFFFF,0xFFFF,0xFFFF};


static void (*DefaultLight_TurnOnLightFunc)(uint16 lightNum);
static void (*DefaultLight_TurnOffLightFunc)(uint16 lightNum);
static void (*DefaultLight_ToggleLightFunc)(uint16 lightNum);
static uint8 (*DefaultLight_LightStatusFunc)(uint16 lightNum);


void DefaultLight_Init(void (*TurnOnFunc)(uint16),void (*TurnOffFunc)(uint16),void (*ToggleFunc)(uint16), uint8 (*LightStatusFunc)(uint16))
{
  DefaultLight_TurnOnLightFunc = TurnOnFunc;
  DefaultLight_TurnOffLightFunc = TurnOffFunc;
  DefaultLight_ToggleLightFunc = ToggleFunc;
  DefaultLight_LightStatusFunc = LightStatusFunc;
}

void DefaultLight_HandleMsg(uint8* msg, uint8* name, uint16 srcAddr)
{
  uint16 lightNum;
  uint16 timeout;
  uint8 name_prefix[64];
  uint8 lightStatus;
  
  printf("DefaultLight_HandleMsg[%s]: %s\n", name, msg);
  
  sprintf((char*)name_prefix, "TurnOn %s", (char*)name);
  
  if (Util_StartWith(msg, name_prefix)) {
    sprintf((char*)name_prefix, "TurnOn %s ", (char*)name);
    if (Util_StartWith(msg, name_prefix)) {
      lightNum = Util_Str2Dec(msg + osal_strlen((char*)name_prefix));
    } else {
      lightNum = 0;
    }
    
    DefaultLight_TurnOnLightFunc(lightNum);
  }
  
  sprintf((char*)name_prefix, "TurnOff %s", (char*)name);
  
  if (Util_StartWith(msg, name_prefix)) {
    
    sprintf((char*)name_prefix, "TurnOff %s ", (char*)name);
    if (Util_StartWith(msg, name_prefix)) {
      lightNum = Util_Str2Dec(msg + osal_strlen((char*)name_prefix));
    } else {
      lightNum = 0;
    }
    
    DefaultLight_TurnOffLightFunc(lightNum);
  }

  sprintf((char*)name_prefix, "Toggle %s", (char*)name);
  
  if (Util_StartWith(msg, name_prefix)) {
    
    sprintf((char*)name_prefix, "Toggle %s ", (char*)name);
    if (Util_StartWith(msg, name_prefix)) {
      lightNum = Util_Str2Dec(msg + osal_strlen((char*)name_prefix));
    } else {
      lightNum = 0;
    }
    
    DefaultLight_ToggleLightFunc(lightNum);
  }
  
  sprintf((char*)name_prefix, "LightStatus %s", (char*)name);
  
  if (Util_StartWith(msg, name_prefix)) {
    
    sprintf((char*)name_prefix, "LightStatus %s ", (char*)name);
    if (Util_StartWith(msg, name_prefix)) {
      lightNum = Util_Str2Dec(msg + osal_strlen((char*)name_prefix));
    } else {
      lightNum = 0;
    }
    
    lightStatus = DefaultLight_LightStatusFunc(lightNum);
    sprintf((char*)name_prefix, "LightStatusResp %s %d", (char*)name, lightStatus);
    //TODO send message
    SmartHome_SendCmdWithAddr(name_prefix, srcAddr);
  }
  
  sprintf((char*)name_prefix, "TurnOffLater %s ", (char*)name);
  
  if (Util_StartWith(msg, name_prefix)) {
    lightNum = Util_Str2Dec(msg + osal_strlen((char*)name_prefix));
    timeout = Util_Str2Dec(msg + osal_strlen((char*)name_prefix) + 2);
    if (lightNum > 2 || timeout > 3000) {
      printf("Invalid args: lightNum=%d, timeout=%d\n");
      return; 
    }
    DefaultLight_TurnOnLightFunc(lightNum);
    light_timer[lightNum] = timeout;
  }
}


// update every second
void DefaultLight_Update(void)
{
  uint16 i;
  for (i = 0; i < LIGHT_TOTAL_NUM; i++) {
    if (light_timer[i] == 0) {
      DefaultLight_TurnOffLightFunc(i);
      light_timer[i] = 0xFFFF;
    } else  if (light_timer[i] == 0xFFFF) {
     // ignore 
    } else {
      light_timer[i]--;
    }
  }
}




