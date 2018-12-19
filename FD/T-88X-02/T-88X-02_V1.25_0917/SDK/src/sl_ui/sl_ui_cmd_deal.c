#include "bt_uart.h"
#include <errno.h>
#include <fcntl.h>
#include <nuttx/audio/sladsp_ae.h>
#include <nuttx/power/pm.h>
#include <nxplayer.h>
#include "player_cmd.h"
#include <pthread.h>
#include <silan_gpio.h>
#include <silan_addrspace.h>
#include <silan_resources.h>
#include "sl_lcd.h"
#include "sl_ui_cmd.h"
#include "sl_ui_handle.h"
#include "sl_ui_local.h"
#include <string.h>
#include <sys/ioctl.h>
#include <silan_adc.h>

#include "ht1633.h"
#include "sl_ui_io_led.h"
#include "sl_ui_cmd_deal.h"
#include "sl_ui_fm.h"
#include "at24c02.h"
#include "tone_data.h"
#include "sl_ui_display.h"
#include <silan_cec.h>
#include "sl_ui_old.h"


#define UI_TASK_DELAY   10000
#define MIC_ENABLE_ADC0   1

#define MIC_VOL_STEP (100.0f/60)


#define MCU_VERSION 0
#define BT_VERSION           1


int mic_vol = 27;
int echo_vol_lev = 15;
int mic_vol_lev = 15;
extern int mute_state;

char echo_text[64] = "1 -600 200 0 0 1 2";
char revb_text[64] = "50 50 2 20 500 14000 -9 -6 90 2";
int echo_mix_table[16]={
	0, -100, -300, -500, -700,	-900, -1100, -1300, -1500,
	-1600, -1800, -2000, -2200, -2400, -2800, -3000
};

int reverb_table[16]={
	20, 740, 1454, 2168, 2882, 3596, 4310, 5024, 5738, 6452,
	7166, 7880, 8594, 9308, 9650, 10000
};

static const int mic_vol_table[61]={
	0, 2, 4, 6, 8, 10,
	11, 13, 15, 17,18,20,
	21, 23, 25, 27,28,30,
	31, 33, 35, 37,38,40,
	41, 43, 45, 47,48,50,
	51, 53, 55, 57,58,60,
	61, 63, 65, 67,68,70,
	71, 73, 75, 77,78,80,
	81, 83, 85, 87,88,90,
	91, 93, 94, 96,98,99,
	100
};



char mic_vol_flag = false;
char mic_echo_flag = false;

static char mic_on_flag=0;
extern  int mix_vol ;
extern int bt_mix_vol;
extern bool bt_connected;

unsigned int  input_number=0;
unsigned int  input_number_ok=0;
char input_n=0;
int auto_input_cnt = 100;

int  bass_vol = 0;
int treble_vol = 0;

int unmute_count = 100;
bool enter_tre_set = false;
bool enter_bass_set = false;


bool fm_test_flag = false;

bool frist_hdmi_init = false;
WDOG_ID wdtimer_hdmion_send = NULL;

extern bool usb_online;
extern bool sd_online;
extern bool aux_online;
extern bool hdmi_cec_online;
extern int ui_source_select;
extern char dis_other_mode;

extern u16 fmFrequency;
extern u8 Cur_Fre_Num;
extern unsigned char power_on_usb_sd_auto_play;
extern  WDOG_ID wdtimer_display_updata;

extern bool test_mode_flag;
extern int tre_bass_cnt;

extern ui_cmd_t get_mq_msg(void);
extern void mq_msg_clear(void);
extern void  put_ui_msg(int ui_cmd);
extern void set_ui_media(int source);
extern void player_paramter_set_init(AUDIO_OUT_MODE outMode, AUDIO_IN_MODE inMode, int chnNum, int spdifNum);

void sl_ui_fm_test(void)
{
	if(fm_test_flag == false)
	{
		fm_test_flag = true;

		player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 1, NULL, NULL);
		usleep(1000);

		FM_Mode();
		pcm1803_power_crt(true);
		aux_fm_channel_choose(false);
		sl_ui_set_reqrate();

		player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_EXTRA_MASTER_IN, 2, 0);
		player_process_cmd(NP_CMD_I2SIN_CHAN, NULL, 1, NULL, NULL);//////////adc1
		usleep(1000);

#if (BYPASS_MODE || INNERADC_MODE)
		player_process_cmd(NP_CMD_LINEIN_ON, NULL, 0, NULL, NULL);
#else
		player_process_cmd(NP_CMD_I2SIN_OPEN, NULL, 1, NULL, NULL);//////////adc1
#endif
		usleep(1000);
		//set_channel_vol_by_mode(SOURCE_SELECT_FM);

		display_ui_fm(0);
	}
	else
	{
		fm_test_flag = false;
		fm_rx_off();
		pcm1803_power_crt(false);
		display_ui_full();
		ht1633_updata_display();

#if (BYPASS_MODE || INNERADC_MODE)
		player_process_cmd(NP_CMD_LINEIN_OFF, NULL, 0, NULL, NULL);
#else
		player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 1, NULL, NULL);
#endif
		usleep(100);
	}
}





/******************************************





******************************************/
void  save_mix_vol(void)
{
	unsigned char temp;

	if(bt_mix_vol <= MIX_LEV_CNT)
	{
		temp= (unsigned char)bt_mix_vol;
		mix_vol = mix_vol_table[bt_mix_vol];
		at24c02_write_one_byte(MEM_MIX_VOL ,temp);
	}

	printf("save-mix_vol = %d\r\n",temp);
}


