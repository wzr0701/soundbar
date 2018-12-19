#include <nuttx/config.h>
#include <nuttx/input.h>
#include <nuttx/module.h>
#include <pthread.h>
#include <stdio.h>
#include "sl_ui_cmd.h"
#include "sl_ui_display.h"
#include "ht1633.h"
#include <pthread.h>
#include <string.h>
#include <sys/ioctl.h>
#include "sl_ui_io_led.h"
#include <zhuque_bsp_gpio.h>
#include <silan_gpio.h>
#include "silan_resources.h"
#include "silan_addrspace.h"

int hdmisend_count = 0;
bool hdmi_cec_online=0;
extern  WDOG_ID wdtimer_pa_station_detect;
extern  WDOG_ID wdtimer_enter_testmode_detect;
extern  WDOG_ID wdtimer_change_mode_unmute;
extern  WDOG_ID wdtimer_hdmion_send;

int enter_count = 0;
int playkey_count = 0;
int prekey_count = 0;
int tre_bass_cnt = 100;

bool test_mode_flag = false;

extern int unmute_count;

extern bool enter_tre_set;
extern bool enter_bass_set;
extern int auto_input_cnt;


/****************************************************************************
 * Name: padmux_init
 *
 * Description:
 *    pad功能设置
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
 void padmux_init(void)
{
	int value=0xffff;
#if 1
	pa_mute_ctrl(true);

	(*(volatile unsigned int  *)(0xbe0a0014)) = 10000;
	(*(volatile unsigned int  *)(0xbe0a0018)) = 10000;

	REG32(KSEG1(SILAN_PADMUX_CTRL))=0;
	REG32(KSEG1(SILAN_PADMUX_CTRL2))=0;
	value = REG32(KSEG1(SILAN_PADMUX_CTRL));
	printf("SILAN_PADMUX_CTRL----int=0x%x\n", value);
	value = REG32(KSEG1(SILAN_PADMUX_CTRL2));
	printf("SILAN_PADMUX_CTRL2----int=0x%x\n", value);

	/////////////////////////////////////////////////////////
	#if 1
	value = REG32(KSEG1(SILAN_PADMUX_CTRL));
	value &= ~(1 << SILAN_PADMUX_SDMMC);
	REG32(KSEG1(SILAN_PADMUX_CTRL)) = value;

	zhuque_bsp_gpio_set_mode(26, GPIO_OUT, PULLING_NONE);
	zhuque_bsp_gpio_set_value(26, GPIO_VALUE_LOW);
	#endif

	///////////////////////////////////////////////////////

	value = REG32(KSEG1(SILAN_PADMUX_CTRL));
	value |= (1 << SILAN_PADMUX_UART2);

	/////////////////////////////////////////
	//IIS-------IN
	value |= (1 << SILAN_PADMUX_IISADC);
	value |= (1 << SILAN_PADMUX_IISADC_FD0);
	//value &=~ (1 << SILAN_PADMUX_IISADC_FD1);
	// value &= ~(1 << SILAN_PADMUX_IISADC_FD2);

	///II2----OUT
	value |= (1 << SILAN_PADMUX_IISDAC_MCLK);
	value |= (1 << SILAN_PADMUX_IISDAC);
	value |= (1 << SILAN_PADMUX_IISDAC_FD0);

	////spdif
	value |= (1 << SILAN_PADMUX_SPDIF);
	value |= (1 << SILAN_PADMUX_SPDIF_IN0);
	value |= (1 << SILAN_PADMUX_SPDIF_IN1);
	value |= (1 << SILAN_PADMUX_SPDIF_IN2);

	value |= (1 << SILAN_PADMUX_I2C2);///////hdmi
	value |= (1 << SILAN_PADMUX_UART3_1);

	REG32(KSEG1(SILAN_PADMUX_CTRL)) = value;

	///////////////////////////////////////////////////////

	///////////////////////////////////////////////////////
	value = REG32(KSEG1(SILAN_PADMUX_CTRL2));

	value |= (1 << SILAN_PADMUX2_IISADC_FD1_CH2);
	value |= (1 << SILAN_PADMUX2_IISADC_FD2_CH2);

	REG32(KSEG1(SILAN_PADMUX_CTRL2)) = value;
	//////////////////////////////////////////////////////

	//////////////////////////////////////////////////////
	value = REG32(KSEG1(SILAN_PADMUX_CTRL));
	printf("SILAN_PADMUX_CTRL=0x%x\n", value);
	value = REG32(KSEG1(SILAN_PADMUX_CTRL2));
	printf("SILAN_PADMUX_CTRL2=0x%x\n", value);

#endif

}


/********************************************************




********************************************************/
void io_early_set(void)
{
    padmux_init();
    pa_mute_ctrl(true);
}



