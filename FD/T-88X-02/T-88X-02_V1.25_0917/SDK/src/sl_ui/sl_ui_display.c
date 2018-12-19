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

//#include "sl_ui_handle.h"


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

static  char bt_no_vol[5] = {NUM_B, NUM_t,NUM_OFF,NUM_OFF,NUM_OFF};
static  char aux_no_vol[5] = {NUM_A, NUM_U,NUM_X,NUM_OFF,NUM_OFF};
static  char opt_no_vol[5] ={NUM_O, NUM_P,NUM_T,NUM_OFF,NUM_OFF};
static  char hdmi_no_vol[5] ={NUM_H, NUM_D,NUM_OFF,NUM_OFF,NUM_OFF};
static  char coa_no_vol[5] ={NUM_C, NUM_O,NUM_A,NUM_OFF,NUM_OFF};
static  char fm_no_vol[5] ={NUM_OFF, NUM_F,NUM_M,NUM_OFF,NUM_OFF};
static  char usb_no_vol[5] ={NUM_U, NUM_S,NUM_B,NUM_OFF,NUM_OFF};




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

extern  int mute_state ;
extern unsigned int  fmFrequency;
extern unsigned char Cur_Fre_Num;

extern char mic_vol_flag;
extern char mic_echo_flag;


#define MCU_VER1 1
#define MCU_VER2 3

#define BT_VER1 1
#define BT_VER2 2

#define HOR_DIS 1
#define VER_DIS      2
#define ALL_DIS      3
#define NO_DIS       4

int led_test_mode = HOR_DIS;


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
	ht1633_clear_disbuf();
}
/**************************************



**************************************/


void display_ui_icon(char icon,bool on_off)
{
	ht1633_set_icon(icon,on_off);
}