/******************************************





******************************************/
void  read_mix_vol(void)
{
	unsigned char temp;
	temp=at24c02_read_one_byte(MEM_MIX_VOL);

	if(temp <= MIX_LEV_CNT)
	{
		bt_mix_vol = temp;
		mix_vol = mix_vol_table[bt_mix_vol];
	}
	else
	{
		bt_mix_vol = Frist_MIX_LEV;
		mix_vol = mix_vol_table[bt_mix_vol];
	}


	printf("read-mix_vol = %d\r\n",temp);

}


/******************************************





******************************************/
void  save_player_info(void)
{
	unsigned char temp;

	temp= (unsigned char)ui_source_select;
	if(temp>=0)
		at24c02_write_one_byte(MEM_WORK_MODE ,temp);

	printf("save-ui_source_select=%d\r\n",temp);
}


/******************************************





******************************************/
void  read_player_info(void)
{
	unsigned char temp;
	temp=at24c02_read_one_byte(MEM_WORK_MODE);
	if(temp>=SOURCE_SELECT_END)
		temp=SOURCE_SELECT_BT;
	ui_source_select=temp;

	//ui_source_select=SOURCE_SELECT_HDMI;

	printf("ui_source_select=%d\r\n",temp);

}



/************************************************




************************************************/
void dis_play_update(void)
{

	static unsigned char second_count=0;
	ui_cmd_t cmd;

	if (NULL == wdtimer_display_updata)
	{ 	//首次启动
		wdtimer_display_updata = wd_create();
	}
	else
	{ 	//停止当前的看门狗
		wd_cancel(wdtimer_display_updata);
	}


	if(!dis_other_mode)
	{
		if(!mute_state)
		{
			display_ui_bt();
			ui_update_music_time();
		}
	}

	////////////////////////////////////////////

	second_count++;
	if(second_count==2)
	{
		second_count=0;

		if(dis_other_mode<4)
		{
			display_ui_mute();
		}

		if((0<dis_other_mode)&&(dis_other_mode<3))
		{
			dis_other_mode=3;
			ui_goback_source(400);
		}

	}

	if(wdtimer_display_updata != NULL)
	{
		wd_start(wdtimer_display_updata,  CLOCKS_PER_SEC/2,dis_play_update, 0);/////1*10ms
	}
}


/**************************************************************




***************************************************************/
#if(MIC_ENABLE_ADC0==1)
void mic_open(bool on_off)
{
	char * mic_str;
	if(on_off)
	{
		pcm1803_power_crt(true);
		//mic_str = "0 1 100 100 100 100 100 100";
		//player_process_cmd(NP_CMD_POSTVOL, mic_str, 0, NULL, NULL);
		//player_process_cmd(NP_CMD_I2SIN_OPEN, NULL,0, NULL, NULL);///////adc0
		usleep(1000);
		set_adc_channel_vol(0,mic_vol);
	}
	else
	{
		//mic_str = "0 1 0 0 0 0 0 0";
		//player_process_cmd(NP_CMD_POSTVOL, mic_str, 0, NULL, NULL);
		set_adc_channel_vol(0,0);
	}
	//usleep(1000);

}

void set_echo_vol(int vol)
{
	if (vol == 0)
	{
		player_process_cmd(NP_CMD_UNLOAD_MIC_ECHO, NULL, 0, NULL, NULL);
		usleep(100);
		player_process_cmd(NP_CMD_UNLOAD_REVERB, NULL, 0, NULL, NULL);
		usleep(100);
	}
	else
	{
		// ae_echo_para :type, gain, delay, stype, shift, number, mode
		int set_echo[7] = {1, -600, 200, 0, 0, 1, 2}; //������
		player_process_cmd(NP_CMD_LOAD_MIC_ECHO, NULL, 0, NULL, NULL);
		usleep(1000);
		//set_echo[1] = echo_mix_table[vol];
		sprintf(echo_text, "%d %d %d %d %d %d %d", set_echo[0], set_echo[1]
		, set_echo[2], set_echo[3], set_echo[4], set_echo[5], set_echo[6]);
		printf("echo_text %s \n", echo_text);
		player_process_cmd(NP_CMD_SET_MIC_ECHO, echo_text, 0, NULL, NULL);
		usleep(1000);

		// ae_reverb_para1 :roomsize,damp,predelay,reverberance,
		// tone_low,tone_high,wet,dry,width,mode
		int set_revb[10] = {50, 50, 2, 20, 500, 14000, -9, -6, 90, 2}; //������
		player_process_cmd(NP_CMD_LOAD_REVERB, NULL, 0, NULL, NULL);
		usleep(1000);
		set_revb[3] = reverb_table[vol];
		sprintf(revb_text, "%d %d %d %d %d %d %d %d %d %d", set_revb[0], set_revb[1]
		, set_revb[2], set_revb[3], set_revb[4], set_revb[5], set_revb[6]
		, set_revb[7], set_revb[8], set_revb[9]);
		printf("revb_text %s \n", revb_text);
		player_process_cmd(NP_CMD_CUSTOM_REVERB, revb_text, 0, NULL, NULL);
		usleep(1000);
		//ui_goback_source(900);
	}
}


void set_bass_treble_vol(int mode,int vol)//mode=0:bass  mode = 2:treble
{
	if (vol < -15)
		vol = -15;
	else if (vol > 15)
		vol = 15;
	handle_bass_treble(mode, vol);
}
#endif

