#include "uart.h"

void UART_delay(void)
{
	TR0 = 0;
	TH0 = 0xff;
	TL0 = 0x50;
	TF0 = 0;
	TR0 = 1;
	while (!TF0);
}

void UART_tx(INT8 dat)//9600波特率  无校验位
{
	INT8 i,tmp;
	
	tmp = dat;
	UART_TX = 0;
	UART_delay();//起始位
	for (i=0; i<8; i++)
	{
		UART_TX = tmp& 0x01;//数据位 
		tmp >>= 1;							
		UART_delay();
	}
	UART_TX = 1;
	UART_delay();//停止位	
}

void UART_Send(const INT8 *pdat)
{
	cli();
	while (*pdat != '\0')
	{
		UART_tx(*pdat ++); 	
	}
	TF0 = 0;
	sei();
}