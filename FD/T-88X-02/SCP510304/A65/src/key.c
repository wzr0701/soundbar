#include "key.h"
#include "powerup.h"
#include "hdmidet.h"
KeyForm keyattr;
SysRun run;



/****************************************************************************
 * Name: led_init
 *
 * Description:
 *   按键初始化
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void key_init(void)
{
	P1MDL &= 0x3f;
	P1MDL |= 0x40;//P13上拉输入
}
void RunScan(void)
{
	run.cnt++;
	run.f1ms = 1;
	if(run.cnt%10==0)
	{
		run.f10ms = 1;
	}

	if(run.cnt%20==0)
	{
		run.f20ms = 1;

	}

	if(run.cnt%50==0)
	{
		run.f50ms = 1;
	}

	if(run.cnt%100==0)
	{
		run.f100ms = 1;
	}

	if(run.cnt%1000==0)
	{
		run.f1000ms = 1;
	}

	if(run.cnt==5000)
	{//10s
		run.cnt = 0;
	}

}
void RunHandler(void)
{
	if(run.f1ms)
	{
		run.f1ms = 0;
		IrPower();
	}

	if(run.f10ms)
	{
		run.f10ms = 0;
		KeyScan();
	}

	if(run.f20ms)
	{
		run.f20ms = 0;
		DelayEnablePowerOFF();//定时减去延时间值
		ProcessPowerUp();
		ProcessPowerDown();
	}

	if(run.f50ms)
	{
		run.f50ms = 0;
		DelayEnableHdmiDet();
	}

	if(run.f100ms)
	{
		run.f100ms = 0;
	}

	if(run.f1000ms)
	{
		run.f1000ms = 0;
		//AutoPowerUp();
	}
}
/****************************************************************************
 * Name: led_init
 *
 * Description:
 *   按键解析
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/

void KeyAnalysis(UINT8 key)
{

	UINT8 i;
	for(i=0;i<KEY_NUM;i++)//KEY_MAX_CHN按键通道数
	{
		if(key & (1<<i))
		{// 按下
				if(keyattr.key[i].cnt==0)
				{
					keyattr.key[i].state = 1;
					//keyattr.key[i].ShortPressDownEvent= 1;
				}
		}
		else
		{//弹起
			if(keyattr.key[i].state)
			{
				if(keyattr.key[i].cnt>=1&&keyattr.key[i].cnt<=100)
				{
					keyattr.key[i].ShortPressUpEvent= 1;
				}
				else if(keyattr.key[i].cnt>100&&keyattr.key[i].cnt<=500)
				{

				}
				keyattr.key[i].state = 0;
				keyattr.key[i].cnt = 0;
			}
		}
	}
}
/****************************************************************************
 * Name: led_init
 *
 * Description:
 *   读取按键
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/

UINT8 key_read(void)
{
	UINT8 value = 0;
	UINT8 key = 0;

	if(POWER_KEY)
	{
		value |= 1<<0;
	}
	return value;
}
UINT8 Debounce(UINT8 *key)
{
	static UINT8 step = 0;
	static UINT8 keyold = 0;
	UINT8 value = 0;
	switch(step)
	{
		case 0:
			keyold = key_read();
			step = 1;
			break;
		case 1:
			if(keyold == key_read())
			{
				step = 2;
			}
			else
			{
				keyold = 0;
				step = 0;
			}
			break;
		case 2:
			step = 0;
			if(keyold==key_read())
			{
				*key = keyold;
				keyold = 0;
				value = 1;
			}
			else
			{
				keyold = 0;
			}
			break;
		default:
			break;
	}
	return value;
}
/****************************************************************************
 * Name: led_init
 *
 * Description:
 *   按键扫描
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/

void KeyScan(void)
{
	UINT8 key=0,keytmp=0;
	static UINT8 keyold =0;

	if(Debounce(&key))
	{
		keytmp = key^keyold;
		if(keytmp)
		{
			KeyAnalysis(key);
		}
		keyold = key;
	}
	KeyPressCnt();
	KeyHandler();
}
/****************************************************************************
 * Name: led_init
 *
 * Description:
 *   按键计数
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/

void KeyPressCnt(void)
{
	UINT8 i;
	for(i=0;i<KEY_NUM;i++)//KEY_MAX_CHN按键通道数
	{
		if(keyattr.key[i].state)
		{
			keyattr.key[i].cnt++;
			if(keyattr.key[i].cnt==40)
			{
				keyattr.key[i].LongPressDownEvent = 1;
			}
			else if(keyattr.key[i].cnt >= 201)
			{
				keyattr.key[i].cnt = 201;
			}
		}
		else
		{
			keyattr.key[i].cnt = 0;
		}
	}
}
/****************************************************************************
 * Name: led_init
 *
 * Description:
 *   按键触发事件
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/

void KeyHandler(void)
{
	if(keyattr.key[0].ShortPressUpEvent)
	{
		keyattr.key[0].ShortPressUpEvent = 0;
	}

	if(keyattr.key[0].LongPressDownEvent)
	{//长按开机 关机
		keyattr.key[0].LongPressDownEvent = 0;
		if(POWER_ON_OFF)
		{//刚才已经开机了
			if(DelayON)
			{
				return;
			}
			ENABLE_POWER_ON = 0; //如果在开机则退出
			ENABLE_POWER_OFF = 1; //关机使能
			DelayOFF = DELAY_OFF_TIME;// 延时关机

			//PowerLED(0);
			//hdmi_cec_det_delay = 0;
		}
		else
		{//刚才已经关机了
			if(DelayOFF)
			{//刚关机不久，不运行太快触发开机，否则soundbar出现po声
				return;
			}
			ENABLE_POWER_ON = 1; //开机使能
			ENABLE_POWER_OFF = 0; //在开机时则退出关机
			//PowerLED(1);
			//hdmi_cec_det_delay = 1;
		}
		step = 0;
		cnt = 0;
	}
}


