/****************************************************************

��һ������0----һ·I2Sin��1---2·i2sin
�ڶ�������������ģʽ��
���������������ģʽ
���ĸ����������������
��5��������  ���������
�����ַ�


*****************************************************************/
void sl_ui_set_bt_samp(bool on_off )
{
#if 1
	char *samp_str;
	if(on_off)
		samp_str = "1 1 1 48000 48000";
	else
		samp_str = "1 1 1 44100 44100";
	player_process_cmd(NP_CMD_SET_SAMPLE, samp_str, 0, NULL, NULL);
#endif

}

/*
改音量太大的问题，固定channel音量，调节总音量NP_CMD_VOLUME_SET；
*/
#define ADC_CHANNEL_VOL_MIX 23
#define ADC_CHANNEL_VOL_USB 14
void set_channel_vol_by_mode(int mode)
{
	/*解mute后opt/coa等模式会有1秒“哧”的噪声，解mute不立即出声音，
	  延时1秒再出声音。
	*/
	#if 1
	set_adc_channel_vol(0,0);
	set_adc_channel_vol(1,0);
	set_adc_channel_vol(2,0);
	set_adc_channel_vol(3,0);

	player_process_cmd(NP_CMD_VOLUME_SET, NULL,0, NULL, NULL);
	#endif
    //usleep(500000);
    //usleep(500000);
	set_adc_channel_vol(0,mic_vol);
	switch(mode)
	{
	case SOURCE_SELECT_BT:
		set_adc_channel_vol(1,0);
		//set_adc_channel_vol(2,mix_vol);
		set_adc_channel_vol(2,ADC_CHANNEL_VOL_MIX);
		set_adc_channel_vol(3,0);
		break;
	case SOURCE_SELECT_USB:
	case SOURCE_SELECT_SD:
		set_adc_channel_vol(1,0);
		set_adc_channel_vol(2,0);
		//set_adc_channel_vol(3,mix_vol);
		set_adc_channel_vol(3,ADC_CHANNEL_VOL_USB);
		break;
	case SOURCE_SELECT_SPDIFIN:
	case SOURCE_SELECT_HDMI:
	case SOURCE_SELECT_COA:
		set_adc_channel_vol(1,0);
		set_adc_channel_vol(2,0);
		//set_adc_channel_vol(3,mix_vol);
		set_adc_channel_vol(3,ADC_CHANNEL_VOL_MIX);
		break;

	case SOURCE_SELECT_FM:
	case SOURCE_SELECT_LINEIN:
		//set_adc_channel_vol(1,mix_vol);
		set_adc_channel_vol(1,ADC_CHANNEL_VOL_MIX);
	    set_adc_channel_vol(2,0);
		set_adc_channel_vol(3,0);
		break;
	}

	player_process_cmd(NP_CMD_VOLUME_SET, NULL,mix_vol, NULL, NULL);
}


/*********************************************



*********************************************/
void enter_mode( int mode)
{
	char *vol_str;

	set_adc_channel_vol(0,0);
	set_adc_channel_vol(1,0);
	set_adc_channel_vol(2,0);
	set_adc_channel_vol(3,0);
	player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
	pa_mute_ctrl(true);
	unmute_count = 0;
	//mute_state = false;
	usleep(1000);

	mq_msg_clear();
#if 1
#if (BYPASS_MODE || INNERADC_MODE)
	player_process_cmd(NP_CMD_LINEIN_OFF, NULL, 0, NULL, NULL);
#else
	player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 1, NULL, NULL);
#endif
	usleep(1000);
	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
	usleep(1000);
	player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);
#endif


	display_ui_main_sys(ui_source_select);

	set_ui_media(ui_source_select);

	save_player_info();
	save_mix_vol();
	////////////////////////////////////
	usleep(1000);
	dis_play_update();


	switch(mode)
	{
		case SOURCE_SELECT_BT:
			printf("enter bt mode\n");
			bt_power_crt(true);

			handle_bt_cmd(AT_QUERY_CONNECT_STATE, 0);
			usleep(60000);

			//handle_bt_cmd(AT_DISCONNECT, 0);
			//usleep(200000);
			//handle_bt_cmd(AT_IS, 0);
			//usleep(500000);
			//handle_bt_cmd(AT_CONNECT_LAST_DEVICE, 0);

			//if(bt_connected==false)
			//handle_bt_cmd(AT_CONNECT_LAST_DEVICE, 0);

#if BYPASS_MODE
			if(bypass_in_vol > BYPASS_VOL_IN_MIN)
			{
				silan_set_linebypass_enable();
				usleep(1000);
			}
			bypass_vol_offset = 2;
			float vol = bypass_in_vol + bypass_vol_offset;
			//设置输入音量
			silan_set_hardware_gilr_volume(vol, vol);
			//设置输出音量
			silan_set_hardware_golr_volume(6, 6);
#endif


			player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_EXTRA_MASTER_IN, 2, 0);
			//player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_INNER_IN, 2, 0);

#if (BYPASS_MODE || INNERADC_MODE)
			player_process_cmd(NP_CMD_LINEIN_ON, NULL, 0, NULL, NULL);
#else
			player_process_cmd(NP_CMD_I2SIN_CHAN, NULL, 2, NULL, NULL);//////////adc2
			usleep(1000);
			player_process_cmd(NP_CMD_I2SIN_OPEN, NULL,1, NULL, NULL);////////adc0
