#include <nuttx/config.h>
#include <nuttx/input.h>
#include <nuttx/module.h>
#include <pthread.h>
#include <stdio.h>
#include "sl_ui_cmd.h"
#include "sl_ui_display.h"
#include "sc6138_led7.h"
#include <pthread.h>
#include <string.h>
#include <sys/ioctl.h>

//#include "sl_ui_handle.h"

static const char rca_str[] = {NUM_A, NUM_U, NUM_H, NUM_1};
static const char aux_str[] = {NUM_A, NUM_U, NUM_H, NUM_2};
static const char bt_str[] = {NUM__, NUM_B, NUM_T, NUM__};
static const char clear_str[] = {NUM_OFF, NUM_OFF, NUM_OFF, NUM_OFF};
static const char fm_str[] = {NUM_T, NUM_U, NUM_N, NUM_E};
static const char optical_str[] = {NUM__, NUM_0, NUM_P, NUM__};
static const char coa_str[] = {NUM__, NUM_C, NUM_0, NUM__};
static const char power_on_str[] = {NUM__, NUM__, NUM__, NUM__};
static const char power_off_str[] = {NUM_0, NUM_V, NUM_E, NUM_OFF};
static const char sd_str[] = {NUM__, NUM_5, NUM_D, NUM__};
static const char usb_str[] = {NUM_U, NUM_5, NUM_B, NUM_OFF};
static const char up_str[] = {NUM__, NUM_U, NUM_P, NUM__};
static const char off_str[] = {NUM_0, NUM_F, NUM_F, NUM_OFF};
static const char power_wait_str[] = {NUM__,NUM__, NUM__, NUM__};
static const char hdmi_str[] = {NUM__,NUM_H,NUM_D, NUM__,};

#if 0
static  char aux_str[] = {NUM_OFF,NUM_A, NUM_U, NUM_X, NUM_OFF};
static  char bt_str[] = {NUM_OFF,NUM_OFF, NUM_B, NUM_t, NUM_OFF};
static  char clear_str[] = {NUM_OFF,NUM_OFF, NUM_OFF, NUM_OFF, NUM_OFF};
static  char fm_str[] = {NUM_OFF,NUM_OFF, NUM_F, NUM_M, NUM_OFF};
static  char optical_str[] = {NUM_OFF,NUM_O, NUM_P, NUM_T, NUM_OFF};
static  char power_on_str[] = {NUM_OFF,NUM__, NUM_H, NUM_I, NUM__};
static  char power_off_str[] = {NUM_OFF,NUM_B, NUM_Y, NUM_E, NUM_OFF};
static  char sd_str[] = {NUM_OFF,NUM_OFF, NUM_S, NUM_D, NUM_OFF};
static  char usb_str[] = {NUM_OFF,NUM_U, NUM_S, NUM_B, NUM_OFF};
static  char hdmi_str[] = {NUM_OFF,NUM_H, NUM_D, NUM_M ,NUM_I};
static  char coa_str[] = {NUM_OFF,NUM_C, NUM_O, NUM_A ,NUM_OFF};
static  char power_wait_str[] = {NUM__,NUM__, NUM__, NUM__ ,NUM__};
#endif



extern void ui_handle_cmd(ui_cmd_t *cmd);
char input_flag=0;
char play_pause=0;

extern WDOG_ID wdtimer_goback_mode;
extern int ui_source_select;
extern bool bt_connected;
extern  char dis_other_mode;
extern int sd_playtime;
extern int usb_playtime;
extern bool bt_connected ;
extern bool usb_online;
extern bool sd_online;
extern bool aux_online;
extern float mix_vol ;
extern int bt_mix_vol;
extern unsigned int  input_number;

extern unsigned int  input_number_ok;
extern char input_n;

extern bool woofer_connected;
extern bool bt_ok_flag;

extern  int mute_state ;
extern unsigned int  fmFrequency;
extern unsigned char Cur_Fre_Num;

extern char mic_vol_flag;
extern char mic_echo_flag;

extern int folder_index_dis;
//extern bool fm_manual_save_status;

extern unsigned char bt_version_num;


#define MCU_VER1 2
#define MCU_VER2 5

#define BT_VER1 1
#define BT_VER2 2

#define HOR_DIS 1
#define VER_DIS      2
#define ALL_DIS      3
#define NO_DIS       4

int led_test_mode = HOR_DIS;
bool mute_dis_flag = false;

