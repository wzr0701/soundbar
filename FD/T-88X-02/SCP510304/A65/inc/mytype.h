#ifndef _MYTYPE_H_
#define _MYTYPE_H_

#include "MCU_1007.H"
#include <intrins.h>

#define INT8 	char
#define UINT8 	unsigned char
#define UINT16 	unsigned int

#define sei()	EA = 1
#define cli()	EA = 0


void System_Init(void);

#endif