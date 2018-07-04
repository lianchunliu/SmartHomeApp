#ifndef DefaultProcessor_H
#define DefaultProcessor_H

#include "OSAL.h"
#include "user_printf.h"

void DefaultProcessor_HandleKeys(uint8 keys);

void DefaultProcessor_HandleMsg(uint8* msg, uint16 srcAddr);

void DefaultProcessor_Init(void);
void DefaultProcessor_Update(void);


#endif /* DefaultProcessor_H*/