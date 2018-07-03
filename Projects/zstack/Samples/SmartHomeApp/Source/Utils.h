#ifndef SmartHome_Utils_H
#define SmartHome_Utils_H

#include "OSAL.h"


const uint8* Util_ReadName(void);
void Util_WriteName(uint8 *buf, uint16 len);

uint8 Util_StartWith(const uint8* src, const uint8* dst);

uint8 Util_StrEqual(const uint8* str1, const uint8* str2);

void NameAddrCache_put(uint8* name, uint16 addr);
uint16 NameAddrCache_getAddrFromCmd(uint8* cmd);

uint16 Util_Str2Uint16(uint8* str);

uint16 NameAddrCache_getAddrFromCmd(uint8* cmd);

uint8* Util_GetNameFromCmd(uint8* cmd);

#endif /* SmartHome_Utils_H*/