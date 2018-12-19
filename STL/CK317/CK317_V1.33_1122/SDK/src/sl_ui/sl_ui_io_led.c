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


/////////////////////////STL_CK317/////////////////////////
int rgb_colour = RGB_RED;



//////////////////////////////////////////////////////////////

int hdmisend_count = 0;
int hdmi_deinit_count = 0;
bool hdmi_cec_online = false;
bool mic_detect_online = false;
bool auxin_detect_online = false;


extern  WDOG_ID wdtimer_pa_station_detect;
//extern  WDOG_ID wdtimer_enter_testmode_detect;
extern  WDOG_ID wdtimer_change_mode_unmute;
extern  WDOG_ID wdtimer_hdmion_send;

int enter_count = 0;
int playkey_count = 0;
int prekey_count = 0;
int tre_bass_cnt = 100;
int bt_wait_cnt = 0;
bool bt_wait_flag = false;


bool test_mode_flag = false;

extern int unmute_count;

extern int  mix_vol;


extern bool enter_tre_set;
extern bool enter_bass_set;
extern int auto_input_cnt;

extern struct input_event save_ir_event;
extern bool ir_short_flag;
extern int save_ir_cnt;
extern int fm_manual_save_cnt;
extern bool fm_manual_save_status;
extern bool change_mode_flag;
extern int change_mode_cnt;
extern int usb_play_cnt;

extern bool frist_hdmi_init;

void set_iic_clk_freq(void)
{

	(*(volatile unsigned int  *)(0xbe0a0014)) = 10000;
	(*(volatile unsigned int  *)(0xbe0a0018)) = 10000;	
}


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
	set_iic_clk_freq();
		
	pa_mute_ctrl(true);

	REG32(KSEG1(SILAN_PADMUX_CTRL))=0;
	REG32(KSEG1(SILAN_PADMUX_CTRL2))=0;
	value = REG32(KSEG1(SILAN_PADMUX_CTRL));
	printf("SILAN_PADMUX_CTRL----int=0x%x\n", value);
	value = REG32(KSEG1(SILAN_PADMUX_CTRL2));
	printf("SILAN_PADMUX_CTRL2----int=0x%x\n", value);

	
	//REG1/////////////////////////////////////////////////////
	//UART2 debug
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
	//value |= (1 << SILAN_PADMUX_SPDIF_IN1);
	value |= (1 << SILAN_PADMUX_SPDIF_IN2);

	value |= (1 << SILAN_PADMUX_I2C2);///////hdmi
	value |= (1 << SILAN_PADMUX_UART3_1);

	REG32(KSEG1(SILAN_PADMUX_CTRL)) = value;

	///////////////////////////////////////////////////////

	///////////////////////////////////////////////////////
	value = REG32(KSEG1(SILAN_PADMUX_CTRL2));

	//value |= (1 << SILAN_PADMUX2_IISADC_FD1_CH2);
	value |= (1 << SILAN_PADMUX2_IISADC_FD2_CH2);

	REG32(KSEG1(SILAN_PADMUX_CTRL2)) = value;
	//////////////////////////////////////////////////////

	//////////////////////////////////////////////////////
	value = REG32(KSEG1(SILAN_PADMUX_CTRL));
	printf("SILAN_PADMUX_CTRL=0x%x\n", value);
	value = REG32(KSEG1(SILAN_PADMUX_CTRL2));
	printf("SILAN_PADMUX_CTRL2=0x%x\n", value);

	//zhuque_bsp_gpio_set_mode(PA_FAULT_PIN, GPIO_IN, PULLING_HIGH);
	//zhuque_bsp_gpio_set_mode(PA_CLIP_OTW_PIN, GPIO_IN, PULLING_HIGH);
	zhuque_bsp_gpio_set_mode(SL_HDMI_CEC_DET_PIN, GPIO_IN, PULLING_HIGH);
	zhuque_bsp_gpio_set_mode(SL_AUXIN_DET_PIN, GPIO_IN, PULLING_HIGH);

	zhuque_bsp_gpio_set_mode(PWR_LED_PIN, GPIO_OUT, PUSH_PULL);
	
	zhuque_bsp_gpio_set_mode(BT_LED_PIN, GPIO_OUT, PUSH_PULL);
	zhuque_bsp_gpio_set_mode(HDMI_LED_PIN, GPIO_OUT, PUSH_PULL);
	zhuque_bsp_gpio_set_mode(OPT_LED_PIN, GPIO_OUT, PUSH_PULL);
	zhuque_bsp_gpio_set_mode(LINEIN_LED_PIN, GPIO_OUT, PUSH_PULL);


	zhuque_bsp_gpio_set_mode(RGB_R_PIN, GPIO_OUT, PUSH_PULL);
	zhuque_bsp_gpio_set_mode(RGB_G_PIN, GPIO_OUT, PUSH_PULL);
	zhuque_bsp_gpio_set_mode(RGB_B_PIN, GPIO_OUT, PUSH_PULL);

	zhuque_bsp_gpio_set_mode(MIC_LED_PIN, GPIO_OUT, PUSH_PULL);

	zhuque_bsp_gpio_set_mode(SW1_4052, GPIO_OUT, PUSH_PULL);
	zhuque_bsp_gpio_set_mode(SW2_4052, GPIO_OUT, PUSH_PULL);

	zhuque_bsp_gpio_set_value(PWR_LED_PIN, GPIO_VALUE_LOW);
	
	show_RGB_off();
	show_modeled_off();

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