int ui_display_select = -1;


/**************************************



**************************************/
void display_ui_full(void)
{
	ht1633_full_disbuf();
}


/**************************************



**************************************/
void display_ui_clear(void)
{
	led7_clear_disbuf();
    led7_clear_bitbuf();
	led7_clear_display();
}
/**************************************



**************************************/


void display_ui_icon(char icon,bool on_off)
{
	if((icon>ICON_TOTAL)||(icon<0))
		return;
	
	switch(icon)
	{
		case ICON_AUX:
			led7_set_aux(on_off);
		break;
		
		case ICON_USB:
			led7_set_usb(on_off);
		break;

		case ICON_BT:
			led7_set_bt(on_off);
		break;

		case ICON_OPTI:
			led7_set_opt(on_off);
		break;

		case ICON_COA:
			led7_set_coa(on_off);
		break;

		case ICON_HDMI:
			led7_set_hdmi(on_off);
		break;

		case ICON_EQ:
			led7_set_eq(on_off);
		break;

		case ICON_DOT:
			led7_set_dot(on_off);
		break;

		case ICON_CLON_2:
			led7_set_colon(on_off);
		break;			
	}
	led7_update_char();
}



void display_str( char *dis_str)
{
    if (dis_str != NULL)
    {
        int i;
        for (i = 0; i < 4; ++i)
        {
            led7_set_display(i, dis_str[i]);
        }

        led7_update_bitbuf();
    }
}



/****************************************************



****************************************************/
void display_ui_power(char on_off)
{
	display_ui_clear();
	if(on_off==1)
		display_str(power_on_str);
	else if(on_off==0)
		display_str(power_off_str);
	else if(on_off==2)
		display_str(power_wait_str);
}

/****************************************************



****************************************************/
void display_ui_main_sys(char wm_mode)
{
	if(bt_ok_flag)
	{
		display_ui_clear();
		display_ui_device(wm_mode);
	}	
	//dis_other_mode=1;
	//ui_goback_source(400);
}


/*********************************************




***********************************************/
void display_ui_device(char wm_mode)
{

	display_ui_clear();
	switch(wm_mode)
	{
		case SOURCE_SELECT_BT:
			display_ui_icon(ICON_BT,1);
			display_str(bt_str);
			break;

		case SOURCE_SELECT_USB:
			display_ui_icon(ICON_USB,1);
			display_str(usb_str);
			break;

		case SOURCE_SELECT_SD:
			// display_ui_icon(ICON_SD,sd_online);
			display_str(sd_str);
			break;

		case SOURCE_SELECT_FM:
			//display_ui_icon(ICON_FM,1);
			display_str(fm_str);
			break;

		case SOURCE_SELECT_LINEIN:
			display_ui_icon(ICON_AUX,1);
			display_str(aux_str);
			break;

		case SOURCE_SELECT_RCA:
			display_ui_icon(ICON_AUX,1);
			display_str(rca_str);
			break;

		case SOURCE_SELECT_COA:
			display_ui_icon(ICON_COA,1);
			display_str(coa_str);
			break;

		case SOURCE_SELECT_SPDIFIN:
			display_ui_icon(ICON_OPTI,1);
			display_str(optical_str);
			break;

		case SOURCE_SELECT_HDMI:
			display_ui_icon(ICON_HDMI,1);
			display_str(hdmi_str);
			break;
	}
}