#endif

			handle_bt_cmd(AT_IS, 0);
			usleep(60000);
			handle_bt_cmd(AT_IS, 0);
			usleep(60000);
			//handle_bt_cmd(AT_CONNECT_LAST_DEVICE, 0);


			usleep(1000);
			/*蓝牙固定48k数据，8836需设定48k采样率*/
			//sl_ui_set_bt_samp(1);
			//usleep(1000);
			sl_ui_set_reqrate();
			set_channel_vol_by_mode(SOURCE_SELECT_BT);

			break;

		case SOURCE_SELECT_USB:
			sl_ui_set_reqrate();

			set_channel_vol_by_mode(SOURCE_SELECT_USB);

			handle_local(SEARCH_USB_NAME);
			break;

		case SOURCE_SELECT_SD:


			sl_ui_set_reqrate();
			set_channel_vol_by_mode(SOURCE_SELECT_SD);

			handle_local(SEARCH_SD_NAME);
			break;

		case SOURCE_SELECT_FM:
			FM_Mode();
			pcm1803_power_crt(true);
			aux_fm_channel_choose(false);
			sl_ui_set_reqrate();

			player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_EXTRA_MASTER_IN, 2, 0);
			player_process_cmd(NP_CMD_I2SIN_CHAN, NULL, 1, NULL, NULL);//////////adc1
			usleep(1000);

#if (BYPASS_MODE || INNERADC_MODE)
			player_process_cmd(NP_CMD_LINEIN_ON, NULL, 0, NULL, NULL);
#else
			player_process_cmd(NP_CMD_I2SIN_OPEN, NULL, 1, NULL, NULL);//////////adc1
#endif
			usleep(1000);
			set_channel_vol_by_mode(SOURCE_SELECT_FM);
			break;


		case SOURCE_SELECT_LINEIN:
			pcm1803_power_crt(true);
			aux_fm_channel_choose(true);
			sl_ui_set_reqrate();

#if BYPASS_MODE
			bypass_vol_offset = 0;
			float vol = bypass_in_vol + bypass_vol_offset;

			silan_set_hardware_gilr_volume(vol, vol);
			silan_set_hardware_golr_volume(0, 0);
#endif
			player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_EXTRA_MASTER_IN, 2, 0);
			player_process_cmd(NP_CMD_I2SIN_CHAN, NULL, 1, NULL, NULL);//////////adc1
			usleep(1000);



#if (BYPASS_MODE || INNERADC_MODE)
			player_process_cmd(NP_CMD_LINEIN_ON, NULL, 0, NULL, NULL);
#else
			player_process_cmd(NP_CMD_I2SIN_OPEN, NULL, 1, NULL, NULL);///////adc1
#endif
			usleep(1000);
			set_channel_vol_by_mode(SOURCE_SELECT_LINEIN);

			printf("SOURCE_LINE_IN\r\n");
			break;


		case SOURCE_SELECT_SPDIFIN:
#if BYPASS_MODE
			silan_set_hardware_gilr_volume(0, 0);
			silan_set_linebypass_disable();
			usleep(1000);
#endif
			sl_ui_set_reqrate();

			player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_EXTRA_MASTER_IN, 2, 0);
			usleep(1000);
			player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
			usleep(1000);
			set_channel_vol_by_mode(SOURCE_SELECT_SPDIFIN);

			break;

		case SOURCE_SELECT_HDMI:
			printf(">>>>>>>>>> hdmi mode <<<<<<<<<<\n");

			sl_ui_set_reqrate();

			action_hdmi_on();
#if 0
			if(frist_hdmi_init == true)
			{
				frist_hdmi_init = false;
				if(wdtimer_hdmion_send == NULL)
		        {
		            wdtimer_hdmion_send = wd_create();
		        }
				ui_hdmion_send();
			}
			else
			{
				wd_cancel(wdtimer_hdmion_send);
			}
#endif
			//if(hdmi_cec_online)//////

			player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_EXTRA_MASTER_IN, 2, 1);
			usleep(1000);
			player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
			usleep(1000);
			player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
			usleep(1000);
			set_channel_vol_by_mode(SOURCE_SELECT_HDMI);

			break;

		case SOURCE_SELECT_COA:
#if BYPASS_MODE
			silan_set_hardware_gilr_volume(0, 0);
			silan_set_linebypass_disable();
			usleep(1000);
#endif

			sl_ui_set_reqrate();

			player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_EXTRA_MASTER_IN, 2, 2);
			usleep(1000);
			/*切到同轴会出现overflow，先关闭再开就不会*/
			player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
			usleep(1000);
			player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
			usleep(1000);
			set_channel_vol_by_mode(SOURCE_SELECT_COA);

			break;



		case SOURCE_SELECT_START:
			ui_handle_power(POWER_OFF);////////power_off
			break;

	}



#if(MIC_ENABLE_ADC0==1)

	player_process_cmd(NP_CMD_I2SIN_OPEN, NULL,0, NULL, NULL);///////adc0

	if(mic_on_flag)
		mic_open(true);
	else
		mic_open(false);
#endif
/*
    usleep(200000);
    usleep(500000);
    usleep(500000);

	if (mute_state == UNMUTE)
	{
		pa_mute_ctrl(false);
		set_channel_vol_by_mode(mode);
	}
*/

}

/*********************************************



*********************************************/
void exit_mode( int mode)
{
	pa_mute_ctrl(true);
	usleep(10000);

	player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);

	///////////////////////////////////

	switch(mode)
	{
		case SOURCE_SELECT_BT:
			handle_bt_cmd(AT_DS, 0);
			usleep(60000);
			handle_bt_cmd(AT_DS, 0);
			usleep(60000);
			bt_power_crt(false);
			//handle_bt_cmd(AT_DISCONNECT, 0);
			//handle_bt_cmd(AT_STOP, 0);

			/*退出蓝牙模式，8836需设定44.1k采样率*/
			//sl_ui_set_bt_samp(0);

#if (BYPASS_MODE || INNERADC_MODE)
			player_process_cmd(NP_CMD_LINEIN_OFF, NULL, 0, NULL, NULL);
#else
			player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 1, NULL, NULL);
