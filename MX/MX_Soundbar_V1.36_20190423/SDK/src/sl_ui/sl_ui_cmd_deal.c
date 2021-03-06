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

#include "sc6138_led7.h"
//#include "ht1633.h"
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

#define PCM2452_Vol_Lowlev             -2
#define PCM2452_Vol_Normallev             0

#define MCU_VERSION 0
#define BT_VERSION           1

int mic_vol = 45;
int echo_vol_lev = 5;
int mic_vol_lev = 15;
extern int mute_state;


static int parameter_echo[ECHO_SET]   ={1, -600, 200, 0, 0, 1, 255};  //{0, -600, 300, 0, 0, 5, 0};

#if 0
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
#endif

#define MIC_LEV_CNT 30
#define MIC_ECHOLEV_CNT 15

static const int mic_vol_table[31]={
	0, 3, 6, 9, 12, 15,
	18, 21, 24, 27,30,
	33, 36, 39,42,45,
	48, 51, 54, 57,60,
	64, 68, 72,76,80,
	84, 88, 92,96,100
};

static const int mic_auxvol_table[31]={
	0, 2, 4, 6, 8, 10,
	12, 14, 16, 18,20,
	23, 26, 29,33,36, 
	39, 42, 45,48, 51, 
	54, 57, 60, 63, 66, 
	69, 72,75,78,80
};

#define SPDIFIN_ADC_CHANNEL              3
#define SPDIFIN_ADC_VALUE                20



char mic_vol_flag = false;
char mic_echo_flag = false;

char mic_on_flag=0;

char movie_on_flag=0;

extern  int mix_vol ;
extern int bt_mix_vol;
extern bool bt_connected;

unsigned int  input_number=0;
unsigned int  input_number_ok=0;
char input_n=0;
int auto_input_cnt = 100;

int  bass_vol = 0;
int  treble_vol = 0;

int unmute_count = 100;
bool enter_tre_set = false;
bool enter_bass_set = false;

bool change_mode_flag = false;

bool usb_is_load=false;



bool fm_test_flag = false;
//bool fm_manual_save_status = false;
//int fm_manual_save_cnt = 0;

bool frist_hdmi_init = false;

char eq_mode = EQ_NORMAL;

extern WDOG_ID wdtimer_hdmion_send;

extern bool usb_online;
extern bool sd_online;
extern bool aux_online;
extern bool hdmi_cec_online;
extern bool mic_detect_online;
extern int ui_source_select;
extern char dis_other_mode;

extern bool bt_wait_flag;
extern int bt_wait_cnt;

extern bool usb_prev_flag;


extern u16 fmFrequency;
extern u8 Cur_Fre_Num;
extern unsigned char power_on_usb_sd_auto_play;
extern  WDOG_ID wdtimer_display_updata;
extern WDOG_ID wdtimer_goback_mode;

//extern  WDOG_ID wdtimer_display_bt_wait;

extern u8 Fre_Total_Num;

extern unsigned char bt_version_num;

extern bool test_mode_flag;
extern int tre_bass_cnt;
extern bool folder_dis_flag;

extern int tre_bass_cnt;
extern int bt_wait_cnt;
extern int auto_input_cnt;
extern int save_ir_cnt;
//extern int fm_manual_save_cnt;
extern int usb_play_cnt;
extern int  mix_vol;
extern struct input_event save_ir_event;
extern bool enter_tre_set;
extern bool enter_bass_set;
extern bool ir_short_flag;
extern bool ir_long_flag;
extern bool bt_wait_flag;

extern bool app_mute_flag;

extern bool bt_ok_flag;

extern int usb_playtime;
extern int usb_last_file_index;

extern bool usb_play_flag;

extern int folder_index_cnt;
extern int folder_index_tab[255][2];

extern int file_rel_pos;

extern bool maxmin_vol_flag;

extern ui_cmd_t get_mq_msg(void);
extern void mq_msg_clear(void);
extern void  put_ui_msg(int ui_cmd);
extern void set_ui_media(int source);
extern void player_paramter_set_init(AUDIO_OUT_MODE outMode, AUDIO_IN_MODE inMode, int chnNum, int spdifNum);


void select_eq_mode(void)
{
	switch(eq_mode)
	{
		case EQ_NORMAL:
			pt2386_switch_ctrl(false);
			bt_cmd_eq_set();
			display_eq_mode(eq_mode);
			eq_mode = EQ_NIGHT;
			break;
			
		case EQ_NIGHT:
			pt2386_switch_ctrl(false);
			bt_cmd_eq_set();
			display_eq_mode(eq_mode);
			eq_mode = EQ_3D;
			break;
			
		case EQ_3D:
			pt2386_switch_ctrl(true);
			bt_cmd_eq_set();
			display_eq_mode(eq_mode);
			eq_mode = EQ_CLEAR;
			break;
		case EQ_CLEAR:
			pt2386_switch_ctrl(false);
			bt_cmd_eq_set();
			display_eq_mode(eq_mode);
			eq_mode = EQ_NORMAL;
			break;
	}
}


void select_mixvol_table(void)
{
#if 0
	#ifdef MIX_VOL_BT
	
	if(ui_source_select == SOURCE_SELECT_BT)
	{
		mix_vol = mix_vol_bt_table[bt_mix_vol];
	}
	else if(ui_source_select == SOURCE_SELECT_USB)
	{
		mix_vol = mix_vol_usb_table[bt_mix_vol];
	}
	else if((ui_source_select == SOURCE_SELECT_SPDIFIN)||(ui_source_select == SOURCE_SELECT_COA)||(ui_source_select == SOURCE_SELECT_HDMI))
	{
		mix_vol = mix_vol_opt_table[bt_mix_vol];
	}
	else
	{
		mix_vol = mix_vol_aux_table[bt_mix_vol];
	}
	#else
	mix_vol = mix_vol_table[bt_mix_vol];
	#endif
#endif
}