/********************************************************




********************************************************/

void show_modeled_off(void)
{
    zhuque_bsp_gpio_set_value(BT_LED_PIN, GPIO_VALUE_HIGH);
	zhuque_bsp_gpio_set_value(HDMI_LED_PIN, GPIO_VALUE_HIGH);
	zhuque_bsp_gpio_set_value(OPT_LED_PIN, GPIO_VALUE_HIGH);
	zhuque_bsp_gpio_set_value(LINEIN_LED_PIN, GPIO_VALUE_HIGH);
}


/********************************************************




********************************************************/
void show_modeled_on(int mode)
{
	int value;

    show_modeled_off();

	switch(mode)
	{
		//P2.10 RGB:B	  low enable
		case SOURCE_SELECT_AUX:
			//AUX:Red
			zhuque_bsp_gpio_set_value(LINEIN_LED_PIN, GPIO_VALUE_LOW);
			break;
		
		case SOURCE_SELECT_RCA:
			//RCA:white白色
			zhuque_bsp_gpio_set_value(LINEIN_LED_PIN, GPIO_VALUE_LOW);
			break;
		
		case SOURCE_SELECT_SPDIFIN:
			//OPT
			zhuque_bsp_gpio_set_value(OPT_LED_PIN, GPIO_VALUE_LOW);
			break;
		
		case SOURCE_SELECT_HDMI:
            //HDMI:Green        //Purple紫色
			zhuque_bsp_gpio_set_value(HDMI_LED_PIN, GPIO_VALUE_LOW);
			break;
		
		case SOURCE_SELECT_BT:
            //BT:Blue
			zhuque_bsp_gpio_set_value(BT_LED_PIN, GPIO_VALUE_LOW);
			break;
	}
}


/********************************************************




********************************************************/

void show_RGB_off(void)
{
    zhuque_bsp_gpio_set_value(RGB_R_PIN, GPIO_VALUE_HIGH);
	zhuque_bsp_gpio_set_value(RGB_G_PIN, GPIO_VALUE_HIGH);
	zhuque_bsp_gpio_set_value(RGB_B_PIN, GPIO_VALUE_HIGH);
}