#endif
			usleep(100);
			break;

		case SOURCE_SELECT_USB:

			break;

		case SOURCE_SELECT_SD:

			break;

		case SOURCE_SELECT_FM:
			fm_rx_off();
			pcm1803_power_crt(false);

#if (BYPASS_MODE || INNERADC_MODE)
			player_process_cmd(NP_CMD_LINEIN_OFF, NULL, 0, NULL, NULL);
#else
			player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 1, NULL, NULL);
#endif
			usleep(100);
			break;

		case SOURCE_SELECT_LINEIN:
			pcm1803_power_crt(false);
#if (BYPASS_MODE || INNERADC_MODE)
			player_process_cmd(NP_CMD_LINEIN_OFF, NULL, 0, NULL, NULL);
#else
			player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 1, NULL, NULL);
#endif
			usleep(100);
			break;

		case SOURCE_SELECT_SPDIFIN:
			player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
			usleep(1000);
			break;

		case SOURCE_SELECT_HDMI:
			player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
			usleep(1000);
			action_hdmi_off();
			break;

		case SOURCE_SELECT_COA:
			player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
			usleep(1000);
			break;

		case SOURCE_SELECT_START:
			ui_handle_power(POWER_ON);
			break;

	}

}





/****************************************************************




****************************************************************/
unsigned char ui_handle_cmd_com(ui_cmd_t *cmd)
{
	if(cmd->cmd!=UI_CMD_NULL)
	printf("\n %s:cmd:%d \n", __func__, cmd->cmd);
	unsigned char ret=0;

	int arg1 = cmd->arg2;
	int arg2 = cmd->mode;
	int arg3 = cmd->arg.url;

	if(test_mode_flag == false)
	{
		switch(cmd->cmd)
		{
			case UI_CMD_POWER:
			case UI_CMD_POWER|CCHIP_KEY_LONG:
				dis_other_mode = 10;
				pa_mute_ctrl(true);
				display_ui_clear();
				ht1633_updata_display();
				#if 0
				if(ui_source_select == SOURCE_SELECT_HDMI)
				{
					action_hdmi_standby();
				}
				#endif
				//ui_source_select=source_mode_start;
				sys_power_control();
				ret=1;
				break;

			case UI_CMD_USB_IN:
				usb_online=1;
				if(ui_source_select!=SOURCE_SELECT_USB)
				{
					ui_handle_mode(SOURCE_SELECT_USB,0);
					ret=1;
				}
				break;

			case UI_CMD_SD_IN:
				sd_online=1;
				if(ui_source_select!=SOURCE_SELECT_SD)
				{
					ui_handle_mode(SOURCE_SELECT_SD,0);
					ret=1;
				}
				break;

			case UI_CMD_SD_LOAD:
				if(ui_source_select==SOURCE_SELECT_SD)
					handle_local(SEARCH_SD_NAME);
				break;

			case UI_CMD_USB_LOAD:
				if(ui_source_select==SOURCE_SELECT_USB)
					handle_local(SEARCH_USB_NAME);
				break;


			case UI_CMD_FILES_IS_LOAD:
				ui_handle_file_load(arg1,arg2,arg3);
				break;

			case UI_CMD_PLAYER_FINISH:
				ui_handle_next();
				break;

			case UI_CMD_USB_OUT:
				usb_online=0;
				ui_handle_usb_out();
				break;

			case UI_CMD_SD_UNLOAD:
				sd_online=0;
				ui_handle_sd_unload();
				break;


			case UI_CMD_VOLUME_MUTE:
				ui_handle_mute();
				break;

			case UI_CMD_MODE:
				ui_handle_mode(SOURCE_SELECT_INC,0);
				ret=1;
				break;

			case UI_CMD_BT_PAIR:
				bt_cmd_dis_connect();
				break;

			case UI_CMD_GO_TO_BT:
				ui_handle_mode(SOURCE_SELECT_BT,0);
				ret=1;
				break;

			case UI_CMD_GO_TO_USB:
				ui_handle_mode(SOURCE_SELECT_USB,0);
				ret=1;
				break;

			case UI_CMD_GO_TO_SD:
				ui_handle_mode(SOURCE_SELECT_SD,0);
				ret=1;
				break;

			case UI_CMD_GO_TO_AUX:
				ui_handle_mode(SOURCE_SELECT_LINEIN,0);
				ret=1;
				break;

			case UI_CMD_GO_TO_SPDIF:
				ui_handle_mode(SOURCE_SELECT_SPDIFIN,0);
				ret=1;
				break;

			case UI_CMD_GO_TO_HDMI:
				ui_handle_mode(SOURCE_SELECT_HDMI,0);
				ret=1;
				break;

			case UI_CMD_GO_TO_COA:
				ui_handle_mode(SOURCE_SELECT_COA,0);
				ret=1;
				break;

			case UI_CMD_GO_TO_FM:
				ui_handle_mode(SOURCE_SELECT_FM,0);
				ret=1;
				break;


			case UI_CMD_NUM_0:
			case UI_CMD_NUM_1:
			case UI_CMD_NUM_2:
			case UI_CMD_NUM_3:
			case UI_CMD_NUM_4:
			case UI_CMD_NUM_5:
			case UI_CMD_NUM_6:
			case UI_CMD_NUM_7:
			case UI_CMD_NUM_8:
			case UI_CMD_NUM_9:
				auto_input_cnt = 0;
				if(ui_source_select == SOURCE_SELECT_USB ||ui_source_select == SOURCE_SELECT_SD||ui_source_select == SOURCE_SELECT_FM)
				{
					input_n=cmd->cmd-UI_CMD_FILES_IS_LOAD-1;
					if (input_number > INPUT_MAX)
						input_number = 0;
					input_number = input_number * 10 + input_n;
					display_ui_input(input_number);
				}
				break;

			case UI_CMD_ENTER:
				auto_input_cnt = 100;
				if(ui_source_select == SOURCE_SELECT_USB ||ui_source_select == SOURCE_SELECT_SD)
				{
					if((input_number>0)&&(input_number<INPUT_MAX))
					{
						input_number_ok=input_number;
						input_number=0;
						input_n=0;
						ui_handle_play_num(input_number_ok);
					}
				}
				else if(ui_source_select == SOURCE_SELECT_FM)
				{

					if((input_number>=FM_MIN)&&(input_number<=FM_MAX))
					{
						input_number_ok=input_number;
						input_number=0;
						input_n=0;
						fm_rx_set_freq(input_number_ok);
						if (fm_rx_seek(input_number_ok))
						{
							fmFrequency=input_number_ok;
							fre_save();
						}
						display_ui_fm(0);
					}
					else
					{
						input_number_ok=input_number;
						input_number=0;
						input_n=0;
						fre_num_play(input_number_ok);
					}

				}
				input_number_ok	=0;
				break;

			case UI_CMD_EQ_TRE_ADD:

				treble_vol++;

				if(treble_vol >= 15)
				{
					treble_vol = 15;
				}
				printf("UI_CMD_EQ_TRB_ADD:%d\n",treble_vol);
				set_bass_treble_vol(2,treble_vol);
				display_ui_bass_vol(2,treble_vol);
				break;

			case UI_CMD_EQ_TRE_SUB:

				treble_vol--;

				if(treble_vol <= -15)
				{
					treble_vol = -15;
				}
				printf("UI_CMD_EQ_TRB_SUB:%d\n",treble_vol);
				set_bass_treble_vol(2,treble_vol);
				display_ui_bass_vol(2,treble_vol);
				break;

			case UI_CMD_EQ_BASS_ADD:
				bass_vol++;

				if(bass_vol >= 15)
				{
					bass_vol = 15;
				}
				printf("UI_CMD_EQ_BASS_ADD:%d\n",bass_vol);
				set_bass_treble_vol(0,bass_vol);
				display_ui_bass_vol(0,bass_vol);
				break;

			case UI_CMD_EQ_BASS_SUB:
				bass_vol--;
				if(bass_vol <= -15)
				{
					bass_vol = -15;
				}
				printf("UI_CMD_EQ_BASS_SUB:%d\n",bass_vol);
				set_bass_treble_vol(0,bass_vol);
				display_ui_bass_vol(0,bass_vol);
				break;

			case UI_CMD_AUX_CONNECT:
				aux_online=1;
				break;

			case UI_CMD_AUX_DISCONNECT:
				aux_online=0;
				break;

			case UI_CMD_BLUETOOTH_CONNECT:
				bt_set_connect_state(true);
				//display_ui_main_sys(ui_source_select);
				break;

			case UI_CMD_BLUETOOTH_DISCONNECT:
				bt_set_connect_state(false);
				break;

			case UI_CMD_HDMI_CONNECT:
				//if(ui_source_select == SOURCE_SELECT_HDMI)
				{
					ui_handle_mode(SOURCE_SELECT_HDMI,0);
					ret=1;
					//set_channel_vol_by_mode(SOURCE_SELECT_HDMI);
				}
				break;

			case UI_CMD_HDMI_DISCONNECT:
				//player_process_cmd(NP_CMD_VOLUME_SET, NULL,0, NULL, NULL);
				break;

			case UI_CMD_VOLUME_INC|CCHIP_KEY_LONG:
			case UI_CMD_VOLUME_INC|CCHIP_KEY_HOLD:
			case UI_CMD_VOLUME_INC:
				ui_handle_vol_up();
				break;

			case UI_CMD_VOLUME_DEC|CCHIP_KEY_LONG:
			case UI_CMD_VOLUME_DEC|CCHIP_KEY_HOLD:
			case UI_CMD_VOLUME_DEC:
				ui_handle_vol_down();
				break;


			case UI_CMD_VOLUME_SET:
				ui_handle_vol_set(cmd->arg2);
				break;

			case UI_CMD_VOLUME_INC_DOWN:
				if(ui_source_select == SOURCE_SELECT_BT)
				{
					bt_cmd_next_song();
				}
				else if(ui_source_select == SOURCE_SELECT_USB)
				{
					save_usb_play_time();
					ui_handle_next();
				}
				else if(ui_source_select == SOURCE_SELECT_FM)
				{
					fm_ch_add_sub(1);
				}
				else
				{
					ui_handle_vol_inc_long_press();
				}

		        break;

		    case UI_CMD_VOLUME_INC_UP:
		        ui_handle_vol_long_press_up();
		        break;

		    case UI_CMD_VOLUME_DEC_DOWN:
				if(ui_source_select == SOURCE_SELECT_BT)
				{
					bt_cmd_prev_song();
				}
				else if(ui_source_select == SOURCE_SELECT_USB)
				{
					save_usb_play_time();
					ui_handle_prev();
				}
				else if(ui_source_select == SOURCE_SELECT_FM)
				{
					fm_ch_add_sub(0);
				}
				else
				{
					ui_handle_vol_dec_long_press();
				}

		        break;

		    case UI_CMD_VOLUME_DEC_UP:
		        ui_handle_vol_long_press_up();
	        break;


#if(MIC_ENABLE_ADC0==1)
			case UI_CMD_MIC_ON:
				mic_on_flag^=0xff;
				if(mic_on_flag)
				{
					mic_open(true);
					display_ui_mic(true);
				}
				else
				{
					mic_vol_flag = false;
					mic_echo_flag = false;
					mic_open(false);
					display_ui_mic(false);
				}
			break;

#endif
			case UI_CMD_MIC_VOL_ADD:
					if(mic_on_flag)
					{
						mic_vol_flag = true;
						mic_echo_flag = false;
						if(mic_vol_lev < 60)
							mic_vol_lev++;

						mic_vol = mic_vol_table[mic_vol_lev];

						set_adc_channel_vol(0,mic_vol);

						display_mic_vol(mic_vol_lev);
						printf("mic_vol_add\n");
					}
					break;

			case UI_CMD_MIC_VOL_SUB:
					if(mic_on_flag)
					{
						mic_vol_flag = true;
						mic_echo_flag = false;

						if(mic_vol_lev > 0)
							mic_vol_lev--;

						mic_vol = mic_vol_table[mic_vol_lev];

						set_adc_channel_vol(0,mic_vol);

						display_mic_vol(mic_vol_lev);
						printf("mic_vol_sub\n");
					}
					break;

			case UI_CMD_ECHO_ADD:
					if(mic_on_flag)
					{
						mic_echo_flag = true;
						mic_vol_flag = false;
						if(echo_vol_lev >= 30)
						{
							echo_vol_lev = 30;
						}
						else
						{
							echo_vol_lev += 1;
						}
						set_echo_vol(echo_vol_lev);
						display_mic_vol(echo_vol_lev);
						printf("mic_echo_add\n");
					}
					break;

			case UI_CMD_ECHO_SUB:
					if(mic_on_flag)
					{
						mic_echo_flag = true;
						mic_vol_flag = false;
						if(echo_vol_lev <= 0)
						{
							echo_vol_lev = 0;
						}
						else
						{
							echo_vol_lev -= 1;
						}
						set_echo_vol(echo_vol_lev);
						display_mic_vol(echo_vol_lev);
						printf("mic_echo_sub\n");
					}
					break;

			case UI_CMD_SYS_RESET:
					sl_ui_system_reset();
					break;

			case UI_CMD_CHANGE_MODE_UNMUTE:
				if (mute_state == UNMUTE)
				{
					pa_mute_ctrl(false);
					set_adc_channel_vol(0,0);
					set_adc_channel_vol(1,0);
					set_adc_channel_vol(2,0);
					set_adc_channel_vol(3,0);
					player_process_cmd(NP_CMD_VOLUME_SET, NULL,0, NULL, NULL);
				}
				break;

			case UI_CMD_CHANGE_MODE_VOL_REC:
				if (mute_state == UNMUTE)
				{
					set_channel_vol_by_mode(ui_source_select);
				}
				break;

			case UI_CMD_ENTER_TREBLE_SET:
				enter_tre_set = true;
				tre_bass_cnt = 0;
				display_ui_enter_tre_bass(0);
				break;

			case UI_CMD_ENTER_BASS_SET:
				enter_bass_set = true;
				tre_bass_cnt = 0;
				display_ui_enter_tre_bass(1);
				break;

			case UI_CMD_HALF_SECOND:
				//if(!dis_other_mode)
				//{
				//display_ui_bt();
				//ui_update_music_time();
				//}
				break;

			case UI_CMD_SECOND:
				//if((0<dis_other_mode)&&(dis_other_mode<3))
				//{

				//dis_other_mode=3;
				//ui_goback_source(400);
				//}
				break;

			case UI_CMD_LOW_BAT:
				break;

		}
	}
	else
	{
		switch(cmd->cmd)
		{
			case UI_CMD_GO_TO_TEST:
				ui_handle_mode(SOURCE_SELECT_TEST,0);
				ret=1;
				break;
		}
	}

	return ret;
}