/****************************************************************************
 * Name: lcd_display_set_source
 *
 * Description:
 *    音源字符串显示
 *
 * Parameters:
 *    num 音源序号
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
 void display_set_source(int num)
{

	display_ui_clear();

	if(mute_state)
	{
		display_ui_icon(ICON_VOL_MUTE,1);
		display_ui_icon(ICON_VOL_UMUTE,0);
	}
	else
	{
		display_ui_icon(ICON_VOL_MUTE,0);
		display_ui_icon(ICON_VOL_UMUTE,1);
	}

	switch(num)
	{
		case SOURCE_SELECT_BT:
			display_ui_bt();
			break;

		case SOURCE_SELECT_USB:
			display_ui_usb();
			break;

		case SOURCE_SELECT_SD:
			display_ui_sd();
			break;

		case SOURCE_SELECT_FM:
			display_ui_fm(0);
			break;

		case SOURCE_SELECT_LINEIN:
			display_ui_aux();
			break;

		case SOURCE_SELECT_RCA:
			display_ui_rca();
			break;

		case SOURCE_SELECT_SPDIFIN:
			display_ui_op();
			break;

		case SOURCE_SELECT_HDMI:
			display_ui_hdmi();
			break;

		case SOURCE_SELECT_COA:
			display_ui_coaxial();
			break;
	}
	//ht1633_updata_display();
}




/****************************************************************************
 * Name: ui_display_source
 *
 * Description:
 *    音源显示处理
 *
 * Parameters:
 *    argc 看门狗回调参数数量
 *    arg1 看门狗回调入参1
 *    ...  看门狗回调入参
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
 void ui_display_source(void)
{
	ui_cmd_t cmd;

	if(NULL != wdtimer_goback_mode)
	{
		wd_cancel(wdtimer_goback_mode);
	}

	dis_other_mode=0;

	if(input_flag)
	{
		cmd.cmd = UI_CMD_ENTER;
		send_cmd_2_ui(&cmd);
		input_flag=0;
	}

	if(!mute_state)
	{
		cmd.cmd = UI_CMD_SET_SOURCE;
		send_cmd_2_ui(&cmd);
		//display_set_source(ui_source_select);
	}


}

/****************************************************************************
 * Name: ui_goback_source
 *
 * Description:
 *    音源延时显示函数
 *
 * Parameters:
 *    delay 延时的时间
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_goback_source(int delay)
{
	if (wdtimer_goback_mode == NULL)
	{
		wdtimer_goback_mode = wd_create();
	}
	else
	{
		wd_cancel(wdtimer_goback_mode);
	}

	if (wdtimer_goback_mode != NULL)
	{
		wd_start(wdtimer_goback_mode, delay, ui_display_source, 0);
	}

    #ifdef SL_UI_DBG
    printf("%s\n",__func__);
    #endif
}


/****************************************************



****************************************************/
void display_ui_vol(int vol)
{
	//int vol_i =(int)(vol / VOL_STEP);

	char volume[4] = {0, -1, 0, 0};

	volume[0]  = NUM_U;
	volume[2] = vol / 10;
	volume[3] =  vol % 10;

	display_ui_clear();
	display_str(volume);
	
	if((vol > 0)&&(vol < 30) )
	{
		dis_other_mode = 1;
	}
	else
	{
		dis_other_mode = 100;
	}
	
}


/****************************************************



****************************************************/
void display_ui_maxmin_vol(void)
{

	static bool is_Disp = true;
	static int dis_count= 0;

	char volume[4] = {0, NUM_OFF, 0, 0};

	if(++dis_count < 6)
	{
		if(!is_Disp)
		{
			//非显示状态
			display_ui_clear();
			is_Disp = true;
		}
		else
		{
			//显示状态
			display_str(volume);
			is_Disp = false;
		}	
	}
	else
	{
		dis_count = 0;
		is_Disp = true;
		display_set_source(ui_source_select);
	}
	
}


/****************************************************



****************************************************/
void display_mic_vol(int vol)
{
#if 0
	//int vol_i =(int)(vol / VOL_STEP);

	char volume[5] = {0, 0, 0, 0,0};
	volume[3] = vol / 10;
	volume[4] =  vol % 10;

	display_ui_clear();
	if((mic_vol_flag == true)||(mic_echo_flag == true))
	{

		if(mic_vol_flag ==      true)
		{
			volume[0]=NUM_OFF;
			volume[1]=NUM_n;
			volume[2]=NUM_n;
		}
		else if(mic_echo_flag ==       true)
		{
			volume[0]=NUM_OFF;
			volume[1]=NUM_E;
			volume[2]=NUM_OFF;
		}

		dis_other_mode=1;
		display_str(volume);
		ht1633_updata_display();

	}
	else
	{
		dis_other_mode=1;
		display_set_source(ui_source_select);
	}
#endif
}


/****************************************************



****************************************************/
void display_ui_usb(void)
{
	char dis[4]={NUM_U,NUM_5,NUM_B,NUM_OFF};
	display_ui_icon(ICON_USB,1);
	if(usb_online)
	{
		ui_update_music_time();
	}
	else
	{
		display_str(dis);
	}
	//ht1633_updata_display();
}


/****************************************************



****************************************************/
void display_ui_sd(void)
{
	char dis[4]={NUM_5,NUM_D,NUM_N,NUM_0};
	if(sd_online)
	{
		ui_update_music_time();
		//display_str(sd_str);
		//display_ui_icon(ICON_SD,sd_online);
		//ht1633_updata_display();
	}
	else
	{
		//display_ui_icon(ICON_SD,sd_online);
		display_str(dis);
	}
}