/****************************************************************************
 * Name: pa_mute_ctrl
 *
 * Description:
 *    功放控制
 *
 * Parameters:
 *    en 1-使能，0-禁用
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void pa_mute_ctrl(bool mute)
{
    const int pin = PA_MUTE_PIN;
    zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PUSH_PULL);

#if(PA_MUTE_HIGH==1)
    zhuque_bsp_gpio_set_value(pin, mute?GPIO_VALUE_HIGH:GPIO_VALUE_LOW);
#else
    zhuque_bsp_gpio_set_value(pin, mute?GPIO_VALUE_LOW:GPIO_VALUE_HIGH);
#endif
    printf("pa_mute_ctrl===%d\r\n",mute);
}


/****************************************************************************
 * Name: bt_mute_detect
 *
 * Description:
 *    蓝牙mute检测
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void bt_power_crt(bool on_off)
{
    const int pin = BT_POWER_CRT_PIN;
    zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PULLING_HIGH);
    zhuque_bsp_gpio_set_value(pin, on_off?GPIO_VALUE_HIGH:GPIO_VALUE_LOW);
}



/****************************************************************************
 * Name: bt_mute_detect
 *
 * Description:
 *    蓝牙mute检测
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
/****************************************************************




******************************************************************/

void sys_power_control(void)
{
	printf("%s:hdmi power off\n", __func__);
    const int pin = SYS_POWER_CON_PIN;
    zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PULLING_NONE);
    zhuque_bsp_gpio_set_value(pin, GPIO_VALUE_HIGH);
	usleep(400000);
	zhuque_bsp_gpio_set_value(pin, GPIO_VALUE_LOW);
	//usleep(500000);
	//zhuque_bsp_gpio_set_value(pin, GPIO_VALUE_HIGH);
	//usleep(500000);
	//zhuque_bsp_gpio_set_value(pin, GPIO_VALUE_LOW);

}


/****************************************************************




******************************************************************/
void pa_io_ret_set(bool on_off)
{
    zhuque_bsp_gpio_set_mode(PA_RESET_PIN, GPIO_OUT, PULLING_HIGH);
    zhuque_bsp_gpio_set_value(PA_RESET_PIN, on_off?GPIO_VALUE_HIGH:GPIO_VALUE_LOW);
}



/****************************************************************




******************************************************************/
void pa_static_check(void)
{
	static int count1,count2,count3,count4= 0;
	uint32_t value1,value2,value3;
	ui_cmd_t cmd;

	zhuque_bsp_gpio_set_mode(PA_FAULT_PIN, GPIO_IN, PULLING_HIGH);
	zhuque_bsp_gpio_set_mode(PA_CLIP_OTW_PIN, GPIO_IN, PULLING_HIGH);
	zhuque_bsp_gpio_set_mode(SL_HDMI_CEC_DET_PIN, GPIO_IN, PULLING_HIGH);

	zhuque_bsp_gpio_get_value(PA_FAULT_PIN, &value1);
	zhuque_bsp_gpio_get_value(PA_CLIP_OTW_PIN, &value2);
	zhuque_bsp_gpio_get_value(SL_HDMI_CEC_DET_PIN, &value3);

	tre_bass_cnt   ++;
	auto_input_cnt ++;
	if(tre_bass_cnt == 60)
	{
		enter_tre_set = false;
		enter_bass_set = false;
	}
	if(tre_bass_cnt > 1000)
	{
		tre_bass_cnt = 100;
	}

	if(auto_input_cnt == 60)
	{
		cmd.cmd = UI_CMD_ENTER;
		send_cmd_2_ui(&cmd);
	}

	if(value1==0)
	{
		if(count1<100)
		{
			count1++;
			if(count1>80)
			{
				count1=200;
				//pa_io_ret_set(false);
			}
		}
	}
	else
	{
		//  if(count1)
		//  pa_io_ret_set(true);
		count1=0;
	}
	////////////////////////////////////////////////


	if(value2==0)
	{
		if(count2<100)
		{
			count2++;
			if(count2>80)
			{
				count2=200;
			}
		}
	}
	else
	{
		count2=0;
	}

	/////////////////////////////////////////////////////

	if(value3==0)
	{
		count4=0;
		if(count3<100)
		{
			count3++;
			if(count3>20)
			{
				count3=200;
				hdmi_cec_online=1;
				cmd.cmd = UI_CMD_HDMI_CONNECT;
				send_cmd_2_ui(&cmd);
			}
		}
	}
	else
	{
		count3=0;
		if(count4<100)
		{
			count4++;
			if(count4>20)
			{
				count4=200;
				hdmi_cec_online=0;
				cmd.cmd = UI_CMD_HDMI_DISCONNECT;
				send_cmd_2_ui(&cmd);
			}
		}
	}


	////////////////////////////////////

	if(wdtimer_pa_station_detect != NULL)
	{
		wd_start(wdtimer_pa_station_detect, CLOCKS_PER_SEC/20,pa_static_check, 0);/////50ms
	}
}