/****************************************************************




****************************************************************/
void source_mode_bt(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;

	enter_mode(SOURCE_SELECT_BT);

	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_PLAY_PAUSE:
				bt_cmd_play_pause();
				break;

			case UI_CMD_NEXT:
				bt_cmd_next_song();
				break;

			case UI_CMD_PREV:
				bt_cmd_prev_song();
				break;

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;
		}

		if(ret)
		{
			if(ui_source_select!=SOURCE_SELECT_BT)
			{
				exit_mode(SOURCE_SELECT_BT);
				return ;
			}
		}
	}
}

/****************************************************************




****************************************************************/
void source_mode_usb(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;

	enter_mode(SOURCE_SELECT_USB);

	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_PLAY_PAUSE:
				ui_handle_pause_play();
				break;

			case UI_CMD_NEXT:
				save_usb_play_time();
				ui_handle_next();
				break;

			case UI_CMD_PREV:
				save_usb_play_time();
				ui_handle_prev();
				break;

			case UI_CMD_FOLDER_NEXT:
				save_usb_play_time();
				ui_handle_folder_next();
				break;

			case UI_CMD_FOLDER_PREV:
				save_usb_play_time();
				ui_handle_folder_prev();
				break;

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;
		}

		if(ret)
		{
			if(ui_source_select!=SOURCE_SELECT_USB)
			{
				exit_mode(SOURCE_SELECT_USB);
				return ;
			}
		}

	}
}