void sl_ui_fm_test(void)
{
	if(fm_test_flag == false)
	{
		fm_test_flag = true;

		//set_adc_channel_vol(1,0);
		//set_adc_channel_vol(2,0);
		//set_adc_channel_vol(3,0);
		//player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
		pa_mute_ctrl(true);
		usleep(100000);

		FM_Mode();
		usleep(10000);
		sl_ui_set_reqrate();

		//player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_EXTRA_MASTER_IN, 2, 0);
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

		usleep(100000);
		//set_adc_channel_vol(1,100);
	    //set_adc_channel_vol(2,0);
		//set_adc_channel_vol(3,0);
		//set_channel_vol_by_mode(ui_source_select);
		usleep(500000);
		//set_channel_mixvol_by_mode(ui_source_select);
		usleep(100000);
		pa_mute_ctrl(false);
		
	}
	else
	{
		fm_test_flag = false;
		fm_rx_off();
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
void  save_usb_num(int file_index)
{
#if 0
	unsigned char temp[4];
	int i;

	temp[0] = file_index/1000;
	temp[1] = (file_index%1000)/100;
	temp[2] = (file_index%100)/10;
	temp[3] = file_index%10;

	for(i = 0;i < 4;i++)
	{
		at24c02_write_one_byte(MEM_USB_NUM+i ,temp[i]);
		Delay5Ms(10);
	}
			
#endif
}


/******************************************





******************************************/
int  read_usb_num(void)
{
#if 0
	unsigned char temp[4];
	int file_index,i;

	for(i = 0;i < 4;i++)
	{
		if(temp[i] == 0xFF)
		{
			return -1;
		}
		temp[i]=at24c02_read_one_byte(MEM_USB_NUM+i);
	}
	
    file_index = temp[3]+temp[2]*10+temp[1]*100+temp[0]*1000;

	return file_index;
	
#endif
}




/******************************************





******************************************/
void  save_mix_vol(void)
{
#if 1
	unsigned char temp;

	if(bt_mix_vol <= MIX_LEV_CNT)
	{
		temp= (unsigned char)bt_mix_vol;
		
		at24c02_write_one_byte(MEM_MIX_VOL ,temp);
		Delay5Ms(10);
	}

	printf("save-mix_vol = %d\r\n",temp);
#endif
}


/******************************************





******************************************/
void  read_mix_vol(void)
{
#if 1
	unsigned char temp;
	temp=at24c02_read_one_byte(MEM_MIX_VOL);

	if(temp <= MIX_LEV_CNT)
	{
		bt_mix_vol = temp;
		//select_mixvol_table();
		//bt_cmd_current_mainvol();
	}
	else
	{
		bt_mix_vol = Frist_MIX_LEV;
		//select_mixvol_table();
		//bt_cmd_current_mainvol();
	}


	printf("read-mix_vol = %d\r\n",temp);
#endif
}


/******************************************





******************************************/
void  save_player_info(void)
{
#if 1
	unsigned char temp;

	temp= (unsigned char)ui_source_select;
	
	if(temp>=0)
	{
		at24c02_write_one_byte(MEM_WORK_MODE ,temp);
		Delay5Ms(10);
	}
	
	printf("save-ui_source_select=%d\r\n",temp);
#endif
}


/******************************************





******************************************/
void  read_player_info(void)
{
#if 1
	unsigned char temp;
	temp=at24c02_read_one_byte(MEM_WORK_MODE);
	
	if(temp>=SOURCE_SELECT_END)
	{
		temp=SOURCE_SELECT_BT;
	}
		
	ui_source_select=temp;

	//ui_source_select=SOURCE_SELECT_HDMI;

	printf("ui_source_select=%d\r\n",temp);
#endif
}




void hdmi_send_unmute(void)
{
	if (mute_state == UNMUTE)
	{
		sc8836_action_hdmi_soundbar_unmute_tv();
	}
	else
	{
		sc8836_action_hdmi_soundbar_mute_tv();
	}
	usleep(100000);
	sc8836_action_hdmi_soundbar_adj_tv_vol();
	usleep(100000);
	if (mute_state == UNMUTE)
	{
		//player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
		//set_channel_mixvol_by_mode(ui_source_select);
		//usleep(500000);
		pa_mute_ctrl(false);
	}
	
}


/************************************************




************************************************/
void dis_play_update(void)
{

	static unsigned char second_count=0;
	static unsigned char wait_count=0;
	static bool is_Disp = true;
	ui_cmd_t cmd;

	//int energy[6];
	if(bt_ok_flag)
	{
		//printf("dis_other_mode == %d.\n",dis_other_mode);
		if(!dis_other_mode)
		{
			if(!mute_state)
			{
				if(bt_wait_flag == false)
				{
					display_ui_bt();
				}
				else
				{
					wait_count++;
					if(wait_count==2)
					{
						wait_count=0;
						display_ui_bt();
					}
				}
				if(folder_dis_flag == false)
				{
					ui_update_music_time();
				}
			}
		}

		
		if(dis_other_mode == 100)
		{
			//printf("dis_other_mode == %d.\n",dis_other_mode);
			display_ui_maxmin_vol(bt_mix_vol);
		}


		////////////////////////////////////////////

		second_count++;
		if(second_count==2)
		{
			second_count=0;
			//printf("dis_other_mode == %d.\n",dis_other_mode);
			//swa_audio_audproc_get_energy(AUDPROC_LIST_DEC, &energy);
			//printf("energy[0] = %d, energy[1] = %d.\n", energy[0], energy[1]);

			if(dis_other_mode < 4)
			{		
				display_ui_mute();
				//printf("display_ui_mute.\n");
			}

			if(dis_other_mode == 5)
			{
				display_ui_woofer();
			}
			
			if((0<dis_other_mode)&&(dis_other_mode<3))
			{
				dis_other_mode=3;
				ui_goback_source(400);
			}

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
#if 0
	char * mic_str;
	if(on_off)
	{
		usleep(1000);
		mic_vol = mic_vol_table[mic_vol_lev];
		set_adc_channel_vol(0,mic_vol);
		bt_cmd_current_micvol();
		//usleep(1000);
	}
	else
	{
		usleep(1000);
		set_adc_channel_vol(0,0);
		usleep(1000);
	}
	//usleep(1000);
#endif
}

void set_echo_vol(int vol)
{
	char parg[256] = {0};
	//int *echovol = &vol;
	
	if(vol == 0)
	{
		parameter_echo[ECHO_DELAY] = 0;
	}
	else
	{
		parameter_echo[ECHO_DELAY] = 200;
	}
	
	if(vol == 15)
	{
		vol = 14;
	}

	//echo_vol_lev = *echovol;
	
	parameter_echo[ECHO_GAIN] =- (3000-(vol*200));

	player_process_cmd(NP_CMD_UNLOAD_MIC_ECHO, NULL, 0, NULL, NULL);

	sprintf(parg, "%d %d %d %d %d %d", \
			parameter_echo[ECHO_TYPE], parameter_echo[ECHO_GAIN], \
			parameter_echo[ECHO_DELAY], parameter_echo[ECHO_STYPE], \
			parameter_echo[ECHO_SHIFT], parameter_echo[ECHO_NUMBER]);

	//printf("echo param :%s\r\n", parg);

	player_process_cmd(NP_CMD_LOAD_MIC_ECHO, NULL, 0, NULL, NULL);
	player_process_cmd(NP_CMD_SET_MIC_ECHO, parg, 0, NULL, NULL);

}


void set_bass_treble_vol(int mode,int vol,int dis_flag)//mode=0:bass  mode = 2:treble
{
	if (vol < BASS_TREBLE_LEVEL_MIN)
	{
		vol = BASS_TREBLE_LEVEL_MIN;
	}		
	else if (vol > BASS_TREBLE_LEVEL_MAX)
	{
		vol = BASS_TREBLE_LEVEL_MAX;
	}
	//printf("%s:vol = %d.\n",__func__,vol);
	
	handle_bass_treble(mode, vol);

	if(dis_flag)
	{
		display_ui_bass_vol(mode, vol);
	}
}

void set_micvol_level(int vol)
{
#if 0
		/*
		if((ui_source_select == SOURCE_SELECT_FM)||(ui_source_select == SOURCE_SELECT_LINEIN))
		{
			mic_vol = mic_auxvol_table[mic_vol_lev];
		}
		else
		{
			mic_vol = mic_vol_table[mic_vol_lev];
		}
		*/
		int *micvol = &vol;

		mic_vol = mic_vol_table[mic_vol_lev];

		mic_vol_lev = *micvol;
		
		if(mic_detect_online)
		{
			set_adc_channel_vol(0,mic_vol);
		}

#endif		
}

#endif

/****************************************************************

第一个参数0----一路I2Sin，1---2路i2sin
第二个参数，输入模式，
第三个参数，输出模式
第四个参数，输入采样率
第5个参数，  输出采样率
都是字符


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
鏀归煶閲忓お澶х殑闂锛屽浐瀹歝hannel闊抽噺锛岃皟鑺傛�婚煶閲廚P_CMD_VOLUME_SET锛?*/
#define ADC_CHANNEL_VOL_BT 41
#define ADC_CHANNEL_VOL_MIX 24
#define ADC_CHANNEL_VOL_USB 16
#define ADC_CHANNEL_VOL_AUX 80

void set_channel_vol_by_mode(int mode)
{
#if 0
	switch(mode)
	{
		case SOURCE_SELECT_BT:
			set_adc_channel_vol(1,0);
			set_adc_channel_vol(2,mix_vol);
			set_adc_channel_vol(3,0);
			break;
		case SOURCE_SELECT_USB:
		case SOURCE_SELECT_SD:
			set_adc_channel_vol(1,0);
			set_adc_channel_vol(2,0);
			set_adc_channel_vol(3,mix_vol);
			break;
		case SOURCE_SELECT_SPDIFIN:
		case SOURCE_SELECT_HDMI:
		case SOURCE_SELECT_COA:
			set_adc_channel_vol(1,0);
			set_adc_channel_vol(2,0);
			set_adc_channel_vol(3,mix_vol);
			break;

		case SOURCE_SELECT_FM:
		case SOURCE_SELECT_LINEIN:
		case SOURCE_SELECT_TEST:	
			set_adc_channel_vol(1,mix_vol);
		    set_adc_channel_vol(2,0);
			set_adc_channel_vol(3,0);
			break;
	}
#endif
}

/*********************************************



*********************************************/

/*
鏀归煶閲忓お澶х殑闂锛屽浐瀹歝hannel闊抽噺锛岃皟鑺傛�婚煶閲廚P_CMD_VOLUME_SET锛?*/
#define ADC_CHANNEL_MIXVOL_BT 39
#define ADC_CHANNEL_MIXVOL_MIX 39
#define ADC_CHANNEL_MIXVOL_USB 39
#define ADC_CHANNEL_MIXVOL_AUX 50

void set_channel_mixvol_by_mode(int mode)
{
	switch(mode)
	{
		case SOURCE_SELECT_BT:
			player_process_cmd(NP_CMD_VOLUME_SET, NULL, ADC_CHANNEL_MIXVOL_BT, NULL, NULL);
			break;
		
		case SOURCE_SELECT_USB:
		case SOURCE_SELECT_SD:
			player_process_cmd(NP_CMD_VOLUME_SET, NULL, ADC_CHANNEL_MIXVOL_USB, NULL, NULL);
			break;
		
		case SOURCE_SELECT_SPDIFIN:
		case SOURCE_SELECT_HDMI:
		case SOURCE_SELECT_COA:
			player_process_cmd(NP_CMD_VOLUME_SET, NULL, ADC_CHANNEL_MIXVOL_MIX, NULL, NULL);
			break;

		case SOURCE_SELECT_FM:
		case SOURCE_SELECT_LINEIN:
		case SOURCE_SELECT_TEST:	
			player_process_cmd(NP_CMD_VOLUME_SET, NULL, ADC_CHANNEL_MIXVOL_AUX, NULL, NULL);
			break;
	}

}


/*********************************************



*********************************************/
void enter_mode( int mode)
{

	printf("%s %d\n", __func__, ui_source_select);
	char *vol_str;

	change_mode_flag = false;

	player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
	pa_mute_ctrl(true);
	usleep(10000);
	
	//select_mixvol_table();

	bt_cmd_current_mainvol();

	mq_msg_clear();

#if (BYPASS_MODE || INNERADC_MODE)
	player_process_cmd(NP_CMD_LINEIN_OFF, NULL, 0, NULL, NULL);
#else
	player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
#endif
	usleep(1000);
	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
	usleep(1000);
	player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);
	usleep(1000);

	display_ui_main_sys(ui_source_select);


	save_player_info();
	save_mix_vol();
	
	//if(ui_source_select != SOURCE_SELECT_USB)
	//{
	//	dis_other_mode=1;
	//}

	set_ui_media(ui_source_select);

	
	if(bt_version_num == 0)
	{
		bt_cmd_get_version();
	}
	
	
	////////////////////////////////////
	usleep(1000);
	//set_channel_vol_by_mode(ui_source_select);
	//usleep(1000);

	switch(mode)
	{
		case SOURCE_SELECT_BT:
			printf(">>>>>>>>>> bt mode <<<<<<<<<<\n");

			handle_bt_cmd(AT_QUERY_CONNECT_STATE, 0);
			usleep(60000);

			bt_wait_cnt = 0;
			bt_wait_flag = false;

			//handle_bt_cmd(AT_CONNECT_LAST_DEVICE, 0);
			break;

		case SOURCE_SELECT_USB:
			printf(">>>>>>>>>> usb mode <<<<<<<<<<\n");
			sl_ui_set_reqrate();
			usleep(1000);	
			handle_local(SEARCH_USB_NAME);
			break;

		case SOURCE_SELECT_SD:
			printf(">>>>>>>>>> sd mode <<<<<<<<<<\n");
			sl_ui_set_reqrate();
			usleep(1000);
			handle_local(SEARCH_SD_NAME);
			break;

		case SOURCE_SELECT_FM:
			printf(">>>>>>>>>> fm mode <<<<<<<<<<\n");
			switch_4052_function(FM_4052);
			FM_Mode();
			break;


		case SOURCE_SELECT_LINEIN:
			printf(">>>>>>>>>> aux mode <<<<<<<<<<\n");
			switch_4052_function(AUX_4052);
			break;

		case SOURCE_SELECT_RCA:
			printf(">>>>>>>>>> rca mode <<<<<<<<<<\n");
			switch_4052_function(RCA_4052);
			break;


		case SOURCE_SELECT_SPDIFIN:
			printf(">>>>>>>>>> opt mode <<<<<<<<<<\n");
			player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_INNER_IN, 2, 0);
			usleep(50000);
			sl_ui_set_reqrate();
			usleep(50000);
			player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
			usleep(50000);
			set_adc_channel_vol(SPDIFIN_ADC_CHANNEL,SPDIFIN_ADC_VALUE);
			usleep(10000);
			switch_4052_function(SPDF_4052);
			break;

		case SOURCE_SELECT_HDMI:
			printf(">>>>>>>>>> hdmi mode <<<<<<<<<<\n");

			sl_ui_set_reqrate();
			usleep(1000);
			sc8836_action_hdmi_on();

			usleep(50000);
			player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_INNER_IN, 2, 2);
			usleep(50000);
			player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
			usleep(50000);
			set_adc_channel_vol(SPDIFIN_ADC_CHANNEL,SPDIFIN_ADC_VALUE);
			usleep(10000);
			switch_4052_function(SPDF_4052);			
			wd_start(wdtimer_hdmion_send, 900, ui_hdmion_send, 0);
			break;

		case SOURCE_SELECT_COA:
			printf(">>>>>>>>>> coa mode <<<<<<<<<<\n");
			sl_ui_set_reqrate();
			usleep(50000);
			player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_INNER_IN, 2, 1);
			usleep(50000);
			player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
			usleep(50000);
			set_adc_channel_vol(SPDIFIN_ADC_CHANNEL,SPDIFIN_ADC_VALUE);
			usleep(10000);
			switch_4052_function(SPDF_4052);
			break;

		case SOURCE_SELECT_TEST:
			
			break;

		case SOURCE_SELECT_START:
			ui_handle_power(POWER_OFF);////////power_off
			break;

	}
	

	unmute_count = 0;
	change_mode_flag = true;
	usleep(100000);
	printf("%s:mode init success!\n", __func__);

	change_mode_unmute();
	
	//if(ui_source_select == SOURCE_SELECT_USB)
	//{
	//	dis_other_mode = 1;
	//}

}

