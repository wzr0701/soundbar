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
int hdmi_deinit_count = 0;
bool hdmi_cec_online=false;
bool mic_detect_online=false;

extern  WDOG_ID wdtimer_pa_station_detect;
//extern  WDOG_ID wdtimer_enter_testmode_detect;
extern  WDOG_ID wdtimer_change_mode_unmute;
extern  WDOG_ID wdtimer_hdmion_send;

struct input_event save_ir_event_tmp;


int enter_count = 0;
int playkey_count = 0;
int prevkey_count = 0;

int enter_count1 = 0;
int playkey_count1 = 0;
int nextkey_count = 0;

int tre_bass_cnt = 100;
int bt_wait_cnt = 0;
int fm_scan_start_end_cnt = 0;


bool bt_wait_flag = false;

bool test_mode_flag = false;

extern int unmute_count;

extern int  mix_vol;


extern bool enter_tre_set;
extern bool enter_bass_set;
extern int auto_input_cnt;

extern struct input_event save_ir_event;
extern bool ir_short_flag;
extern bool ir_long_flag;
extern int save_ir_cnt;
//extern int fm_manual_save_cnt;
//extern bool fm_manual_save_status;
extern bool change_mode_flag;
extern int change_mode_cnt;
extern int usb_play_cnt;
extern int folder_dis_cnt;
extern bool frist_hdmi_init;

extern bool fm_scan_end_flag;

extern char dis_other_mode;

extern bool folder_dis_flag;

extern char mic_on_flag;

