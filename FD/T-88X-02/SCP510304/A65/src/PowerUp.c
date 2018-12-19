#include "PowerUp.h"
#include "process.h"
#include "hdmidet.h"
#include <string.h>
UINT8 ENABLE_POWER_ON = 0;
UINT8 ENABLE_POWER_OFF = 0;
UINT8 POWER_ON_OFF = 0; //开机或者关机标志 1 开机 0 关机
UINT16 DelayOFF = 0; //检测到按键长按时，启动延时触发关机任务，目的是等soundbar 处理关机任务，否则出现PO声
UINT16 DelayON = 0;
UINT8 step = 0;
UINT16 cnt = 0;
UINT8 AutoPowerUpFlag = 1;


void AutoPowerUp(void)
{
	if(AutoPowerUpFlag)
	{
		ENABLE_POWER_ON = 1;
		AutoPowerUpFlag = 0;
	}
}
void HDMIPower(void)
{
	if(!POWER_ON_OFF)
	{//已经关机机，现在开机
		if(DelayOFF)
		{//刚关机丢弃本次操作
			return;
		}
		DelayON = 100;//开机时延时一段时间，关机键按下时才有效
		ENABLE_POWER_ON = 1; //开机使能
		ENABLE_POWER_OFF = 0; //在开机时则退出关机
		//PowerLED(1);
		step = 0;
		cnt = 0;
	}
}
void IrPower(void)
{
	if(g_Ir.OK)
	{//红外有接收
		memset((char*)&g_Ir,0,sizeof(g_Ir));
		if(POWER_ON_OFF)
		{//已经开机，现在关机
			if(DelayON>0)
			{//刚开机不久，不能这么快关机
				return;
			}
			DelayOFF = DELAY_OFF_TIME; //延时关机
			ENABLE_POWER_ON = 0; //如果在开机则退出
			ENABLE_POWER_OFF = 1; //关机
			//PowerLED(0);
			step = 0;
			cnt = 0;
		}
		else
		{//已经关机机，现在开机
			if(DelayOFF)
			{//刚关机丢弃本次操作
				return;
			}
			DelayON = 100;//开机时延时一段时间，关机键按下时才有效
			ENABLE_POWER_ON = 1; //开机使能
			ENABLE_POWER_OFF = 0; //在开机时则退出关机
			//PowerLED(1);
			step = 0;
			cnt = 0;
		}
	}
}
void DelayEnablePowerOFF(void)
{
	if(DelayON>0)
	{
		DelayON--;
	}
}
void ProcessPowerUp(void)
{
	if(!ENABLE_POWER_ON)
	{//不是开机返回
		return;
	}
	POWER_ON_OFF = 1;//已经开机
	switch(step)
	{
		case 0:
		{
			if(++cnt<DELAY_TIME+10)
			{
				//PowerLED(1);
				Power1_2V(1);//高电平导通
				Power3_3V(1);
				Reset(0);
			}
			else
			{
				cnt = 0;
				step = 1;
			}
			break;
		}
		case 1:
		{
			if(++cnt<DELAY_TIME+11)
			{
				Power3_3V(0);
			}
			else
			{
				cnt = 0;
				step =2;
			}
			break;
		}
		case 2:
		{
			if(++cnt<DELAY_TIME)
			{
				Reset(1);
			}
			else
			{
				cnt = 0;
				step =3;
			}
			break;
		}
		case 3:
		{
			if(++cnt<DELAY_TIME)
			{
				Reset(0);
			}
			else
			{
				cnt = 0;
				step =4;
			}
			break;
		}
		case 4:
		{
			if(++cnt<DELAY_TIME)
			{
				Reset(1);
			}
			else
			{//上电时序完成
				//Power14V(1);
				cnt = 0;
				step =6;

			}
			break;
		}
	}
}
void ProcessPowerDown(void)
{
	if(!ENABLE_POWER_OFF)
	{//不是开机返回
		return;
	}
	if(DelayOFF>0)
	{//延时关机，等soundbar 处理完关机任务
		DelayOFF--;
		return;
	}
	switch(step)
	{
		case 0:
		{
			if(++cnt<DELAY_TIME)
			{
				//PowerLED(0);
				Power3_3V(1); //关闭电源
				Reset(0);
			}
			else
			{//上电时序完成
				cnt = 0;
				step =1;
			}
			break;
		}
		case 1:
		{
			Power1_2V(0);
			step =2;
			break;
		}
		case 2:
		{
			step =3;
			ENABLE_POWER_OFF = 0; //关机完成 ，取消使能标志
			POWER_ON_OFF = 0;//已经关机
			hdmi_cec_det_delay = 58;
			//Power14V(0);
			break;
		}
	}
}