/********************************************************




********************************************************/
void show_RGB_on(int mode)
{

    //show_RGB_off();

	switch(rgb_colour)
	{		
		case RGB_RED:
			zhuque_bsp_gpio_set_value(RGB_R_PIN, GPIO_VALUE_LOW);
			zhuque_bsp_gpio_set_value(RGB_G_PIN, GPIO_VALUE_HIGH);
			zhuque_bsp_gpio_set_value(RGB_B_PIN, GPIO_VALUE_HIGH);
			rgb_colour = RGB_GREEN;
			break;
		
		case RGB_GREEN:
			zhuque_bsp_gpio_set_value(RGB_R_PIN, GPIO_VALUE_HIGH);
			zhuque_bsp_gpio_set_value(RGB_G_PIN, GPIO_VALUE_LOW);
			zhuque_bsp_gpio_set_value(RGB_B_PIN, GPIO_VALUE_HIGH);
			rgb_colour = RGB_BLUE;
			break;
		
		case RGB_BLUE:
			zhuque_bsp_gpio_set_value(RGB_R_PIN, GPIO_VALUE_HIGH);
			zhuque_bsp_gpio_set_value(RGB_G_PIN, GPIO_VALUE_HIGH);
			zhuque_bsp_gpio_set_value(RGB_B_PIN, GPIO_VALUE_LOW);
			rgb_colour = RGB_WHITE;
			break;
		
		case RGB_WHITE:
			zhuque_bsp_gpio_set_value(RGB_R_PIN, GPIO_VALUE_LOW);
			zhuque_bsp_gpio_set_value(RGB_G_PIN, GPIO_VALUE_LOW);
			zhuque_bsp_gpio_set_value(RGB_B_PIN, GPIO_VALUE_LOW);
			rgb_colour = RGB_PURPLE;
			break;
		
		case RGB_PURPLE:
			zhuque_bsp_gpio_set_value(RGB_R_PIN, GPIO_VALUE_LOW);
			zhuque_bsp_gpio_set_value(RGB_G_PIN, GPIO_VALUE_HIGH);
			zhuque_bsp_gpio_set_value(RGB_B_PIN, GPIO_VALUE_LOW);
			rgb_colour = RGB_ORANGE;
			break;

		case RGB_ORANGE:
			zhuque_bsp_gpio_set_value(RGB_R_PIN, GPIO_VALUE_HIGH);
			zhuque_bsp_gpio_set_value(RGB_G_PIN, GPIO_VALUE_LOW);
			zhuque_bsp_gpio_set_value(RGB_B_PIN, GPIO_VALUE_LOW);
			rgb_colour = RGB_YELLOW;
			break;

		case RGB_YELLOW:
			zhuque_bsp_gpio_set_value(RGB_R_PIN, GPIO_VALUE_LOW);
			zhuque_bsp_gpio_set_value(RGB_G_PIN, GPIO_VALUE_LOW);
			zhuque_bsp_gpio_set_value(RGB_B_PIN, GPIO_VALUE_HIGH);
			rgb_colour = RGB_RED;
			break;
	}
}

/****************************************************************************
 * Name: switch_4052_function
 *
 * Description:
 *    通过IO口控制4052切换声音输出
 *
 * Parameters:
 *    function  switch_4052类型，指定要切换到AUX1、AUX2还是蓝牙
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
		case BT_4052:
			sw1 = 0;
			sw2 = 0;
			break;
		case AUX_4052://AUX        SW1:B --- 0   SW2:A --- 1
			sw1 = 0;
			sw2 = 1;
			printf("%s:choose AUX.\n",__func__);
			break;
		case RCA_4052://RCA        SW1:B --- 1    SW2:A --- 0
			sw1 = 1;
			sw2 = 0;
			printf("%s:choose RCA.\n",__func__);
			break;
		default:
			break;
	}

	zhuque_bsp_gpio_set_value(SW1_4052, sw1);
	zhuque_bsp_gpio_set_value(SW2_4052, sw2);
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
#if 0
    const int pin = BT_POWER_CRT_PIN;
    zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PULLING_HIGH);
    zhuque_bsp_gpio_set_value(pin, on_off?GPIO_VALUE_HIGH:GPIO_VALUE_LOW);
#endif
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
#if 0
	printf("%s:hdmi power off\n", __func__);
    const int pin = SYS_POWER_CON_PIN;
    zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PULLING_NONE);
	usleep(500000);
	usleep(500000);
    zhuque_bsp_gpio_set_value(pin, GPIO_VALUE_HIGH);
	usleep(500000);
	zhuque_bsp_gpio_set_value(pin, GPIO_VALUE_LOW);
	//usleep(500000);
	//zhuque_bsp_gpio_set_value(pin, GPIO_VALUE_HIGH);
	//usleep(500000);
	//zhuque_bsp_gpio_set_value(pin, GPIO_VALUE_LOW);
#endif
}


/****************************************************************




******************************************************************/
void pa_io_ret_set(bool on_off)
{
	#if 0
    zhuque_bsp_gpio_set_mode(PA_RESET_PIN, GPIO_OUT, PULLING_HIGH);
    zhuque_bsp_gpio_set_value(PA_RESET_PIN, on_off?GPIO_VALUE_HIGH:GPIO_VALUE_LOW);
	#endif
}