extern int ui_source_select;

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
	REG32(KSEG1(SILAN_PADMUX_CTRL))=0;
	REG32(KSEG1(SILAN_PADMUX_CTRL2))=0;
	value = REG32(KSEG1(SILAN_PADMUX_CTRL));
	printf("SILAN_PADMUX_CTRL----int=0x%x\n", value);
	value = REG32(KSEG1(SILAN_PADMUX_CTRL2));
	printf("SILAN_PADMUX_CTRL2----int=0x%x\n", value);

	pa_mute_ctrl(true);

	value = REG32(KSEG1(SILAN_PADMUX_CTRL));

	//Debug UART2
	value |= (1 << SILAN_PADMUX_UART2);

	/////////////////////////////////////////
	//IIS-------IN  //IIS CLK  GPIO1_1 GPIO1_2
	value |= (1 << SILAN_PADMUX_IISADC);

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

	//////////////////////////////////////////////////////
	value = REG32(KSEG1(SILAN_PADMUX_CTRL));
	printf("SILAN_PADMUX_CTRL=0x%x\n", value);
	value = REG32(KSEG1(SILAN_PADMUX_CTRL2));
	printf("SILAN_PADMUX_CTRL2=0x%x\n", value);

	zhuque_bsp_gpio_set_mode(SL_HDMI_CEC_DET_PIN, GPIO_IN, PULLING_HIGH);

	zhuque_bsp_gpio_set_mode(PA_MUTE_PIN, GPIO_OUT, PUSH_PULL);

	zhuque_bsp_gpio_set_mode(SW1_4052_PIN, GPIO_OUT, PUSH_PULL);
	zhuque_bsp_gpio_set_mode(SW2_4052_PIN, GPIO_OUT, PUSH_PULL);
	

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
 * Name: switch_4052_function
 *
 * Description:
 *    通过IO口控制4052切换声音输出
 *
 * Parameters:
 *    function  switch_4052类型，指定要切换到AUX1、AUX2还是FM
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void switch_4052_function(int function)
{
	//SW1:B SW2:A
	int sw1, sw2;

	switch (function)
	{
		case FM_4052:
			sw1 = 0;
			sw2 = 0;
			printf("%s:choose FM.\n",__func__);
			break;
		case RCA_4052://RCA        SW1:B --- 0   SW2:A --- 1
			sw1 = 0;
			sw2 = 1;
			printf("%s:choose RCA.\n",__func__);
			break;
		case AUX_4052://AUX        SW1:B --- 1    SW2:A --- 0
			sw1 = 1;
			sw2 = 0;
			printf("%s:choose AUX.\n",__func__);
			break;
		case NONE_4052:
			sw1 = 1;
			sw2 = 1;
			printf("%s:choose NO.\n",__func__);
		default:
			break;
	}

	zhuque_bsp_gpio_set_value(SW1_4052_PIN, sw1);
	zhuque_bsp_gpio_set_value(SW2_4052_PIN, sw2);
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

#if(PA_MUTE_HIGH==0)
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
/****************************************************************




******************************************************************/

void sys_power_control(void)
{
	
}



/****************************************************************




******************************************************************/
void pa_static_check(void)
{
	static int count1,count2,count3,count4= 0;
	static int mode_count = 0;
	uint32_t value1,value2,value3;
	ui_cmd_t cmd;

	zhuque_bsp_gpio_get_value(SL_HDMI_CEC_DET_PIN, &value3);

#if 1
	folder_dis_cnt++;
	usb_play_cnt++;
	//fm_manual_save_cnt++;
	bt_wait_cnt++;
	save_ir_cnt++;
	tre_bass_cnt   ++;
	auto_input_cnt ++;
	fm_scan_start_end_cnt++;
	if(tre_bass_cnt == 50)
	{
		enter_tre_set = false;
		enter_bass_set = false;
	}

	if(auto_input_cnt == 50)
	{
		cmd.cmd = UI_CMD_ENTER;
		send_cmd_2_ui(&cmd);
	}

	if(save_ir_event.type != 0)
	{
		save_ir_event_tmp.type = save_ir_event.type;
		save_ir_event_tmp.code =save_ir_event.code;
		save_ir_event_tmp.value = save_ir_event.value;

		save_ir_event.type = 0;
		save_ir_event.code = 0;
		save_ir_event.value = 0;
		
	}
	
	if(save_ir_cnt == 4)
	{
		if(ir_short_flag)
		{
			if(save_ir_event_tmp.type == EV_IR)
			{
				input_add_event(&save_ir_event_tmp);	

				save_ir_event_tmp.type = 0;
				save_ir_event_tmp.code = 0;
				save_ir_event_tmp.value = 0;
			}

		}
	}


	if(bt_wait_cnt == 3600)
	{
		bt_wait_flag = true;
	}


	if(usb_play_cnt == 50)
	{
		cmd.cmd = UI_CMD_USB_PLAY_MUTE;
		send_cmd_2_ui(&cmd);
	}

	if(folder_dis_cnt <= 20)
	{
		dis_other_mode=1;
		if(folder_dis_cnt == 20)
		{
			folder_dis_flag = false;
			cmd.cmd = UI_CMD_USB_FOLDER_DIS;
			send_cmd_2_ui(&cmd);
		}
	}

	if(fm_scan_start_end_cnt == 3)
	{
		fm_scan_start_end_cnt = 0;
		
		if(fm_scan_end_flag)
		{
			fm_scan_end_flag = false;
			cmd.cmd = UI_CMD_FM_SCAN_END;
			send_cmd_2_ui(&cmd);
		}	
	
	}

	//enter_othermode_check();

#endif

#if 0
	if(value1==0)
	{
		count2=0;
		if(count1<100)
		{
			count1++;
			if(count1>10)
			{
				count1=200;
				mic_detect_online = false;
				cmd.cmd = UI_CMD_MIC_DISCONNECT;
				send_cmd_2_ui(&cmd);
			}
		}
	}
	else
	{
		count1=0;
		if(count2<100)
		{
			count2++;
			if(count2>10)
			{
				count2=200;
				mic_detect_online = true;
				cmd.cmd = UI_CMD_MIC_CONNECT;
				send_cmd_2_ui(&cmd);
			}
		}
	}
	////////////////////////////////////////////////
#endif
#if 0
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
#endif
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
				hdmi_cec_online = true;
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
				hdmi_cec_online = false;
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
void enter_othermode_check(void)
{
	ui_cmd_t cmd;

	enter_count++;
	enter_count1++;
	if(enter_count >= 40)
	{
		enter_count = 0;
		if(playkey_count < 3)
		{
			playkey_count = 0;
		}
		else
		{
			if(prevkey_count < 3)
			{
				playkey_count = 0;
			}
		}

		if(prevkey_count < 3)
		{
			prevkey_count = 0;
		}

	}

	if(enter_count1 >= 40)
	{
		enter_count1 = 0;
		if(playkey_count1 < 5)
		{
			playkey_count1 = 0;
		}
		else
		{
			if(nextkey_count < 5)
			{
				playkey_count1 = 0;
			}
		}

		if(nextkey_count < 5)
		{
			nextkey_count = 0;
		}
	}

	if(prevkey_count >= 3)
	{
		prevkey_count = 0;
		playkey_count = 0;
		test_mode_flag = true;
		cmd.cmd = UI_CMD_GO_TO_TEST;
		send_cmd_2_ui(&cmd);
	}

	if(nextkey_count >= 5)
	{
		nextkey_count = 0;
		playkey_count1 = 0;
		cmd.cmd = UI_CMD_EQ_POWER_TEST;
		send_cmd_2_ui(&cmd);
	}

	////////////////////////////////////
#if 0
	if(wdtimer_enter_testmode_detect != NULL)
	{
		wd_start(wdtimer_enter_testmode_detect, CLOCKS_PER_SEC/20,enter_othermode_check, 0);/////50ms
	}
#endif
}


/****************************************************************




******************************************************************/
void change_mode_unmute(void)
{
	ui_cmd_t cmd;

	if(wdtimer_change_mode_unmute != NULL)
	{
		wd_start(wdtimer_change_mode_unmute, CLOCKS_PER_SEC/20,change_mode_unmute, 0);/////50ms
	}

	unmute_count++;

	if(unmute_count == 30)
	{
		//printf("%s:2.\n",__func__);
		cmd.cmd = UI_CMD_CHANGE_MODE_VOL_REC;
		send_cmd_2_ui(&cmd);
		wd_cancel(wdtimer_change_mode_unmute);
	}

	////////////////////////////////////
}




void ui_hdmion_send(void)
{
	static int send_count1=0,send_count2=0;
	ui_cmd_t cmd;

	if(NULL != wdtimer_hdmion_send)
	{
		wd_start(wdtimer_hdmion_send, 900, ui_hdmion_send, 0);
	}

	if (swa_audio_check_playing())
	{
		printf("\n---------->>>>>>>>>>>>hdmi is playing ! cancel hdmi send\n");
		cmd.cmd = UI_CMD_HDMION_SEND;
		send_cmd_2_ui(&cmd);
		wd_cancel(wdtimer_hdmion_send);
	}


#ifdef SL_UI_DBG
	printf("%s\n",__func__);
#endif

}

