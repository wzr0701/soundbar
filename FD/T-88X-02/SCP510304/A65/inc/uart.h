#ifndef _UART_H_
#define _UART_H_

#include "mytype.h"

sbit UART_TX = P1^1;
//sbit UART_TX = P1^3;

void UART_Send(const INT8 *pdat);

#endif