#include "mytype.h"
#include "process.h"
#include "key.h"
#include "powerup.h"
#include "hdmidet.h"

UINT8  hdmi_wkup = 0;
UINT8  hdmi_wkup_cnt = 0;
UINT16 hdmi_cec_det_delay = 0;
UINT16 hdmi_cec_pulse_interval = 0;


void CEC_init(void)
{
	P1MDL &= 0xFC;//P10
	P1MDL |= 0x01;//P10上拉输入
}

void HDMI_CEC_Det(void)
{
	if(hdmi_cec_det_delay>0)
		return;
	if(!HDMI_CEC_DET_PIN)
	{
		if(hdmi_wkup == 0)
		{
			hdmi_wkup = 1;
			if(hdmi_cec_pulse_interval <= 0)
			{
				hdmi_wkup_cnt = 0;
			}
			hdmi_wkup_cnt ++;
			if(hdmi_wkup_cnt > 12)
			{
				hdmi_wkup_cnt = 0;
				hdmi_cec_det_delay = 58;//7s:140
				//PowerOn
				HDMIPower();
			}
		}
	}
	else
	{
		hdmi_wkup = 0;
		hdmi_cec_pulse_interval = 5;
	}
}

void DelayEnableHdmiDet(void)
{
	if(!POWER_ON_OFF)
	{
		if(hdmi_cec_det_delay > 0)
		{
			hdmi_cec_det_delay--;
		}

		if(hdmi_cec_pulse_interval > 0)
		{
			hdmi_cec_pulse_interval --;
		}
	}
}