/****************************************************************




******************************************************************/
void pa_static_check(void)
{
	static int count1,count2,count3,count4= 0;
	static int mode_count = 0;
	uint32_t value1,value2,value3;
	ui_cmd_t cmd;

	//zhuque_bsp_gpio_get_value(PA_FAULT_PIN, &value1);
	//zhuque_bsp_gpio_get_value(MIC_DET_PIN, &value1);
	zhuque_bsp_gpio_get_value(SL_AUXIN_DET_PIN, &value2);
	zhuque_bsp_gpio_get_value(SL_HDMI_CEC_DET_PIN, &value3);

#if 0
	usb_play_cnt++;
	fm_manual_save_cnt++;
	bt_wait_cnt++;
	save_ir_cnt++;
	tre_bass_cnt   ++;
	auto_input_cnt ++;
	if(tre_bass_cnt == 60)
	{
		enter_tre_set = false;
		enter_bass_set = false;
	}

	if(auto_input_cnt == 60)
	{
		cmd.cmd = UI_CMD_ENTER;
		send_cmd_2_ui(&cmd);
	}

	if(save_ir_cnt == 4)
	{
		if(ir_short_flag)
		{
			ir_short_flag = false;
			input_add_event(&save_ir_event);
		}
	}


	if(bt_wait_cnt == 3600)
	{
		bt_wait_flag = true;
	}


	if(fm_manual_save_cnt == 100)
	{
		cmd.cmd = UI_CMD_ENTER;
		send_cmd_2_ui(&cmd);
	}


	if(usb_play_cnt == 50)
	{
		cmd.cmd = UI_CMD_USB_PLAY_MUTE;
		send_cmd_2_ui(&cmd);
	}

	enter_testmode_check();

#endif

#if 0
	if(value1==0)
	{
		count2=0;
		if(count1<100)
		{
			count1++;
			if(count1>20)
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
			if(count2>20)
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

	if(value2==0)
	{
		count2=0;
		if(count1<100)
		{
			count1++;
			if(count1>20)
			{
				count1=200;
				auxin_detect_online = true;
				cmd.cmd = UI_CMD_AUX_CONNECT;
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
			if(count2>20)
			{
				count2=200;
				auxin_detect_online = false;
				cmd.cmd = UI_CMD_AUX_DISCONNECT;
				send_cmd_2_ui(&cmd);
			}
		}
	}

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
#if 0
	if(wdtimer_enter_testmode_detect != NULL)
	{
		wd_start(wdtimer_enter_testmode_detect, CLOCKS_PER_SEC/20,enter_testmode_check, 0);/////50ms
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
		printf("%s:1.\n",__func__);
		cmd.cmd = UI_CMD_CHANGE_MODE_UNMUTE;
		send_cmd_2_ui(&cmd);
	}

	if(unmute_count == 40)
	{
		printf("%s:2.\n",__func__);
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



/****************************************************************




******************************************************************/
void aux_fm_channel_choose(bool chan)///////0----fm,1-----aux
{
#if 0
    const int pin = FM_AUX_CHANNEL_CRT_PIN;
    zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PULLING_HIGH);
    zhuque_bsp_gpio_set_value(pin, chan?GPIO_VALUE_HIGH:GPIO_VALUE_LOW);
#endif
}




/****************************************************************




******************************************************************/
void pcm1803_power_crt(bool on_off)
{
#if 0
	const int pin = POWER_MIC_PIN;
	zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PULLING_HIGH);
	zhuque_bsp_gpio_set_value(pin, on_off?GPIO_VALUE_HIGH:GPIO_VALUE_LOW);
#endif
}