/****************************************************



****************************************************/
void display_ui_bt(void)
{

	static bool isDisp = true;

	//int vol_i =(int)( mix_vol / VOL_STEP);

	char volume[4] =  {NUM__, NUM_B, NUM_T, NUM__};

	//volume[2] = bt_mix_vol / 10;
	//volume[3] = bt_mix_vol % 10;

	if(ui_source_select == SOURCE_SELECT_BT )
	{
		display_ui_clear();
		display_ui_icon(ICON_BT,1);
		if(bt_connected)
		{
			//已经连接
			//  if(!isDisp)
			{//非显示状态
			display_str(volume);
			isDisp = true;
			}
		}
		else
		{//未连接
			if(!isDisp)
			{
				//非显示状态
				display_str(volume);
				isDisp = true;
			}
			else
			{
				//显示状态
				display_str(clear_str);
				isDisp = false;
			}
		}


		if(mute_state)
		{
			display_ui_icon(ICON_VOL_MUTE,1);
			display_ui_icon(ICON_VOL_UMUTE,0);
		}
		else
		{
			display_ui_icon(ICON_VOL_MUTE,0);
			display_ui_icon(ICON_VOL_UMUTE,1);
		}

		
		//ht1633_updata_display();
	}
}

/****************************************************



****************************************************/
void display_ui_fm(unsigned char f_ch)
{
	char dis_buf[4];
	display_ui_clear();
	if(f_ch==0)
	{
		display_ui_icon(ICON_DOT,1);
		if(fmFrequency<1000)
		{
			dis_buf[0]=NUM_OFF;
			dis_buf[1]=(char)((fmFrequency/100%10)>=0?(fmFrequency/100%10):NUM_OFF);
			dis_buf[2]=(char)((fmFrequency/10%10)>=0?(fmFrequency/10%10):NUM_OFF);
			dis_buf[3]=(char)((fmFrequency%10)>=0?(fmFrequency%10):NUM_OFF);
		}
		else
		{
			dis_buf[0]=(char)((fmFrequency/1000%10)>=0?(fmFrequency/1000%10):NUM_OFF);
			dis_buf[1]=(char)((fmFrequency/100%10)>=0?(fmFrequency/100%10):NUM_OFF);
			dis_buf[2]=(char)((fmFrequency/10%10)>=0?(fmFrequency/10%10):NUM_OFF);
			dis_buf[3]=(char)((fmFrequency%10)>=0?(fmFrequency%10):NUM_OFF);
		}
	}
	else
	{
		if(Cur_Fre_Num < 100)
		{
			dis_buf[0]=NUM_C;
			dis_buf[1]=NUM_H;
			dis_buf[2]=(char)((Cur_Fre_Num/10%10)>=0?(Cur_Fre_Num/10%10):NUM_OFF);
			dis_buf[3]=(char)((Cur_Fre_Num%10)>=0?(Cur_Fre_Num%10):NUM_OFF);
		}
		else if(Cur_Fre_Num == 100)
		{
			dis_buf[0]=NUM_C;
			dis_buf[1]=NUM_H;
			dis_buf[2]=NUM_0;
			dis_buf[3]=NUM_0;
		}
		dis_other_mode=1;
	}

	display_ui_icon(ICON_FM,1);
	display_str(dis_buf);
	

	if(mute_state)
	{
		display_ui_icon(ICON_VOL_MUTE,1);
		display_ui_icon(ICON_VOL_UMUTE,0);
	}
	else
	{
		display_ui_icon(ICON_VOL_MUTE,0);
		display_ui_icon(ICON_VOL_UMUTE,1);
	}
	//ht1633_updata_display();
}


/****************************************************



****************************************************/
void display_ui_aux(void)
{
	char volume[4] = {NUM_A, NUM_U,NUM_H,NUM_2};
	aux_online=true;

	//volume[2] = bt_mix_vol / 10;
	//volume[3] = bt_mix_vol % 10;
	display_ui_icon(ICON_AUX,1);
	display_str(volume);
	
	//ht1633_updata_display();

}


/****************************************************



****************************************************/
void display_ui_rca(void)
{
	char volume[4] = {NUM_A, NUM_U,NUM_H,NUM_1};
	aux_online=true;

	//volume[2] = bt_mix_vol / 10;
	//volume[3] = bt_mix_vol % 10;
	display_ui_icon(ICON_AUX,1);
	display_str(volume);
	
	//ht1633_updata_display();

}


