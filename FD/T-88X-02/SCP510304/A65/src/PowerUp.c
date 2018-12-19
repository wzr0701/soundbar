#include "PowerUp.h"
#include "process.h"
#include "hdmidet.h"
#include <string.h>
UINT8 ENABLE_POWER_ON = 0;
UINT8 ENABLE_POWER_OFF = 0;
UINT8 POWER_ON_OFF = 0; //�������߹ػ���־ 1 ���� 0 �ػ�
UINT16 DelayOFF = 0; //��⵽��������ʱ��������ʱ�����ػ�����Ŀ���ǵ�soundbar ����ػ����񣬷������PO��
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
	{//�Ѿ��ػ��������ڿ���
		if(DelayOFF)
		{//�չػ��������β���
			return;
		}
		DelayON = 100;//����ʱ��ʱһ��ʱ�䣬�ػ�������ʱ����Ч
		ENABLE_POWER_ON = 1; //����ʹ��
		ENABLE_POWER_OFF = 0; //�ڿ���ʱ���˳��ػ�
		//PowerLED(1);
		step = 0;
		cnt = 0;
	}
}
void IrPower(void)
{
	if(g_Ir.OK)
	{//�����н���
		memset((char*)&g_Ir,0,sizeof(g_Ir));
		if(POWER_ON_OFF)
		{//�Ѿ����������ڹػ�
			if(DelayON>0)
			{//�տ������ã�������ô��ػ�
				return;
			}
			DelayOFF = DELAY_OFF_TIME; //��ʱ�ػ�
			ENABLE_POWER_ON = 0; //����ڿ������˳�
			ENABLE_POWER_OFF = 1; //�ػ�
			//PowerLED(0);
			step = 0;
			cnt = 0;
		}
		else
		{//�Ѿ��ػ��������ڿ���
			if(DelayOFF)
			{//�չػ��������β���
				return;
			}
			DelayON = 100;//����ʱ��ʱһ��ʱ�䣬�ػ�������ʱ����Ч
			ENABLE_POWER_ON = 1; //����ʹ��
			ENABLE_POWER_OFF = 0; //�ڿ���ʱ���˳��ػ�
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
	{//���ǿ�������
		return;
	}
	POWER_ON_OFF = 1;//�Ѿ�����
	switch(step)
	{
		case 0:
		{
			if(++cnt<DELAY_TIME+10)
			{
				//PowerLED(1);
				Power1_2V(1);//�ߵ�ƽ��ͨ
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
			{//�ϵ�ʱ�����
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
	{//���ǿ�������
		return;
	}
	if(DelayOFF>0)
	{//��ʱ�ػ�����soundbar ������ػ�����
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
				Power3_3V(1); //�رյ�Դ
				Reset(0);
			}
			else
			{//�ϵ�ʱ�����
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
			ENABLE_POWER_OFF = 0; //�ػ���� ��ȡ��ʹ�ܱ�־
			POWER_ON_OFF = 0;//�Ѿ��ػ�
			hdmi_cec_det_delay = 58;
			//Power14V(0);
			break;
		}
	}
}
