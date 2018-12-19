#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "mytype.h"

sbit POWER_EN=P1^0;

typedef struct
{	
	UINT8 OK;
	UINT8 Buff;
	UINT8 LastCount;
}S_IR;

extern S_IR g_Ir;

#define CODE_USER			 0x80//0x00
#define CODE_ONOFF 			0x0A //开关

void Time1_Close(void);
void BT_Send_Cmd(void);

#endif