/*********************************************



*********************************************/
void exit_mode( int mode)
{
	printf("%s %d\n", __func__, ui_source_select);

	pa_mute_ctrl(true);
	usleep(10000);

	player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);

	///////////////////////////////////

	switch(mode)
	{
		case SOURCE_SELECT_BT:
			break;

		case SOURCE_SELECT_USB:
			break;

		case SOURCE_SELECT_SD:
			break;

		case SOURCE_SELECT_FM:
			usleep(1000);
			fm_rx_off();
			usleep(1000);
			break;

		case SOURCE_SELECT_LINEIN:
			break;

		case SOURCE_SELECT_RCA:
			break;

		case SOURCE_SELECT_SPDIFIN:
			player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
			usleep(1000);
			break;

		case SOURCE_SELECT_HDMI:
			sc8836_action_hdmi_off();
			usleep(1000);
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

	int value;

	if(test_mode_flag == false)
	{
		switch(cmd->cmd)
		{
			case UI_CMD_POWER:
			case UI_CMD_POWER|CCHIP_KEY_LONG:
				
				if(ui_source_select == SOURCE_SELECT_HDMI)
				{
					hdmi_cec_online = false;
					sc8836_action_hdmi_off();
					usleep(1000);
				}
				pa_mute_ctrl(true);
				wd_cancel(wdtimer_goback_mode);
				wd_cancel(wdtimer_display_updata);
				dis_other_mode = 10;
				display_ui_clear();
				ht1633_updata_display();
				#if 0
				if(ui_source_select == SOURCE_SELECT_HDMI)
				{
					sc8836_action_hdmi_standby();
				}
				#endif
				//ui_source_select=source_mode_start;
				sys_power_control();
				ret=0;
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
				{
					if(!usb_is_load)
					{
						handle_local(SEARCH_USB_NAME);
					}						
				}
					
				break;


			case UI_CMD_FILES_IS_LOAD:
				ui_handle_file_load(arg1,arg2,arg3);
				break;

			case UI_CMD_PLAYER_FINISH:
				if(usb_prev_flag)
				{
					save_usb_play_time();
					ui_handle_prev();
				}
				else
				{
					save_usb_play_time();
					ui_handle_next();
				}				
				break;

			case UI_CMD_USB_OUT:
				usb_is_load = false;
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
				bt_cmd_source_select(ui_source_select);
				ret=1;
				break;

			case UI_CMD_BT_PAIR:
				bt_cmd_dis_connect();
				break;

			case UI_CMD_GO_TO_BT:
				while(1)
				{
					if(change_mode_flag == true)
					{
						ui_handle_mode(SOURCE_SELECT_BT,0);
						ret=1;
						break;
					}
				}				
				break;

			case UI_CMD_GO_TO_USB:
				while(1)
				{
					if(change_mode_flag == true)
					{
						ui_handle_mode(SOURCE_SELECT_USB,0);
						ret=1;
						break;
					}
				}
				break;

			case UI_CMD_GO_TO_SD:
				ui_handle_mode(SOURCE_SELECT_SD,0);
				ret=1;
				break;

			case UI_CMD_GO_TO_AUX2:
				while(1)
				{
					if(change_mode_flag == true)
					{
						ui_handle_mode(SOURCE_SELECT_LINEIN,0);
						ret=1;
						break;
					}
				}
				break;

			case UI_CMD_GO_TO_AUX1:
				while(1)
				{
					if(change_mode_flag == true)
					{
						ui_handle_mode(SOURCE_SELECT_RCA,0);
						ret=1;
						break;
					}
				}
				break;

			case UI_CMD_GO_TO_SPDIF:
				while(1)
				{
					if(change_mode_flag == true)
					{
						ui_handle_mode(SOURCE_SELECT_SPDIFIN,0);
						ret=1;
						break;
					}
				}
				break;

			case UI_CMD_GO_TO_HDMI:
				while(1)
				{
					if(change_mode_flag == true)
					{
						ui_handle_mode(SOURCE_SELECT_HDMI,0);
						ret=1;
						break;
					}
				}
				break;

			case UI_CMD_GO_TO_COA:
				while(1)
				{
					if(change_mode_flag == true)
					{
						ui_handle_mode(SOURCE_SELECT_COA,0);
						ret=1;
						break;
					}
				}
				break;

			case UI_CMD_GO_TO_FM:
				while(1)
				{
					if(change_mode_flag == true)
					{
						ui_handle_mode(SOURCE_SELECT_FM,0);
						ret=1;
						break;
					}
				}
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
				if(ui_source_select == SOURCE_SELECT_USB ||ui_source_select == SOURCE_SELECT_SD||ui_source_select == SOURCE_SELECT_FM)
				{
					input_n=cmd->cmd-UI_CMD_FILES_IS_LOAD-1;
					if (input_number > INPUT_MAX)
						input_number = 0;
					input_number = input_number * 10 + input_n;
					display_ui_input(input_number);
				}
				auto_input_cnt = 0;
				break;

			case UI_CMD_ENTER:
				auto_input_cnt = 100;
				
				if((enter_tre_set)||(enter_bass_set))
				{
					enter_tre_set = false;
					enter_bass_set = false;
					display_set_source(ui_source_select);
				}
			
				if(ui_source_select == SOURCE_SELECT_USB ||ui_source_select == SOURCE_SELECT_SD)
				{
					if((input_number>0)&&(input_number<=INPUT_MAX))
					{
						usb_prev_flag = false;
						if(folder_index_cnt < 1)
						{
							input_number_ok = input_number;
						}
						else
						{
							input_number_ok = input_number+(folder_index_tab[folder_index_cnt-1][1]+1);
						}
						input_number=0;
						input_n=0;
						//ui_handle_play_num(input_number_ok);
						#if 1
						//printf("%s:input_number_ok === %d\n", __func__,input_number_ok);
						if(input_number_ok < folder_index_tab[folder_index_cnt][1]+1)
						{
							//printf("%s:input_number_ok1 === %d\n", __func__,input_number_ok);
							ui_handle_play_num(input_number_ok);
						}
						else
						{
							//printf("%s:folder_index_tab[folder_index_cnt][1]+1 === %d\n", __func__,folder_index_tab[folder_index_cnt][1]+1);
							ui_handle_play_num(folder_index_tab[folder_index_cnt][1]+1);
						}
						#endif
					}
				}
				else if(ui_source_select == SOURCE_SELECT_FM)
				{
				#if 0
					if(fm_manual_save_status == false)
					{
						if((input_number>=FM_MIN)&&(input_number<=FM_MAX))
						{
							input_number_ok=input_number;
							input_number=0;
							input_n=0;
							fm_rx_set_freq(input_number_ok);
							//if (fm_rx_seek(input_number_ok))
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
					else
					{
						fm_manual_save_status = false;
						fm_manual_save_cnt = 200;
						Fre_Total_Num = Fre_Total_Num +1;
						SaveChan(Fre_Total_Num);
						Cur_Fre_Num = Fre_Total_Num;
						display_ui_fm(1);
						usleep(500000);
						display_ui_fm(0);
					}
				#else
					if((input_number>=FM_MIN)&&(input_number<=FM_MAX))
					{
						input_number_ok=input_number;
						input_number=0;
						input_n=0;
						fm_rx_set_freq(input_number_ok);
						//if (fm_rx_seek(input_number_ok))
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
				#endif
				}
				input_number_ok	=0;
				break;

			case UI_CMD_EQ_TRE_ADD:

				treble_vol++;

				if(treble_vol >= BASS_TREBLE_LEVEL_MAX)
				{
					treble_vol = BASS_TREBLE_LEVEL_MAX;
				}
				printf("UI_CMD_EQ_TRB_ADD:%d\n",treble_vol);
				set_bass_treble_vol(TREBLE_MODE,treble_vol,1);
				save_trebass_level(TREBLE_MODE);
				
				bt_cmd_current_treble(treble_vol); //treble
		
				//display_ui_bass_vol(TREBLE_MODE,treble_vol);
				break;

			case UI_CMD_EQ_TRE_SUB:

				treble_vol--;

				if(treble_vol <= BASS_TREBLE_LEVEL_MIN)
				{
					treble_vol = BASS_TREBLE_LEVEL_MIN;
				}
				printf("UI_CMD_EQ_TRB_SUB:%d\n",treble_vol);
				set_bass_treble_vol(TREBLE_MODE,treble_vol,1);
				save_trebass_level(TREBLE_MODE);
				
				bt_cmd_current_treble(treble_vol); //treble
				//display_ui_bass_vol(TREBLE_MODE,treble_vol);
				break;

			case UI_CMD_EQ_BASS_ADD:
				bass_vol++;

				if(bass_vol >= BASS_TREBLE_LEVEL_MAX)
				{
					bass_vol = BASS_TREBLE_LEVEL_MAX;
				}
				printf("UI_CMD_EQ_BASS_ADD:%d\n",bass_vol);
				set_bass_treble_vol(BASS_MODE,bass_vol,1);
				save_trebass_level(BASS_MODE);

				bt_cmd_current_bass(bass_vol); //treble
				//display_ui_bass_vol(BASS_MODE,bass_vol);
				break;

			case UI_CMD_EQ_BASS_SUB:
				bass_vol--;
				if(bass_vol <= BASS_TREBLE_LEVEL_MIN)
				{
					bass_vol = BASS_TREBLE_LEVEL_MIN;
				}
				printf("UI_CMD_EQ_BASS_SUB:%d\n",bass_vol);
				set_bass_treble_vol(BASS_MODE,bass_vol,1);
				save_trebass_level(BASS_MODE);

				bt_cmd_current_bass(bass_vol); //treble
				//display_ui_bass_vol(BASS_MODE,bass_vol);
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
					bt_cmd_source_select(ui_source_select);
					ret=1;
					//set_channel_vol_by_mode(SOURCE_SELECT_HDMI);
				}
				break;

			case UI_CMD_HDMI_DISCONNECT:
				if(ui_source_select == SOURCE_SELECT_HDMI)
				{
					sc8836_action_hdmi_off();
				}
				//printf(">>>>>>>>>>>HDMI is disconnect!>>>>>>>>>>\n");
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

			case UI_CMD_TREBLE_SET:
				treble_vol = (cmd->arg2)-5;
				set_bass_treble_vol(TREBLE_MODE,treble_vol,1);
				save_trebass_level(TREBLE_MODE);
				//display_ui_bass_vol(TREBLE_MODE,treble_vol);
				break;

			case UI_CMD_BASS_SET:
				bass_vol = (cmd->arg2)-5;
				set_bass_treble_vol(BASS_MODE,bass_vol,1);
				save_trebass_level(BASS_MODE);
				//display_ui_bass_vol(BASS_MODE,bass_vol);
				break;

			case UI_CMD_ECHO_SET:
				mic_vol_flag = false;
				mic_echo_flag = true;
			
				if(cmd->arg2 <= 15)
					echo_vol_lev = cmd->arg2;
				else
					echo_vol_lev = 15;
				display_mic_vol(echo_vol_lev);
				set_echo_vol(cmd->arg2);
				break;

			case UI_CMD_MICVOL_SET:
				mic_vol_flag = true;
				mic_echo_flag = false;
			
				if(mic_vol_lev <= 30)
					mic_vol_lev = cmd->arg2;
				else
					mic_vol_lev = 30;
				display_mic_vol(mic_vol_lev);
				set_micvol_level(cmd->arg2);
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
			case UI_CMD_MIC_CONNECT:
				printf("mic conncect!\n");
				if(mic_on_flag)
				{
					mic_open(true);
				}
				else
				{
					mic_open(false);
				}
				break;

			case UI_CMD_MIC_DISCONNECT:
				printf("mic disconnect!\n");
				mic_open(false);
				break;

			case UI_CMD_APP_MIC_ON:
				if(!mic_on_flag)
				{
					mic_on_flag^=0xff;	
					
					if(mic_detect_online)
					{
						mic_open(true);
						set_echo_vol(echo_vol_lev);
						bt_cmd_current_echo();
					}
					else
					{
						mic_open(false);
					}				
				}

				display_ui_mic(true);
				break;

			case UI_CMD_APP_MIC_OFF:
				if(mic_on_flag)
				{
					mic_on_flag^=0xff;	
					
					mic_vol_flag = false;
					mic_echo_flag = false;
					mic_open(false);
				}
					
				display_ui_mic(false);
				//bt_cmd_mic_status(mic_on_flag);
				break;
				
			case UI_CMD_MIC_ON:

				mic_on_flag^=0xff;	
			
				if(mic_on_flag)
				{
					if(mic_detect_online)
					{
						mic_open(true);
						set_echo_vol(echo_vol_lev);
						bt_cmd_current_echo();
					}
					else
					{
						mic_open(false);
					}

					display_ui_mic(true);
					//bt_cmd_mic_status(mic_on_flag);
					
				}
				else
				{
					mic_vol_flag = false;
					mic_echo_flag = false;
					mic_open(false);
					display_ui_mic(false);
					//bt_cmd_mic_status(mic_on_flag);
				}

				bt_cmd_mic_status(mic_on_flag);
			break;

#endif
			case UI_CMD_MIC_VOL_ADD:
					if(mic_on_flag)
					{
						mic_vol_flag = true;
						mic_echo_flag = false;
						if(mic_vol_lev < MIC_LEV_CNT)
							mic_vol_lev++;

						set_micvol_level(mic_vol_lev);

						bt_cmd_current_micvol();
						
						display_mic_vol(mic_vol_lev);

						if(ui_source_select == SOURCE_SELECT_USB)
						{
							save_usb_play_time();
						}
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

						set_micvol_level(mic_vol_lev);

						bt_cmd_current_micvol();

						display_mic_vol(mic_vol_lev);

						if(ui_source_select == SOURCE_SELECT_USB)
						{
							save_usb_play_time();
						}

						printf("mic_vol_sub\n");
					}
					break;

			case UI_CMD_ECHO_ADD:
					if(mic_on_flag)
					{
						mic_echo_flag = true;
						mic_vol_flag = false;
						if(echo_vol_lev >= MIC_ECHOLEV_CNT)
						{
							echo_vol_lev = MIC_ECHOLEV_CNT;
						}
						else
						{
							echo_vol_lev += 1;
						}
						set_echo_vol(echo_vol_lev);
						bt_cmd_current_echo();
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
						bt_cmd_current_echo();
						display_mic_vol(echo_vol_lev);
						printf("mic_echo_sub\n");
					}
					break;

			case UI_CMD_SYS_RESET:
					sl_ui_system_reset();
					if(ui_source_select == SOURCE_SELECT_BT)
					{
						pa_mute_ctrl(false);
					}
					else
					{
						ui_handle_mode(SOURCE_SELECT_BT,0);
					}				
					bt_cmd_source_select(ui_source_select);
					ret=1;
					break;

			case UI_CMD_CHANGE_MODE_UNMUTE:
				if(ui_source_select != SOURCE_SELECT_HDMI)
				{
					if(usb_play_flag == false)
					{
						if (mute_state == UNMUTE)
						{
							//set_channel_mixvol_by_mode(ui_source_select);
							//player_process_cmd(NP_CMD_VOLUME_SET, NULL,mix_vol, NULL, NULL);
						}
					}
					
				}
				break;

			case UI_CMD_CHANGE_MODE_VOL_REC:
				if(ui_source_select != SOURCE_SELECT_HDMI)
				{
					if(usb_play_flag == false)
					{
						if (mute_state == UNMUTE)
						{
							pa_mute_ctrl(false);
						}
					}
					else
					{
						usb_play_flag = false;
					}
					
				}
				
				
						
				break;

			case UI_CMD_ENTER_TREBLE_SET:
				enter_bass_set = false;
				enter_tre_set = true;
				tre_bass_cnt = 0;
				//display_ui_enter_tre_bass(0);
				set_bass_treble_vol(2,treble_vol,1);
				
				bt_cmd_current_treble(treble_vol); //treble
				//display_ui_bass_vol(2,treble_vol);
				break;

			case UI_CMD_ENTER_BASS_SET:
				enter_tre_set = false;
				enter_bass_set = true;
				tre_bass_cnt = 0;
				//display_ui_enter_tre_bass(1);
				set_bass_treble_vol(BASS_MODE,bass_vol,1);
				save_trebass_level(BASS_MODE);
				bt_cmd_current_bass(bass_vol); //treble
				//display_ui_bass_vol(0,bass_vol);
				break;
				
			case UI_CMD_APP_MOVIE_ON:
				if(!movie_on_flag)
				{
					movie_on_flag^=0xff;
					ui_handle_eq_movie();
				}
				display_ui_movie(true);
				break;

			case UI_CMD_APP_MOVIE_OFF:
				if(movie_on_flag)
				{
					movie_on_flag^=0xff;
					ui_handle_unload_eq();
				}
				display_ui_movie(false);
				break;

			case UI_CMD_MOVIE_ON:
				movie_on_flag^=0xff;
				if(movie_on_flag)
				{
					ui_handle_eq_movie();
					display_ui_movie(true);
				}
				else
				{
					ui_handle_unload_eq();
					display_ui_movie(false);
				}

				bt_cmd_movie_status(movie_on_flag);
				break;

			case UI_CMD_SET_SOURCE:
				
				if(ui_source_select == SOURCE_SELECT_USB)
				{
					save_usb_play_time();
				}
				
				display_set_source(ui_source_select);
			break;

			case UI_CMD_EQ_POWER_TEST:			
				display_ui_eq_power_test();
				display_set_source(ui_source_select);
				dis_other_mode=0;
			break;

			case UI_CMD_FM_SCAN_START:
				bt_cmd_fmscan_start();
				break;

			case UI_CMD_FM_SCAN_END:
				bt_cmd_fmscan_end();
				break;

			case UI_CMD_EQ_SELECT:
				display_ui_icon(ICON_EQ,1);
				select_eq_mode();
				break;

			case UI_CMD_WOOFER_PAIR:
				dis_other_mode = 5;
				bt_cmd_woofer_pair();
				break;

			case UI_CMD_DISPLAY_VER:
				display_ui_version(cmd->arg2);
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
				exit_mode(ui_source_select);	
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
			case UI_CMD_APP_PLAY:
			case UI_CMD_APP_PAUSE:
				bt_cmd_play_pause();
				break;

			case UI_CMD_BT_PAIR:
				bt_cmd_dis_connect();
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
		//printf("\n %s:cmd:%d \n", __func__, cmd.cmd);

		switch(cmd.cmd)
		{
			case UI_CMD_PLAY_PAUSE:
				ui_handle_pause_play(0,0);
				break;

			case UI_CMD_APP_PLAY:
				//ui_handle_pause_play(1,1);
				break;

			case UI_CMD_APP_PAUSE:
				//ui_handle_pause_play(1,0);
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

			case UI_CMD_USB_PLAY_MUTE:
				if (mute_state == UNMUTE)
				{
					//set_channel_mixvol_by_mode(ui_source_select);
					pa_mute_ctrl(false);
					//player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
				}			
				break;

			case UI_CMD_GET_USB_PLAY_STATUS:
				ui_handle_pause_play(2,0);
				break;

			case UI_CMD_USB_FOLDER_DIS:
				display_ui_usb_number(file_rel_pos);
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
				ui_handle_pause_play(0,0);
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
			case UI_CMD_PLAY_PAUSE:
				//FmScan(1,1);
				break;

			case UI_CMD_FM_TUNE_ADD:
				fm_fre_add_sub(1);
				break;

			case UI_CMD_FM_TUNE_SUB:
				fm_fre_add_sub(0);
				break;

			case UI_CMD_FM_SCAN:
				FmScan(1,1);
				break;

			case UI_CMD_FM_HALF_SCAN_SUB:
				FmScan(0,0);
				break;

			case UI_CMD_FM_HALF_SCAN_ADD:
				FmScan(0,1);
				break;

			case UI_CMD_BT_PAIR:
				FmScan(1,1);
				break;

			case UI_CMD_NEXT:
				fm_ch_add_sub(1);
				break;

			case UI_CMD_PREV:
				fm_ch_add_sub(0);
				break;

			case UI_CMD_FM_MANUAL_SAVE:
				#if 0
				fm_manual_save_status = true;
				fm_manual_save_cnt = 0;
				#else
				fre_manual_save();
				#endif
				break;

			case UI_CMD_APP_PLAY:
				app_mute_flag = true;
				mute_state = MUTE;
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			case UI_CMD_APP_PAUSE:
				app_mute_flag = true;
				mute_state = UNMUTE;
				put_ui_msg(UI_CMD_VOLUME_MUTE);
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

			case UI_CMD_APP_PLAY:
				mute_state = MUTE;
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			case UI_CMD_APP_PAUSE:
				mute_state = UNMUTE;
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
void source_mode_rca(void)
{
	ui_cmd_t cmd;
	unsigned char ret=0;
	enter_mode(SOURCE_SELECT_RCA);

	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_PLAY_PAUSE:
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			case UI_CMD_APP_PLAY:
				mute_state = MUTE;
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			case UI_CMD_APP_PAUSE:
				mute_state = UNMUTE;
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;
		}

		if(ret)
		{
			if(ui_source_select!=SOURCE_SELECT_RCA)
			{
				exit_mode(SOURCE_SELECT_RCA);	
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

			case UI_CMD_APP_PLAY:
				mute_state = MUTE;
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			case UI_CMD_APP_PAUSE:
				mute_state = UNMUTE;
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

			case UI_CMD_APP_PLAY:
				mute_state = MUTE;
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			case UI_CMD_APP_PAUSE:
				mute_state = UNMUTE;
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			case UI_CMD_HDMION_SEND:	
				hdmi_send_unmute();
		    	break;

			case UI_CMD_HDMION_DEINIT:
				sc8836_action_hdmi_on();
				break;

			case UI_CEC_INACTIVE_SOURCE:
				sc8836_ui_handle_cec_inactive_source();
	   			break;

			case UI_CEC_ACTIVE_SOURCE:
				sc8836_ui_handle_cec_active_source();
	   			break;

			#if CONFIG_CEC
			case  UI_CEC_VOLUME_KEY_UP:
				ui_handle_vol_up();
				break;

			case  UI_CEC_VOLUME_KEY_DOWN:
				ui_handle_vol_down();
				break;

			case UI_CEC_MUTE_KEY:
				ui_handle_mute();
				break;

			#endif

			default:
				ret=ui_handle_cmd_com(&cmd);
				break;
		}

		if(ret)
		{
			if((ui_source_select!=SOURCE_SELECT_HDMI)||(hdmi_cec_online==true))
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

			case UI_CMD_APP_PLAY:
				mute_state = MUTE;
				put_ui_msg(UI_CMD_VOLUME_MUTE);
				break;

			case UI_CMD_APP_PAUSE:
				mute_state = UNMUTE;
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
#if 0
	ui_cmd_t cmd;
	unsigned char ret=0;

	test_mode_flag = true;
	//set_adc_channel_vol(1,0);
	//set_adc_channel_vol(2,0);
	//set_adc_channel_vol(3,0);
	//player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
	pa_mute_ctrl(true);

	ui_source_select = SOURCE_SELECT_BT;
	save_player_info();
	ui_source_select = SOURCE_SELECT_TEST;
	usleep(100000);

	fm_clear();
	
	display_ui_full();
	ht1633_updata_display();
	usleep(500000);
	handle_bt_cmd(AT_CLEAR_LIST, 0);
	usleep(200000);
    bt_mix_vol = Frist_MIX_LEV;
	select_mixvol_table();	
	usleep(10000);	
	save_mix_vol();
	usleep(10000);
	display_ui_init();
	ht1633_updata_display();
	
#if (BYPASS_MODE || INNERADC_MODE)
	player_process_cmd(NP_CMD_LINEIN_OFF, NULL, 0, NULL, NULL);
#else
	player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 1, NULL, NULL);
#endif
	usleep(1000);
	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
	usleep(1000);
	player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);
	usleep(1000);
	usleep(1000);
	//set_channel_vol_by_mode(ui_source_select);
	usleep(200000);
	//set_channel_mixvol_by_mode(ui_source_select);
	usleep(100000);
	pa_mute_ctrl(false);

	while(1)
	{
		usleep(UI_TASK_DELAY);
		cmd=get_mq_msg();

		switch(cmd.cmd)
		{
			case UI_CMD_POWER:
				#if 0
				test_mode_flag = false;
				pa_mute_ctrl(true);
				display_ui_clear();
				ht1633_updata_display();
				sys_power_control();
				ret=1;
				#endif
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
#endif
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