/****************************************************************




******************************************************************/
void enter_testmode_check(void)
{
	ui_cmd_t cmd;

	enter_count++;
	if(enter_count >= 40)
	{
		enter_count = 0;
		if(playkey_count < 3)
		{
			playkey_count = 0;
		}

		if(prekey_count < 3)
		{
			prekey_count = 0;
		}
	}

	if(prekey_count >= 3)
	{
		prekey_count = 0;
		playkey_count = 0;
		test_mode_flag = true;
		cmd.cmd = UI_CMD_GO_TO_TEST;
		send_cmd_2_ui(&cmd);
	}

	////////////////////////////////////

	if(wdtimer_enter_testmode_detect != NULL)
	{
		wd_start(wdtimer_enter_testmode_detect, CLOCKS_PER_SEC/20,enter_testmode_check, 0);/////50ms
	}
}


/****************************************************************




******************************************************************/
void change_mode_unmute(void)
{
	ui_cmd_t cmd;

	unmute_count++;

	if(unmute_count == 40)
	{
		printf(">>>>>>>%s:UI_CMD_CHANGE_MODE_UNMUTE>>>>>>>\n",__func__);
		cmd.cmd = UI_CMD_CHANGE_MODE_UNMUTE;
		send_cmd_2_ui(&cmd);
	}

	if(unmute_count == 65)
	{
		printf(">>>>>>>%s:UI_CMD_CHANGE_MODE_VOL_REC.>>>>>>>\n",__func__);
		cmd.cmd = UI_CMD_CHANGE_MODE_VOL_REC;
		send_cmd_2_ui(&cmd);
	}

	if(unmute_count > 1000)
	{
		unmute_count = 100;
	}

	////////////////////////////////////

	if(wdtimer_change_mode_unmute != NULL)
	{
		wd_start(wdtimer_change_mode_unmute, CLOCKS_PER_SEC/20,change_mode_unmute, 0);/////50ms
	}
}




void ui_hdmion_send(void)
{
	static int send_count1=0,send_count2=0;
	ui_cmd_t cmd;

/////////////////////////////////////////////////////
	hdmisend_count++;

	if(hdmisend_count  <= 6)
	{
		cmd.cmd = UI_CMD_HDMION_SEND;
		send_cmd_2_ui(&cmd);
	}

	if(wdtimer_hdmion_send           != NULL)
    {
		wd_start(wdtimer_hdmion_send, 6000,ui_hdmion_send, 0);
    }

    #ifdef SL_UI_DBG
    printf("%s\n",__func__);
    #endif
}



/****************************************************************




******************************************************************/
void aux_fm_channel_choose(bool chan)///////0----fm,1-----aux
{
    const int pin = FM_AUX_CHANNEL_CRT_PIN;
    zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PULLING_HIGH);
    zhuque_bsp_gpio_set_value(pin, chan?GPIO_VALUE_HIGH:GPIO_VALUE_LOW);
}




/****************************************************************




******************************************************************/
void pcm1803_power_crt(bool on_off)
{
	const int pin = POWER_MIC_PIN;
	zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PULLING_HIGH);
	zhuque_bsp_gpio_set_value(pin, on_off?GPIO_VALUE_HIGH:GPIO_VALUE_LOW);
}


