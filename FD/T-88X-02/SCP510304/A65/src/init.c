#include "mytype.h"
#include "key.h"
#include "powerup.h"
#include "hdmidet.h"
void Port_Init(void)
{
	P0MDL = 0xa9;
	P0MDH = 0xaa;//做推挽输出
	P0 = 0x01;

	P1MDL = 0x28;//P10悬空输入	P11推挽输出	P12推挽输出  P13悬空输入
	P1MDH = 0xaa;//P14-15推挽输出
	P1 = 0x02;    //P1.0 低 P1.1高

	P2MDL = 0xaa;//推挽输出
	P2 = 0x00;

	P3MDL = 0x02;//推挽输出
	P3 = 0x00;

	Power1_2V(0);
	Power3_3V(1);
	//Power14V(0);
	Reset(0);
}

void Timer_Init(void)
{
  TMOD = 0x11;	//T0 16位工作模式
	IE = 0;		//T0 禁止中断
	TH0 = 0;
	TL0 = 0;
	TH1 = 0x85;
	TL1 = 0xed;
	xTIMPRS =0x40;//T0 2分频    T1 32分频
	TR0 = 1;		//启动T0
	TR1 = 0;
}

void INT_Init(void)
{
	TCON |= 0x01;//高电平设置中断
	EINTCS0 = 0x10;
	EX0 = 1;//允许INT0中断
}
void CLR_WDT()
{
	WDTCLR0 = 0x00;
	WDTCLR1 = 0x00;
	WDTCLR0 = 0x53;
	WDTCLR1 = 0xAC;
}
void INIT_WDT()
{

	WPKEY = 0x37;
	xSYSCFG = 0x00; //STOP ?￡ê???2?ó???D?mcu
	WDTCON = 0xFF; //RCL128??,????


}
void System_Init(void)
{
	RSTSRC = 0;		// 清除各个复位标志
	xSTOPCFG = 0x00;// STOP模式下,低压复位由LVREN控制（LVREN上电默认为1）
	//xSYSCFG = 0x4e;//00001110复位脚做开漏输出、TR外置、STP模式下WDT不能唤醒系统、高频晶振增益最小、预留上拉、内部RCL始终打开
	xSYSCFG = 0x0C; //BIT6 TREN需要关闭，否则会影响P30	 IO下拉
	_nop_();
	_nop_();
	WPKEY   = 0x37;//写保护控制寄存器
	MCKSET  = 0x0b;//00001011外部晶振关闭、P1.0 P1.1作普通IO口、时钟选择RCH、MCLK为SYSCLK(不分频)
	xIOMUX0 = 0x00;//P3.0 P1.3做普通IO口

	Port_Init();
	key_init();
	CEC_init();
	Timer_Init();
	INT_Init();
	//INIT_WDT();
	sei();
}