/****************************************************



****************************************************/
void display_ui_op(void)
{
	char volume[4] = {NUM__, NUM_0, NUM_P, NUM__};

	//volume[2] = bt_mix_vol / 10;
	//volume[3] = bt_mix_vol % 10;
	display_ui_icon(ICON_OPTI,1);
	display_str(volume);
}
/****************************************************



****************************************************/
void display_ui_hdmi(void)
{
	char volume[4] = {NUM__,NUM_H, NUM_D,NUM__,};

	//volume[2] = bt_mix_vol / 10;
	//volume[3] = bt_mix_vol % 10;
	display_ui_icon(ICON_HDMI,1);
	display_str(volume);

	
	//ht1633_updata_display();
}





void display_ui_coaxial(void)
{

	//int vol_i = (int)(mix_vol / VOL_STEP);

	char volume[4] = {NUM__,NUM_C, NUM_0,NUM__};
	//printf("display_ui_coaxial vol = %d\n",vol_i);
	//volume[2] = bt_mix_vol/ 10;
	//volume[3] = bt_mix_vol % 10;
	display_ui_icon(ICON_COA,1);
	display_str(volume);

	
	//ht1633_updata_display();
}





/****************************************************************************
 * Name: ui_update_music_time
 *
 * Description:
 *    更新时间显示
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_update_music_time(void)
{
#if 1
	if(ui_source_select == SOURCE_SELECT_USB ||
	ui_source_select == SOURCE_SELECT_SD)
	{

		if((usb_online==0)&&(sd_online==0))
			return;
		static bool showed = true;
		static bool colon_showed = true;
		//获取播放器信息
		ui_info_t player_info = sc8836_get_all_play_info();

		//   if (player_info.player_stat == 2 ||
		//      (player_info.player_stat == 3))
		//{   //播放状态或暂停显示隐藏状态
		int * seek_time_p = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
		int curtime = player_info.curtime;
		//printf("%s:* seek_time_p = %d, curtime = %d\n", __func__,* seek_time_p,curtime);
		#if 1
		if(curtime < *seek_time_p)
		{
			//还未seek到指定位置
			curtime = *seek_time_p;
		}
		else if(curtime >= *seek_time_p && curtime < *seek_time_p + 10)
		{
			//有效时间
			//记录当前播放时间
			*seek_time_p = curtime;
		}
		else
		{
			//非法时间
			curtime = 0;

		}
		#endif

		display_ui_clear();
		display_ui_icon(ICON_USB,1);

		if(mute_state)
		{
			display_ui_icon(ICON_VOL_MUTE,1);
			display_ui_icon(ICON_VOL_UMUTE,0);
		}
		else
		{
			display_ui_icon(ICON_VOL_MUTE,0);
			display_ui_icon(ICON_VOL_UMUTE,1);
		}


		if(showed)
		{
			showed = true;
			//显示播放时间
			char min = curtime / 60;
			char sec = curtime % 60;
			char display[4] = {min/10, min%10, sec/10, sec%10};
			display_ui_icon(ICON_CLON_2,colon_showed);
			display_str(display);
			//  if( (player_info.player_stat == 2)||(player_info.player_stat == 3))	//////play
			if( player_info.player_stat == 2)
			{
				colon_showed = ~colon_showed;
				colon_showed =colon_showed&0x01;
			}
			else if(player_info.player_stat == 3)
			{
				colon_showed = true;
			}
			//printf("%s:min = %d, sec = %d\n", __func__,min,sec);
		}
		else
		{
			showed = false;
			//隐藏两点
			display_ui_icon(ICON_CLON_2,0);
			display_str(clear_str);		
		}
		//}

		//ht1633_updata_display();
	}
#endif

}

/*****************************************************




*******************************************************/
void display_ui_input(unsigned int number)
{
	display_ui_clear();
	char dis_buf[4];
	if((number>=0)&&(number<=9999))
	{
		dis_buf[0]=(char)((number/1000%10)>=0?(number/1000%10):NUM_OFF);
		dis_buf[1]=(char)((number/100%10)>=0?(number/100%10):NUM_OFF);
		dis_buf[2]=(char)((number/10%10)>=0?(number/10%10):NUM_OFF);
		dis_buf[3]=(char)((number%10)>=0?(number%10):NUM_OFF);
	}
	display_str(dis_buf);
	//ht1633_updata_display();
	dis_other_mode=2;
	//input_flag=1;
}




