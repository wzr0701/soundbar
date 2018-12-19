#include "mytype.h"
#include "key.h"
#include "powerup.h"
#include "hdmidet.h"
void Port_Init(void)
{
	P0MDL = 0xa9;
	P0MDH = 0xaa;//���������
	P0 = 0x01;

	P1MDL = 0x28;//P10��������	P11�������	P12�������  P13��������
	P1MDH = 0xaa;//P14-15�������
	P1 = 0x02;    //P1.0 �� P1.1��

	P2MDL = 0xaa;//�������
	P2 = 0x00;

	P3MDL = 0x02;//�������
	P3 = 0x00;

	Power1_2V(0);
	Power3_3V(1);
	//Power14V(0);
	Reset(0);
}

void Timer_Init(void)
{
  TMOD = 0x11;	//T0 16λ����ģʽ
	IE = 0;		//T0 ��ֹ�ж�
	TH0 = 0;
	TL0 = 0;
	TH1 = 0x85;
	TL1 = 0xed;
	xTIMPRS =0x40;//T0 2��Ƶ    T1 32��Ƶ
	TR0 = 1;		//����T0
	TR1 = 0;
}

void INT_Init(void)
{
	TCON |= 0x01;//�ߵ�ƽ�����ж�
	EINTCS0 = 0x10;
	EX0 = 1;//����INT0�ж�
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
	xSYSCFG = 0x00; //STOP ?�꨺???2?��???D?mcu
	WDTCON = 0xFF; //RCL128??,????


}
void System_Init(void)
{
	RSTSRC = 0;		// ���������λ��־
	xSTOPCFG = 0x00;// STOPģʽ��,��ѹ��λ��LVREN���ƣ�LVREN�ϵ�Ĭ��Ϊ1��
	//xSYSCFG = 0x4e;//00001110��λ������©�����TR���á�STPģʽ��WDT���ܻ���ϵͳ����Ƶ����������С��Ԥ���������ڲ�RCLʼ�մ�
	xSYSCFG = 0x0C; //BIT6 TREN��Ҫ�رգ������Ӱ��P30	 IO����
	_nop_();
	_nop_();
	WPKEY   = 0x37;//д�������ƼĴ���
	MCKSET  = 0x0b;//00001011�ⲿ����رա�P1.0 P1.1����ͨIO�ڡ�ʱ��ѡ��RCH��MCLKΪSYSCLK(����Ƶ)
	xIOMUX0 = 0x00;//P3.0 P1.3����ͨIO��

	Port_Init();
	key_init();
	CEC_init();
	Timer_Init();
	INT_Init();
	//INIT_WDT();
	sei();
}