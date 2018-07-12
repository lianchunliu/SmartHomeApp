#include "Utils.h"
#include "OSAL_Nv.h"
#include "ZComDef.h"

#include "user_printf.h"

#define MY_NAME_NV_LEN 64
#define MY_NAME_NV_ID 0x2701

static uint8* application_name;

const uint8* Util_ReadName(void)
{
  uint16 len;
  uint8* myBuf;
  
  if (1) {
    return "NoName"; 
  }
  if (application_name != NULL) {
    return application_name;
  }
  
  len = osal_nv_item_len(MY_NAME_NV_ID);
  if (len > MY_NAME_NV_LEN || len == 0) {
    return "Unkown";
  }
  
  myBuf = osal_mem_alloc(len+1);
  //osal_nv_item_init(MY_NAME_NV_ID, len, NULL);
  if (osal_nv_read(MY_NAME_NV_ID, 0, len, myBuf ) == ZSUCCESS) {
    myBuf[len] = 0;
    application_name = myBuf;
    return application_name;
  } else {
    osal_mem_free(myBuf);
    return "Unkown";
  }
}

void Util_WriteName(uint8 *buf, uint16 len)
{
  osal_nv_item_init(MY_NAME_NV_ID, len, NULL);
  osal_nv_write(MY_NAME_NV_ID, 0, len, buf);
  if (application_name != NULL) {
    osal_mem_free(application_name);
  }
  application_name = NULL;
}


uint8 Util_StartWith(const uint8* str1, const uint8* str2)
{
  while (*str2) {
    if (*str1 != *str2) {
      return 0;
    }
    str1++;
    str2++;
  }
  return 1;
}

uint8 Util_StrEqual(const uint8* str1, const uint8* str2)
{
  while (*str2 && *str1) {
    if (*str1 != *str2) {
      return 0;
    }
    str1++;
    str2++;
  }
  if (*str1 == *str2) {
    return 1;
  } else {
    return 0;
  }
}


typedef struct
{
  uint16 addr;
  uint8* name;
} NameAddrCacheData_t;

#define NAME_ADDR_CACHE_SIZE 5

NameAddrCacheData_t* name_addr_cache[NAME_ADDR_CACHE_SIZE];
uint16 name_addr_cache_idx = 0;


void NameAddrCache_delete(uint16 addr)
{
  uint16 i;
  NameAddrCacheData_t* data;
  
  // find old match data, remove it
  for (i = 0; i < NAME_ADDR_CACHE_SIZE; i++) {
     data = name_addr_cache[i];
     if (data == NULL)
       continue;
     if (data->addr == addr) {
       osal_mem_free(data);
       name_addr_cache[i] = NULL;
     }
  }
  
}

void NameAddrCache_put(uint8* name, uint16 addr)
{
  uint16 i;
  NameAddrCacheData_t* data;
  
  // find old match data, remove it
  for (i = 0; i < NAME_ADDR_CACHE_SIZE; i++) {
     data = name_addr_cache[i];
     if (data == NULL)
       continue;
     if (data->addr == addr || Util_StrEqual(name, data->name)) {
       osal_mem_free(data);
       name_addr_cache[i] = NULL;
     }
  }
  
  // remove current point data
  data = name_addr_cache[name_addr_cache_idx];
  if (data != NULL) {
    osal_mem_free(data);
    name_addr_cache[name_addr_cache_idx] = NULL;
  }
  
  data = osal_mem_alloc(2 + 2 + osal_strlen((char*)name)+1);
  data->addr = addr;
  data->name = (uint8*)data + 4;
  osal_memcpy(data->name, name, osal_strlen((char*)name)+1);
  
  name_addr_cache[name_addr_cache_idx] = data;
  name_addr_cache_idx++;
  name_addr_cache_idx = name_addr_cache_idx % NAME_ADDR_CACHE_SIZE;
  
  
  // debug only
//  printf("debug idx=%d\n", name_addr_cache_idx);
//  for (i = 0; i < NAME_ADDR_CACHE_SIZE; i++) {
//     data = name_addr_cache[i];
//     if (data == NULL) {
//        printf("[%d] NULL\n", i);
//       continue;
//     }
//     
//     printf("[%d] %04X %s\n", i, data->addr, data->name);
//  }
}

uint8* Util_GetNameFromCmd(uint8* cmd)
{
  uint16 name_size;
  uint8 *start_p, *end_p;
  
  start_p = cmd;
  while (*start_p != ' ')
    start_p++;
  
  if (*start_p != ' ')
    return NULL;
  
  start_p++;
  
  end_p = start_p;
  
  while (*end_p != ' ' && *end_p)
    end_p++;
  
  if (*end_p != ' ' && *end_p != '\0')
    return NULL;
  
  name_size = end_p - start_p;
  uint8* name = osal_mem_alloc(name_size+1);
  osal_memcpy(name, start_p, name_size);
  name[name_size] = 0;
  
  return name;
}

uint16 NameAddrCache_getAddrFromCmd(uint8* cmd)
{
  uint16 i;
  NameAddrCacheData_t* data;
  uint8* name;
  
  name = Util_GetNameFromCmd(cmd); 
  if (name == NULL)
    return 0xFFFF;
  
  // find match data
  for (i = 0; i < NAME_ADDR_CACHE_SIZE; i++) {
     data = name_addr_cache[i];
     if (data == NULL)
       continue;
     if (Util_StrEqual(name, data->name)) {
       osal_mem_free(name);
       return data->addr;
     }
  }
  
  osal_mem_free(name);
  // not find.
  return 0xFFFF;
}


static uint16 findHexValue(uint8 ch);

uint16 findHexValue(uint8 ch)
{
  switch(ch) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return ch - '0';
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
    return ch - 'A' + 10;
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
    return ch - 'a' + 10;
  }
  return 0;
}


uint16 Util_Str2Uint16(uint8* str)
{
    return findHexValue(str[0])*16*16*16 + findHexValue(str[1])*16*16 + findHexValue(str[2])*16 + findHexValue(str[3]);
}



uint16 Util_Str2Dec(uint8 * str)
{
  uint16 ret = 0;
  while (*str && (*str != ' ')) {
    ret = ret*10 + (*str - '0');
    str++;
  }
  return ret;
}