/****************************************************************




****************************************************************/
void source_mode_sd(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;
	enter_mode(SOURCE_SELECT_SD);

	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_PLAY_PAUSE:
				ui_handle_pause_play();
				break;

			case UI_CMD_NEXT:
				ui_handle_next();
				break;

			case UI_CMD_PREV:
				ui_handle_prev();
				break;

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;

		}

		if(ret)
		{
			if(ui_source_select!=SOURCE_SELECT_SD)
			{
				exit_mode(SOURCE_SELECT_SD);
				return ;
			}
		}


	}
}

/****************************************************************




****************************************************************/
void source_mode_fm(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;
	enter_mode(SOURCE_SELECT_FM);

	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_FM_TUNE_ADD:
				fm_fre_add_sub(1);
				break;

			case UI_CMD_FM_TUNE_SUB:
				fm_fre_add_sub(0);
				break;

			case UI_CMD_FM_SCAN:
				FmScan();
				break;

			case UI_CMD_PLAY_PAUSE:
				FmScan();
				//put_ui_msg(UI_CMD_FM_SCAN);
				break;

			case UI_CMD_NEXT:
				fm_ch_add_sub(1);
				break;

			case UI_CMD_PREV:
				fm_ch_add_sub(0);
				break;

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;
		}

		if(ret)
		{
			if(ui_source_select!=SOURCE_SELECT_FM)
			{
				exit_mode(SOURCE_SELECT_FM);
				return ;
			}
		}


	}
}