/**********************************************************




************************************************************/
void display_ui_mic(bool on_off)
{
#if 0
	char dis_buf1[5]={NUM_M,NUM_I,NUM_C,NUM_O,NUM_N};
	char dis_buf2[5]={NUM_M,NUM_I,NUM_C,NUM_O,NUM_F};

	display_ui_clear();
	dis_other_mode=2;
	if(on_off==1)
		display_str(dis_buf1);
	else
		display_str(dis_buf2);
	ht1633_updata_display();
#endif
}


/**********************************************************




************************************************************/
void display_ui_movie(bool on_off)
{
#if 0
	char dis_buf1[5]={NUM_M,NUM_O,NUM_U,NUM_O,NUM_N};
	char dis_buf2[5]={NUM_M,NUM_O,NUM_U,NUM_O,NUM_F};

	display_ui_clear();
	if(on_off==1)
		display_str(dis_buf1);
	else
		display_str(dis_buf2);
	ht1633_updata_display();
	dis_other_mode=1;
#endif
}


void display_ui_bass_vol(int mode,int vol)
{
#if 0
	char dis_buf1[5]={NUM_OFF,NUM_B,NUM_0,NUM_0,NUM_0};
	char dis_buf2[5]={NUM_OFF,NUM_t,NUM_0,NUM_0,NUM_0};

	display_ui_clear();
	if(vol >= 0)
	{
		dis_buf1[2] = NUM_ADD;
		dis_buf2[2] = NUM_ADD;
	}
	else
	{
		vol = -vol;
		dis_buf1[2] = NUM__;
		dis_buf2[2] = NUM__;
	}

	dis_other_mode=1;
	if(mode == 2)
	{
		dis_buf2[3] = vol/10;
		dis_buf2[4] = vol%10;
		display_str(dis_buf2);
	}
	else
	{
		dis_buf1[3] = vol/10;
		dis_buf1[4] = vol%10;
		display_str(dis_buf1);
	}

	ht1633_updata_display();
#endif
}




/***********************************************************



************************************************************/
void dis_ui_updata_program(char on_off)
{
	char dis_buf1[4]={NUM_U,NUM_P,NUM_D,NUM_A};
	char dis_buf2[4]={NUM__,NUM_0,NUM_U,NUM__ };

	display_ui_clear();
	if(on_off==0)
		display_str(dis_buf1);
	else
		display_str(dis_buf2);
	//ht1633_updata_display();
}


void display_ui_version(int ver)
{
	char dis_buf1[4]={NUM_OFF,NUM_U,0,0};
	char dis_buf2[4]={NUM_OFF,NUM_U,0,0};

	display_ui_clear();
	if(ver==0)
	{
		dis_buf1[2] = MCU_VER1;
		dis_buf1[3] = MCU_VER2;
		display_str(dis_buf1);
	}
	else if(ver==1)
	{
		dis_buf2[2] = bt_version_num/10;
		dis_buf2[3] = bt_version_num%10;
		display_str(dis_buf2);
	}
	display_ui_icon(ICON_DOT,1);

	//ht1633_updata_display();
}


void display_ui_init(void)
{
#if 0
	char dis_buf[5]={NUM_R,NUM_5,NUM_t,NUM_OFF,NUM_OFF};

	display_ui_clear();

	display_str(dis_buf);

	//ht1633_updata_display();
#endif
}

void display_ui_ledtest(void)
{
#if 0
	char hor_dis_buf[5]={NUM_HOR,NUM_HOR,NUM_HOR,NUM_HOR,NUM_HOR};
	char ver_dis_buf[5]={NUM_VER,NUM_VER,NUM_VER,NUM_VER,NUM_VER};


	switch(led_test_mode)
	{
		case HOR_DIS:
			display_ui_clear();
			display_str(hor_dis_buf);
			ht1633_updata_display();
			led_test_mode = VER_DIS;
			break;

		case VER_DIS:
			display_ui_clear();
			display_str(ver_dis_buf);
			ht1633_updata_display();
			led_test_mode = ALL_DIS;
			break;

		case ALL_DIS:
			display_ui_full();
			ht1633_updata_display();
			led_test_mode = NO_DIS;
			break;

		case NO_DIS:
			display_ui_clear();
			ht1633_updata_display();
			led_test_mode = HOR_DIS;
			break;

		default:
			break;

	}
#endif
}