void display_str( char *dis_str)
{
    if (dis_str != NULL)
    {
        int i;
        for (i = 0; i < 5; ++i)
        {
            ht1633_set_num_leter(i, dis_str[i],1);
        }

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

	ht1633_updata_display();
}

/****************************************************



****************************************************/
void display_ui_main_sys(char wm_mode)
{
	display_ui_device(wm_mode);
	ui_goback_source(400);
}


/*********************************************




***********************************************/
void display_ui_device(char wm_mode)
{

	display_ui_clear();
	switch(wm_mode)
	{
		case SOURCE_SELECT_BT:
			display_ui_icon(ICON_BT,bt_connected);
			display_str(bt_str);
			break;

		case SOURCE_SELECT_USB:
			display_ui_icon(ICON_USB,usb_online);
			display_str(usb_str);
			break;

		case SOURCE_SELECT_SD:
			// display_ui_icon(ICON_SD,sd_online);
			display_str(sd_str);
			break;

		case SOURCE_SELECT_FM:
			display_ui_icon(ICON_FM,1);
			display_str(fm_str);
			break;

		case SOURCE_SELECT_LINEIN:
			//display_ui_icon(ICON_AUX,aux_online);
			display_str(aux_str);
			break;

		case SOURCE_SELECT_COA:
			// display_ui_icon(ICON_COA,1);
			display_str(coa_str);
			break;

		case SOURCE_SELECT_SPDIFIN:
			display_ui_icon(ICON_OPTI,1);
			display_str(optical_str);
			break;

		case SOURCE_SELECT_HDMI:
			// display_ui_icon(ICON_HDMI,1);
			display_str(hdmi_str);
			break;
	}

	ht1633_updata_display();
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
	ht1633_updata_display();
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
	if(NULL != wdtimer_goback_mode)
	{
		wd_cancel(wdtimer_goback_mode);
	}


	if(input_flag)
	{
		ui_cmd_t cmd;
		cmd.cmd = UI_CMD_ENTER;
		send_cmd_2_ui(&cmd);
		input_flag=0;
	}

	dis_other_mode=0;
	if(!mute_state)
	{
		display_set_source(ui_source_select);
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

	char volume[5] = {0, 0, 0, 0,0};
	volume[3] = vol / 10;
	volume[4] =  vol % 10;

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

	if((ui_source_select==SOURCE_SELECT_USB)||(ui_source_select==SOURCE_SELECT_SD)||(ui_source_select==SOURCE_SELECT_FM))
	{
		 if(ui_source_select==SOURCE_SELECT_USB)
		{
			volume[0]=NUM_U;
			volume[1]=NUM_S;
			volume[2]=NUM_B;
		}
		else  if(ui_source_select==SOURCE_SELECT_SD)
		{
			volume[0]=NUM_OFF;
			volume[1]=NUM_S;
			volume[2]=NUM_D;
		}

		else if(ui_source_select==SOURCE_SELECT_FM)
		{
			volume[0]=NUM_OFF;
			volume[1]=NUM_F;
			volume[2]=NUM_M;
		}

		dis_other_mode=1;
		display_str(volume);
		ht1633_updata_display();

	}
	else
	{
		display_set_source(ui_source_select);
	}

}

/****************************************************



****************************************************/
void display_mic_vol(int vol)
{
	//int vol_i =(int)(vol / VOL_STEP);

	char volume[5] = {0, 0, 0, 0,0};
	volume[3] = vol / 10;
	volume[4] =  vol % 10;

	if((mic_vol_flag == true)||(mic_echo_flag == true))
	{
		if(mic_vol_flag ==       true)
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
		display_ui_clear();
		display_str(volume);
		ht1633_updata_display();

	}
	else
	{
		display_set_source(ui_source_select);
	}

}


/****************************************************



****************************************************/
void display_ui_usb(void)
{
	char dis[5]={NUM_U,NUM_S,NUM_B,NUM_N,NUM_O};
	if(usb_online)
	{
		ui_update_music_time();
		display_ui_icon(ICON_USB,usb_online);
	}
	else
	{
		display_str(dis);
		display_ui_icon(ICON_USB,usb_online);
	}
	ht1633_updata_display();
}


/****************************************************



****************************************************/
void display_ui_sd(void)
{
	char dis[5]={NUM_OFF,NUM_S,NUM_D,NUM_N,NUM_O};
	if(sd_online)
	{
		ui_update_music_time();
		//display_str(sd_str);
		//display_ui_icon(ICON_SD,sd_online);
		ht1633_updata_display();
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

	char volume[5] = {NUM_B, NUM_t,NUM_OFF,0,0};

	volume[3] = bt_mix_vol / 10;
	volume[4] = bt_mix_vol % 10;

	if(ui_source_select == SOURCE_SELECT_BT )
	{
		display_ui_clear();
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

		display_ui_icon(ICON_BT,bt_connected);
		ht1633_updata_display();
	}
}

/****************************************************



****************************************************/
void display_ui_fm(unsigned char f_ch)
{
	char dis_buf[5];
	display_ui_clear();
	if(f_ch==0)
	{
		display_ui_icon(ICON_DOT,1);
		if(fmFrequency<1000)
		{
			dis_buf[0]=NUM_OFF;
			dis_buf[4]=NUM_OFF;
			dis_buf[1]=(char)((fmFrequency/100%10)>=0?(fmFrequency/100%10):NUM_OFF);
			dis_buf[2]=(char)((fmFrequency/10%10)>=0?(fmFrequency/10%10):NUM_OFF);
			dis_buf[3]=(char)((fmFrequency%10)>=0?(fmFrequency%10):NUM_OFF);
		}
		else
		{
			dis_buf[4]=NUM_OFF;
			dis_buf[0]=(char)((fmFrequency/1000%10)>=0?(fmFrequency/1000%10):NUM_OFF);
			dis_buf[1]=(char)((fmFrequency/100%10)>=0?(fmFrequency/100%10):NUM_OFF);
			dis_buf[2]=(char)((fmFrequency/10%10)>=0?(fmFrequency/10%10):NUM_OFF);
			dis_buf[3]=(char)((fmFrequency%10)>=0?(fmFrequency%10):NUM_OFF);
		}
	}
	else
	{
		dis_other_mode=1;
		dis_buf[0]=NUM_OFF;
		dis_buf[1]=NUM_C;
		dis_buf[2]=NUM_H;
		dis_buf[3]=(char)((Cur_Fre_Num/10%10)>=0?(Cur_Fre_Num/10%10):NUM_OFF);
		dis_buf[4]=(char)((Cur_Fre_Num%10)>=0?(Cur_Fre_Num%10):NUM_OFF);
	}

	display_str(dis_buf);
	display_ui_icon(ICON_FM,1);

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
	ht1633_updata_display();
}


/****************************************************



****************************************************/
void display_ui_aux(void)
{
	char volume[5] = {NUM_A, NUM_U,NUM_X,0,0};
	aux_online=true;

	volume[3] = bt_mix_vol / 10;
	volume[4] = bt_mix_vol % 10;
	display_str(volume);
	//display_ui_icon(ICON_AUX,aux_online);
	ht1633_updata_display();

}

/****************************************************



****************************************************/
void display_ui_op(void)
{
	char volume[5] = {NUM_O, NUM_P,NUM_T,0,0};

	volume[3] = bt_mix_vol / 10;
	volume[4] = bt_mix_vol % 10;
	display_str(volume);

	display_ui_icon(ICON_OPTI,1);
	ht1633_updata_display();
}
/****************************************************



****************************************************/
void display_ui_hdmi(void)
{
	char volume[5] = {NUM_H, NUM_D,NUM_OFF,0,0};

	volume[3] = bt_mix_vol / 10;
	volume[4] = bt_mix_vol % 10;
	display_str(volume);

	//display_ui_icon(ICON_HDMI,1);
	ht1633_updata_display();
}





void display_ui_coaxial(void)
{

	//int vol_i = (int)(mix_vol / VOL_STEP);

	char volume[5] = {NUM_C, NUM_O,NUM_A,0,0};
	//printf("display_ui_coaxial vol = %d\n",vol_i);
	volume[3] = bt_mix_vol/ 10;
	volume[4] = bt_mix_vol % 10;
	display_str(volume);

	//display_ui_icon(ICON_HDMI,1);
	ht1633_updata_display();
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
		ui_info_t player_info = get_all_play_info();

		//   if (player_info.player_stat == 2 ||
		//      (player_info.player_stat == 3))
		//{   //播放状态或暂停显示隐藏状态
		int * seek_time_p = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
		int curtime = player_info.curtime;
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
		display_ui_icon(ICON_USB,usb_online);

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
			char display[5] = {NUM_OFF,min/10, min%10, sec/10, sec%10};
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
			display_ui_icon(ICON_CLON_2,colon_showed);
			//printf("%s:min = %d, sec = %d\n", __func__,min,sec);
		}
		else
		{
			showed = false;
			display_str(clear_str);
			//隐藏两点
			display_ui_icon(ICON_CLON_2,0);
		}
		//}

		ht1633_updata_display();
	}
#endif

}

/*****************************************************




*******************************************************/
void display_ui_input(unsigned int number)
{
	display_ui_clear();
	char dis_buf[5];
	if((number>=0)&&(number<=9999))
	dis_buf[0]=NUM_OFF;
	dis_buf[1]=(char)((number/1000%10)>=0?(number/1000%10):NUM_OFF);
	dis_buf[2]=(char)((number/100%10)>=0?(number/100%10):NUM_OFF);
	dis_buf[3]=(char)((number/10%10)>=0?(number/10%10):NUM_OFF);
	dis_buf[4]=(char)((number%10)>=0?(number%10):NUM_OFF);

	display_str(dis_buf);
	ht1633_updata_display();
	dis_other_mode=2;
	input_flag=1;

}




/**********************************************************




************************************************************/
void display_ui_mic(bool on_off)
{
	char dis_buf1[5]={NUM_M,NUM_I,NUM_C,NUM_O,NUM_N};
	char dis_buf2[5]={NUM_M,NUM_I,NUM_C,NUM_O,NUM_F};

	display_ui_clear();
	if(on_off==1)
		display_str(dis_buf1);
	else
		display_str(dis_buf2);
	ht1633_updata_display();
	dis_other_mode=2;
}

void display_ui_bass_vol(int mode,int vol)
{
	char dis_buf1[5]={NUM_B,NUM_A,NUM_0,NUM_0,NUM_0};
	char dis_buf2[5]={NUM_T,NUM_R,NUM_0,NUM_0,NUM_0};

	if(vol > 0)
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

	display_ui_clear();
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
	dis_other_mode=2;
}




/***********************************************************



************************************************************/
void dis_ui_updata_program(char on_off)
{
	char dis_buf1[5]={NUM_U,NUM_P,NUM_D,NUM_A,NUM__};
	char dis_buf2[5]={NUM__,NUM_O,NUM_K,NUM__,NUM__};

	display_ui_clear();
	if(on_off==0)
		display_str(dis_buf1);
	else
		display_str(dis_buf2);
	ht1633_updata_display();
}


void display_ui_version(int ver)
{
	char dis_buf1[5]={NUM__,NUM_U,0,0,NUM__};
	char dis_buf2[5]={NUM__,NUM_U,0,0,NUM__};

	display_ui_clear();
	if(ver==0)
	{
		dis_buf1[2] = MCU_VER1;
		dis_buf1[3] = MCU_VER2;
		display_str(dis_buf1);
	}
	else if(ver==1)
	{
		dis_buf2[2] = BT_VER1;
		dis_buf2[3] = BT_VER2;
		display_str(dis_buf2);
	}
	display_ui_icon(ICON_DOT,1);

	ht1633_updata_display();
}


void display_ui_init(void)
{
	char dis_buf[5]={NUM_R,NUM_S,NUM_t,NUM_OFF,NUM_OFF};

	display_ui_clear();

	display_str(dis_buf);

	ht1633_updata_display();
}

void display_ui_ledtest(void)
{
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
}

void display_ui_mute(void)
{
	static bool isDisp = true;

	char volume[5] = {NUM_B, NUM_t,NUM_OFF,0,0};

	volume[3] = bt_mix_vol / 10;
	volume[4] = bt_mix_vol % 10;

	if(ui_source_select == SOURCE_SELECT_BT )
	{
		if(bt_connected)
		{
			if(mute_state)
			{
				display_ui_clear();
				display_ui_icon(ICON_VOL_MUTE,1);
				display_ui_icon(ICON_VOL_UMUTE,0);
				if(!isDisp)
				{
					display_ui_bt();
					isDisp = true;
				}
				else
				{
					display_ui_icon(ICON_BT,bt_connected);
					//显示状态
					display_str(bt_no_vol);
					isDisp = false;
				}
				ht1633_updata_display();
			}
			else
			{
				display_set_source(ui_source_select);
			}

		}
		else
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
			ht1633_updata_display();
		}

	}
	else if(ui_source_select == SOURCE_SELECT_LINEIN )
	{
		if(mute_state)
		{
			display_ui_clear();
			display_ui_icon(ICON_VOL_MUTE,1);
			display_ui_icon(ICON_VOL_UMUTE,0);
			if(!isDisp)
			{
				display_ui_aux();
				isDisp = true;
			}
			else
			{
				//显示状态
				display_str(aux_no_vol);
				isDisp = false;
			}
			ht1633_updata_display();
		}
		else
		{
			display_set_source(ui_source_select);
		}

	}
	else if(ui_source_select == SOURCE_SELECT_SPDIFIN )
	{
		if(mute_state)
		{
			display_ui_clear();
			display_ui_icon(ICON_VOL_MUTE,1);
			display_ui_icon(ICON_VOL_UMUTE,0);
			if(!isDisp)
			{
				display_ui_op();
				isDisp = true;
			}
			else
			{
				//显示状态
				display_ui_icon(ICON_OPTI,1);
				display_str(opt_no_vol);
				isDisp = false;
			}
			ht1633_updata_display();
		}
		else
		{
			display_set_source(ui_source_select);
		}


	}
	else if(ui_source_select == SOURCE_SELECT_HDMI )
	{
		if(mute_state)
		{
			display_ui_clear();
			display_ui_icon(ICON_VOL_MUTE,1);
			display_ui_icon(ICON_VOL_UMUTE,0);
			if(!isDisp)
			{
				display_ui_hdmi();
				isDisp = true;
			}
			else
			{
				//显示状态
				display_str(hdmi_no_vol);
				isDisp = false;
			}
			ht1633_updata_display();
		}
		else
		{
			display_set_source(ui_source_select);
		}


	}
	else if(ui_source_select == SOURCE_SELECT_COA )
	{
		if(mute_state)
		{
			display_ui_clear();
			display_ui_icon(ICON_VOL_MUTE,1);
			display_ui_icon(ICON_VOL_UMUTE,0);
			if(!isDisp)
			{
				display_ui_coaxial();
				isDisp = true;
			}
			else
			{
				//显示状态
				display_str(coa_no_vol);
				isDisp = false;
			}
			ht1633_updata_display();
		}
		else
		{
			display_set_source(ui_source_select);
		}

	}
	else if(ui_source_select == SOURCE_SELECT_FM )
	{
		if(mute_state)
		{
			display_ui_clear();
			if(!isDisp)
			{
				display_ui_vol(bt_mix_vol);
				isDisp = true;
			}
			else
			{
				//显示状态
				display_ui_icon(ICON_VOL_MUTE,1);
				display_ui_icon(ICON_VOL_UMUTE,0);
				display_str(fm_no_vol);
				isDisp = false;
			}
			ht1633_updata_display();
		}
		else
		{
			display_set_source(ui_source_select);
		}

	}
	else if(ui_source_select == SOURCE_SELECT_USB )
	{
		if(mute_state)
		{
			display_ui_clear();

			if(!isDisp)
			{
				display_ui_vol(bt_mix_vol);
				isDisp = true;
			}
			else
			{
				display_ui_icon(ICON_VOL_MUTE,1);
				display_ui_icon(ICON_VOL_UMUTE,0);
				//显示状态
				display_str(usb_no_vol);
				isDisp = false;
			}
			ht1633_updata_display();
		}
		else
		{
			display_set_source(ui_source_select);
		}

	}
}

void display_ui_enter_tre_bass(int mode)
{
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
}




