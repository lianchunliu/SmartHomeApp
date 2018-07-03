#ifndef DefaultProcessor_H
#define DefaultProcessor_H

#include "OSAL.h"
#include "user_printf.h"

void DefaultProcessor_HandleKeys(uint8 keys);

void DefaultProcessor_HandleMsg(uint8* msg);

#endif /* DefaultProcessor_H*/