char bt_no_vol[4] = {NUM_OFF, NUM_OFF,NUM_OFF,NUM_OFF};
char aux_no_vol[4] = {NUM_OFF, NUM_OFF,NUM_OFF,NUM_OFF};
char rca_no_vol[4] = {NUM_OFF, NUM_OFF,NUM_OFF,NUM_OFF};
char opt_no_vol[4] ={NUM_OFF, NUM_OFF,NUM_OFF,NUM_OFF};
char hdmi_no_vol[4] ={NUM_OFF, NUM_OFF,NUM_OFF,NUM_OFF};
char coa_no_vol[4] ={NUM_OFF, NUM_OFF,NUM_OFF,NUM_OFF};
char fm_no_vol[4] ={NUM_OFF,NUM_OFF,NUM_OFF,NUM_OFF};
char usb_no_vol[4] ={NUM_OFF, NUM_OFF,NUM_OFF,NUM_OFF};

/*
char bt_no_vol[4] = {NUM_B, NUM_T,NUM_OFF,NUM_OFF};
char aux_no_vol[4] = {NUM_A, NUM_U,NUM_OFF,NUM_OFF};
char rca_no_vol[4] = {NUM_C, NUM_A,NUM_OFF,NUM_OFF};
char opt_no_vol[4] ={NUM_0, NUM_P,NUM_OFF,NUM_OFF};
char hdmi_no_vol[4] ={NUM_H, NUM_D,NUM_OFF,NUM_OFF};
char coa_no_vol[4] ={NUM_C, NUM_0,NUM_OFF,NUM_OFF};
char fm_no_vol[4] ={NUM_T,NUM_N,NUM_OFF,NUM_OFF};
char usb_no_vol[4] ={NUM_U, NUM_5,NUM_OFF,NUM_OFF};
*/

typedef void (*func_mode)(void);
struct mute_dispui_st
{
	int mode;
	char *no_vol;
	func_mode mode_func;
}mute_dispui[7]=
{
	{SOURCE_SELECT_BT,bt_no_vol,display_ui_bt},
	{SOURCE_SELECT_SPDIFIN,opt_no_vol,display_ui_op},
	{SOURCE_SELECT_USB,usb_no_vol,display_ui_usb},
	{SOURCE_SELECT_LINEIN,aux_no_vol,display_ui_aux},
	{SOURCE_SELECT_RCA,rca_no_vol,display_ui_rca},
	{SOURCE_SELECT_HDMI,hdmi_no_vol,display_ui_hdmi},
	{SOURCE_SELECT_COA,coa_no_vol,display_ui_coaxial},
	{SOURCE_SELECT_FM,fm_no_vol,display_ui_fm},
};

void display_ui_mute(void)
{
	static bool isDisp = true;
	char volume[4] = {NUM__, NUM__,NUM__,NUM__};

	if(mute_state == MUTE)
	{
		mute_dis_flag =      true;
		if(!isDisp)
		{
			//非显示状态
			display_ui_clear();
			isDisp = true;
		}
		else
		{
			//显示状态
			display_ui_clear();
			display_str(volume);
			isDisp = false;
		}	
	}
	else if((mute_state == UNMUTE)&&(mute_dis_flag ==  true))
	{
		mute_dis_flag =      false;
		display_set_source(ui_source_select);
	}
	
#if 0
	int i;
	static bool isDisp = true;
	char *str_no_vol = NULL;
	char volume[4] = {NUM_B, NUM_T,0,0};

	volume[2] = bt_mix_vol / 10;
	volume[3] = bt_mix_vol % 10;

	for(i=0;i<7;i++)
	{
		if(ui_source_select == mute_dispui[i].mode)
		{
			str_no_vol = mute_dispui[i].no_vol;

			if(mute_state)
			{
				mute_dis_flag =  true;
				display_ui_clear();
				display_ui_icon(ICON_VOL_MUTE,1);
				display_ui_icon(ICON_VOL_UMUTE,0);
				if(!isDisp)
				{
					if((ui_source_select == SOURCE_SELECT_FM)||(ui_source_select == SOURCE_SELECT_USB))
					{
						display_ui_vol(bt_mix_vol);
					}
					else if((ui_source_select == SOURCE_SELECT_BT)&&(!bt_connected))
					{
						display_str(volume);
					}
					else
					{
						mute_dispui[i].mode_func();
					}

					isDisp = true;
				}
				else
				{
					//显示状态
					if(ui_source_select == SOURCE_SELECT_SPDIFIN)
					{
						display_ui_icon(ICON_OPTI,1);
					}

					if(ui_source_select == SOURCE_SELECT_BT)
					{
						if(!bt_connected)
						{
							display_str(clear_str);
						}
						else
						{
							display_ui_icon(ICON_BT,bt_connected);
							display_str(str_no_vol);
						}

					}
					else
					{
						display_str(str_no_vol);
					}

					isDisp = false;
				}
				//ht1633_updata_display();
			}
			else if((mute_state == false)&&(mute_dis_flag ==  true))
			{
				mute_dis_flag =   false;
				display_set_source(ui_source_select);
			}
		}
	}
#endif
}