/****************************************************************




****************************************************************/
void source_mode_aux(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;
	enter_mode(SOURCE_SELECT_LINEIN);

	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_PLAY_PAUSE:
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;
		}

		if(ret)
		{
			if(ui_source_select!=SOURCE_SELECT_LINEIN)
			{
				exit_mode(SOURCE_SELECT_LINEIN);
				return ;
			}
		}


	}
}

/****************************************************************




****************************************************************/
void source_mode_spdifin(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;

	enter_mode(SOURCE_SELECT_SPDIFIN);
	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_PLAY_PAUSE:
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;
		}

		if(ret)
		{
			if(ui_source_select!=SOURCE_SELECT_SPDIFIN)
			{
				exit_mode(SOURCE_SELECT_SPDIFIN);
				return ;
			}
		}

	}
}



/****************************************************************




****************************************************************/
void source_mode_hdmi(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;

	enter_mode(SOURCE_SELECT_HDMI);

	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_PLAY_PAUSE:
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			case UI_CMD_HDMION_SEND:
				action_hdmion_send();
		    	break;

			case UI_CEC_INACTIVE_SOURCE:
				sl_ui_handle_cec_inactive_source();
	   			break;

			case UI_CEC_ACTIVE_SOURCE:
				//sl_ui_handle_cec_active_source();
	   			break;

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;
		}

		if(ret)
		{
			if(ui_source_select!=SOURCE_SELECT_HDMI)
			{
				exit_mode(SOURCE_SELECT_HDMI);
				return ;
			}
		}
	}
}


/****************************************************************




****************************************************************/
void source_mode_coaxial(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;
	enter_mode(SOURCE_SELECT_COA);

	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_PLAY_PAUSE:
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;
		}

		if(ret)
		{
			if(ui_source_select!=SOURCE_SELECT_COA)
			{
				exit_mode(SOURCE_SELECT_COA);
				return ;
			}
		}


	}
}


/*********************************************************



**********************************************************/
void source_mode_test(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;

	display_ui_full();
	ht1633_updata_display();
	usleep(500000);
	handle_bt_cmd(AT_DISCONNECT, 0);
	usleep(200000);
	handle_bt_cmd(AT_DISCONNECT, 0);
	usleep(200000);
	mix_vol = 4;
    bt_mix_vol = 2;
	display_ui_init();
	ht1633_updata_display();
	//enter_mode(SOURCE_SELECT_BT);
	pa_mute_ctrl(false);

	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_POWER:
				test_mode_flag = false;
				pa_mute_ctrl(true);
				display_ui_clear();
				ht1633_updata_display();
				sys_power_control();
				ret=1;
				break;

			case UI_CMD_PLAY_PAUSE:
				//bt_cmd_play_pause();
				break;

			case UI_CMD_VOLUME_INC:
				if(fm_test_flag == true)
				{
					fm_fre_test_add_sub(1);
				}
				else
				{
					display_ui_ledtest();
				}
				break;

			case UI_CMD_VOLUME_DEC:
				if(fm_test_flag == true)
				{
					fm_fre_test_add_sub(0);
				}
				else
				{
					display_ui_ledtest();
				}
				break;

			case UI_CMD_VOLUME_INC_DOWN://MCU VERSION
				display_ui_version(MCU_VERSION);
		        break;

			case UI_CMD_VOLUME_DEC_DOWN://BT VERSION
				display_ui_version(BT_VERSION);
		        break;

			case UI_CMD_BT_PAIR:
				sl_ui_fm_test();
				break;

			case UI_CMD_LED_TEST:
				if(fm_test_flag == false)
				{
					display_ui_ledtest();
				}
				break;

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;
		}

		if(ret)
		{
			if(ui_source_select!=SOURCE_SELECT_TEST)
			{
				exit_mode(SOURCE_SELECT_TEST);
				return ;
			}
		}
	}
}


/*********************************************************



**********************************************************/
void source_mode_start(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;
	enter_mode(SOURCE_SELECT_START);
	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();
		switch(cmd.cmd)
		{
			case UI_CMD_POWER:
			case UI_CMD_POWER|CCHIP_KEY_LONG:
				ret=1;
				break;
		}

		if(ret)
		{
			exit_mode(SOURCE_SELECT_START);
			return ;
		}
	}
}


