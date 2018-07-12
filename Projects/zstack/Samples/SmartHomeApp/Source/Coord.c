#include "Coord.h"
#include "hal_mcu.h"
#include "Utils.h"



void Coord_Init()
{ 
  
}

void Coord_HandleKeys(uint8 keys)
{
  printf("Coord_HandleKeys : %X\n", keys);
 
}


void Coord_HandleMsg(uint8* msg, uint16 srcAddr)
{
  printf("Coord_HandleMsg: msg=%s, addr=%04X\n", msg, srcAddr);
}


// update every second
void Coord_Update(void)
{
  
}




