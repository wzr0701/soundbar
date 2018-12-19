#include "mytype.h"
#include "process.h"
#define Imax 	30000//15ms
#define Imin 	17400//8.7ms
#define Inum1 	3200//1.6ms
#define Inum2 	1520//760us
#define Inum3 	6400//3.2ms

void INT(void) interrupt 0
{//上升沿中断
UINT16 Tc; 
static UINT8 m=0,start_ok=0;
static UINT8 IrBuff[4]; 

	EINTCS0 &= 0xfe;

	Tc = TH0*256 + TL0;  //1/(4M/2分频) * Tc = 0.5us * Tc
	TR0 = 0;
	TH0 = 0;
	TL0 = 0;
	TR0 = 1;	
	if ((Tc>Imin) && (Tc<Imax)) //引导码  
	{
		TR1 = 0;
		TF1 = 0;
		TH1 = 0x85;    // 1/(4M/32分频) * (65535 - 34285) = 250ms
		TL1 = 0xed;
		TR1 = 1;
		m = 0;
		start_ok = 1;
		g_Ir.LastCount ++;
		return;
	}
	if (start_ok == 1)
	{
		if ((Tc>Inum1) && (Tc<Inum3))//1
		{//1.6ms< <3.2ms
			IrBuff[m/8] = IrBuff[m/8]>>1 | 0x80;
			m ++; 
		}
		else if ((Tc>Inum2) && (Tc<Inum1))//0
		{//760us< <1.6ms
			IrBuff[m/8] >>= 1;
			m ++;	
		}
		else
		{
		 m = 0;	
		 start_ok = 0;
		}
	}
	if (m == 32)
	{
		m = 0;
		start_ok = 0;
		if ((IrBuff[2] == ~IrBuff[3]) && (IrBuff[0] == ~IrBuff[1]) && (IrBuff[0] == CODE_USER) && (IrBuff[2] == CODE_ONOFF))
		{
			g_Ir.OK = 1;
			g_Ir.Buff = IrBuff[2]; 
		}
		else	 
		{
			g_Ir.OK = 0;
			g_Ir.Buff = 0;
		}	
	}
} 