void display_eq_mode(int mode)
{
	char mode_buf[4] = {0, -1, 0, 0};

	mode_buf[0]  = NUM_E;
	mode_buf[2]  = NUM_0;
	mode_buf[3]  = mode;
	
	display_ui_clear();
	display_str(mode_buf);
	dis_other_mode=1;
}
void display_ui_enter_tre_bass(int mode)
{
#if 0
	char treble_buf[5]={NUM_OFF,NUM_t,NUM_0,NUM_1,NUM_OFF};
	char bass_buf[5]={NUM_OFF,NUM_B,NUM_0,NUM_1,NUM_OFF};

	display_ui_clear();
	if(mode==0)
	{
		display_str(treble_buf);
	}
	else if(mode ==1)
	{
		display_str(bass_buf);
	}

	ht1633_updata_display();
	dis_other_mode=2;
#endif
}

void display_ui_usb_folder(int loc)
{

	char folder_buf1[4] = {NUM_U, 0,0,0};
	char folder_buf2[4] = {NUM_U, NUM_0,NUM_0,NUM_1};

	folder_buf1[1] = (folder_index_dis+1) / 100;
	folder_buf1[2] =((folder_index_dis+1)% 100)/10;
	folder_buf1[3] = (folder_index_dis+1)%10;

	if(ui_source_select == SOURCE_SELECT_USB )
	{
		display_ui_clear();
		if(loc == 0)
		{
			display_str(folder_buf1);
		}
		else if(loc ==1)
		{
			display_str(folder_buf2);
		}

		//ht1633_updata_display();
		dis_other_mode=1;
	}
}

void display_ui_usb_number(int num)
{
	char num_buf1[4] = {NUM_U, 0,0,0};

	num_buf1[0] = (num+1) / 1000;
	num_buf1[1] = ((num+1) %1000)/ 100;
	num_buf1[2] =((num+1)% 100)/10;
	num_buf1[3] = (num+1)%10;

	if(ui_source_select == SOURCE_SELECT_USB )
	{
		display_ui_clear();		
		display_str(num_buf1);		
		//ht1633_updata_display();
		dis_other_mode=1;
	}
}


void display_ui_woofer(void)
{
	static bool isDisp = true;
	char woofer[4] = {NUM_OFF, NUM_OFF,NUM_2,NUM_1};

	if(!woofer_connected)
	{
		if(!isDisp)
		{
			//非显示状态
			display_ui_clear();
			isDisp = true;
		}
		else
		{
			//显示状态
			display_ui_clear();
			display_ui_icon(ICON_DOT,1);
			display_str(woofer);
			isDisp = false;
		}	
	}
	else
	{
		//显示状态
		display_ui_clear();
		display_ui_icon(ICON_DOT,1);
		display_str(woofer);
	}
	
}

/****************************************************



****************************************************/
void display_ui_fm_manual_save(void)
{
#if 0
	static bool isDisp = true;

	if(fm_manual_save_status == true)
	{
		if(ui_source_select == SOURCE_SELECT_FM)
		{
			if(!isDisp)
			{
				display_ui_fm(0);
				isDisp = true;
			}
			else
			{
				display_ui_clear();
				ht1633_updata_display();
				isDisp = false;
			}
		}
	}
#endif
}

/**********************************************************




************************************************************/
void display_ui_eq_power_test(void)
{
#if 0
	char dis_buf[5]={NUM_B,NUM_Y,NUM_P,NUM_A,NUM_S};

	display_ui_clear();
	display_str(dis_buf);
	ht1633_updata_display();
	dis_other_mode=10;
#endif
}



