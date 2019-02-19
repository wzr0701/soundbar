/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "ad82589.h"
#ifdef CONFIG_BLUETOOTH
#include "bluetooth.h"
#endif
#include "bt_uart.h"
#include <errno.h>
#include <fcntl.h>
#include <nuttx/audio/sladsp_ae.h>
#include <nuttx/power/pm.h>
#include <nxplayer.h>
#include "player_cmd.h"
#include <pthread.h>
#include <silan_gpio.h>
//#include "sc6138_led7.h"
#include <silan_addrspace.h>
#include <silan_resources.h>
#include "sl_lcd.h"
#include "sl_ui_cmd.h"
#include "sl_ui_handle.h"
#include "sl_ui_local.h"
#include <string.h>
#include <sys/ioctl.h>
#include "tone_data.h"
#include "sl_ui_display.h"
#include "ht1633.h"
#include "sl_ui_io_led.h"
#include <silan_cec.h>
#include "sl_ui_struct.h"
#include "sl_ui_cmd_deal.h"


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*调试打印开关*/
#define SL_UI_DBG
/*休眠standby*/
#define STANDBY_DELAY_SEC 10
/*开关机切频*/
#define FREQ_SWITCH 0


#define  AUX_DET_BY_BT    0


/****************************************************************************
 * Public Types
 ****************************************************************************/
/*播放状态*/
enum
{
    PLAY_START = -1,
    PLAY,
    STOP,
    PLAY_END,
} UI_SELECT_PLAY;
/*FM区域*/
typedef enum
{
    RADIO_AREA_START = -1,
    USA,
    GEN,
    JAPAN,
    RADIO_AREA_MAX,
} RADIO_AREA;

/****************************************************************************
 * Public Data
 ****************************************************************************/
/*外部*/
extern int ui_source_select;
extern void sl_ui_set_samp(void);

extern int  bass_vol;
extern int treble_vol;

int folder_index_cnt=0;
bool folder_dis_flag = false;

/*变量*/
/*aux连接状态*/
static bool aux_connected = false;
/*收音机当前AM频率*/

bool bt_connected = false;
/*mute脚状态*/
static int bt_mute_last_state = 0;
/*蓝牙开始检测状态信号量*/
static sem_t bt_start_check_sem;
/*蓝牙状态获取线程ID*/
static pthread_t bt_state_tid = -1;
#if BYPASS_MODE
/*bypass模式音量*/
static float bypass_in_vol = -12;
/*bypass音量补偿*/
static float bypass_vol_offset = 0;
#endif

static int cur_freq = 6000000;

extern bool frist_hdmi_init;

static int eq_type = -1;
/*音量模式标志*/
 char dis_other_mode = 0;
/*mix音量*/
int  mix_vol = Frist_MIX_VOL;
int  bt_mix_vol = Frist_MIX_LEV;
/*静音状态标志*/
 int mute_state = 0;
/*播放器信息*/
ui_info_t playerInfo;

/*SD上一次播放的歌曲序号*/
static int sd_last_file_index = -1;
/*SD有音频文件的文件夹数量*/
static int sd_last_folder_num = -1;
/*SD总文件夹数量*/
static int sd_last_total_num = -1;
/*播放时间*/
 int sd_playtime = 0;
/*USB上一次播放的歌曲序号*/
static int usb_last_file_index = -1;
/*USB有音频文件的文件夹数量*/
static int usb_last_folder_num = -1;
/*USB总文件夹数量*/
static int usb_last_total_num = -1;
/*播放时间*/
 int usb_playtime = 0;
/*长按物理上下区处理看门狗*/
static WDOG_ID wdtimer_action_longpress = NULL;
/*mute检测处理看门狗*/
 WDOG_ID wdtimer_bt_mute_detect = NULL;

 WDOG_ID wdtimer_pa_station_detect = NULL;

 /*进入工厂测试模式组合键看门狗*/
//WDOG_ID wdtimer_enter_testmode_detect = NULL;

 WDOG_ID wdtimer_hdmion_send = NULL;

 /*切换模式解MUTE看门狗*/
WDOG_ID wdtimer_change_mode_unmute = NULL;

/*延时显示模式字符串看门狗*/

/*长按物理音量键处理看门狗*/
static WDOG_ID wdtimer_vol_longpress = NULL;


 WDOG_ID wdtimer_goback_mode;

 WDOG_ID wdtimer_display_updata;

//WDOG_ID wdtimer_display_bt_wait;

/*��ǰTREBLE����*/
static int treble_level = 0;
/*��ǰBASS����*/
static int bass_level = 0;

bool next_folder_flag = false;
bool prev_folder_flag = false;

/*全局*/
/*开关机状态*/
int ui_power = POWER_OFF;
int folder_index_dis = 0;

unsigned char bt_version_num = 0;

bool take_micmute_flag = false;

extern bool bt_wait_flag;
extern int bt_wait_cnt;

extern char mic_on_flag;
extern bool mic_detect_online;


unsigned char power_on_usb_sd_auto_play;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/*外部*/
extern void em_player_tone(int index);
extern int fat_get_unicode_size(WORD *ustr);
extern int fat_utf8_to_unicode(const char* str, WORD *ustr, int len);
extern void send_cmd_2_ui(ui_cmd_t *ui_cmd);
extern void sysfs_sd_umount(void);
extern void sysfs_usb_umount(void);
extern void time2str(int curtime, int totaltime, char *str);
extern void usb_manual_disconnect(void);
extern int sl_func_add(void);


/*内部*/
extern int mic_vol;
extern int echo_vol_lev ;
extern int mic_vol_lev;
extern int tre_bass_cnt;
extern bool enter_tre_set;
extern bool enter_bass_set;

static void ui_process_vol_dec(void);
static void ui_process_vol_inc(void);



#define eq_set_ch		6 

//music
const int eq_table_1[]={
	 //flt_id, type, onoff, gain, fc, q, ch 
	 1, 0, 1, -1100, 100, 100, eq_set_ch,//band1 
	 2, 0, 1, -800, 300, 100, eq_set_ch,//band2 
	 3, 0, 1, 0, 1000, 100, eq_set_ch,//band3 
	 4, 0, 1, -800, 3000, 300, eq_set_ch,//band4 
	 5, 0, 1, -1200, 8000, 300, eq_set_ch,//band5 

};

//movie
const int eq_table_2[]={
	//flt_id, type, onoff, gain, fc, q
	1, 0, 1, 300, 101, 100, eq_set_ch,//band1 
	2, 0, 1, -400, 198, 100, eq_set_ch,//band2 
	3, 0, 1, 400, 657, 100, eq_set_ch,//band3 
	4, 0, 1, 200, 3073, 300, eq_set_ch,//band4 
	5, 0, 1, 400, 13867, 300, eq_set_ch,//band5 

};
	
//dialog
const int eq_table_3[]={
	//flt_id, type, onoff, gain, fc, q
	1, 0, 1, -1100, 100, 100, eq_set_ch,//band1 
	2, 0, 1, -800, 300, 100, eq_set_ch,//band2 
	3, 0, 1, 0, 1000, 100, eq_set_ch,//band3 
	4, 0, 1, -800, 3000, 300, eq_set_ch,//band4 
	5, 0, 1, -1200, 8000, 300, eq_set_ch,//band5 

};

void ui_handle_eq_music(void)
{
	int i,j;
	char set_eq_text[64];
	swa_audio_audproc_load(AUDPROC_LIST_MIX, AUDPROC_EQ);
	usleep(100);
	for (i = 0, j = 0; i < 5; i++, j += 7)
	{
		swa_audio_audproc_eq(AUDPROC_LIST_MIX, (ae_eq_para *)&eq_table_1[j]);
		swa_audio_audproc_set(AUDPROC_LIST_MIX, AUDPROC_EQ);
	}		
}

void ui_handle_eq_movie(void)
{
	int i,j;
	char set_eq_text[64];
	
	swa_audio_audproc_load(AUDPROC_LIST_MIX, AUDPROC_EQ);
	usleep(100);
	
	for (i = 0, j = 0; i < 5; i++, j += 7)
	{
		swa_audio_audproc_eq(AUDPROC_LIST_MIX, (ae_eq_para *)&eq_table_2[j]);
		swa_audio_audproc_set(AUDPROC_LIST_MIX, AUDPROC_EQ);
	}		
}

void ui_handle_eq_dialog(void)
{
	int i,j;
	char set_eq_text[64];
	swa_audio_audproc_load(AUDPROC_LIST_MIX, AUDPROC_EQ);
	usleep(100);
	for (i = 0, j = 0; i < 5; i++, j += 7)
	{
		swa_audio_audproc_eq(AUDPROC_LIST_MIX, (ae_eq_para *)&eq_table_3[j]);
		swa_audio_audproc_set(AUDPROC_LIST_MIX, AUDPROC_EQ);
	}		
}

void ui_handle_unload_eq(void)
{
	swa_audio_audproc_unload(AUDPROC_LIST_MIX, AUDPROC_EQ);
	usleep(100);
}
/******************************************





******************************************/
void  save_trebass_level(int mode)
{
	unsigned char temp;
	
	if(mode == 0)//bass
	{
		temp = bass_vol+5;
		if((temp >= 0) && (temp <=10))
		{
			at24c02_write_one_byte(MEM_BASS_LEVEL,temp);
			//printf("%s:SAVE_BASS_temp = %d.\n",  __func__,temp);
		}			
	}
	else//treble
	{
		temp = treble_vol+5;
		if((temp >= 0) && (temp <=10))
		{
			at24c02_write_one_byte(MEM_TREBLE_LEVEL,temp);
			//printf("%s:SAVE_TREBLE_temp = %d.\n",  __func__,temp);
		}	
		
	}
}


/******************************************





******************************************/
void  read_trebass_level(int mode)
{
	unsigned char temp;
	
	if(mode == 0)//bass
	{
		temp = at24c02_read_one_byte(MEM_BASS_LEVEL);
		//printf("%s:READ_BASS_temp = %d.\n",  __func__,temp);
		if((temp >= 0) && (temp <=10))
		{
			bass_vol = temp-5;
		}
		else
		{
			bass_vol = 0;
		}
			
	}
	else//treble
	{
		temp = at24c02_read_one_byte(MEM_TREBLE_LEVEL);
		//printf("%s:READ_TREBLE_temp = %d.\n",  __func__,temp);
		if((temp >= 0) && (temp <=10))
		{
			treble_vol = temp-5;
		}
		else
		{
			treble_vol = 0;
		}
		
	}
	//printf("%s:treble_level = %d--- bass_level = %d.\n",  __func__, treble_level,bass_level);

}


static bool loaded = false;
static void update_bass_treble(int * pLevel, bool bass_or_treble)
{
    if (bass_level == 0 && treble_level == 0)
    {//û��bass��treble
        if (loaded)
        {
            int para[7] = {0, 0, 0, 0, 0, 0, 6};
            int i;
            //�����ǰEQ��ֵ
            for(i=0;i<2;++i)
            {
                para[0] = i+1;
                swa_audio_audproc_eq(AUDPROC_LIST_MIX, (ae_eq_para *)para);
                swa_audio_audproc_set(AUDPROC_LIST_MIX, AUDPROC_EQ);
            }
            //ж��EQ��ֵ
            swa_audio_audproc_unload(AUDPROC_LIST_MIX, AUDPROC_EQ);
            loaded = false;
        }
    }
    else
    {//��bass��treble
    	int para[7] = {1, 0, 1, 0, 100, 300, 6};

		para[3] = (*pLevel) * BASS_TREBLE_GAIN_STEP;

        //����EQֵ
        //if(!loaded)
        {
            loaded = true;
            //����EQ��ֵ
            swa_audio_audproc_load(AUDPROC_LIST_MIX, AUDPROC_EQ);
        }

        //filter_id=?, eq_type=0, on_off=1, gain=?, fc=?, q=300, ch=6
		//para[0] = bass_or_treble?6:7;
        //para[4] = bass_or_treble?90:12000;

		para[0] = bass_or_treble?1:2;
        para[4] = bass_or_treble?45:12000;
		para[5] = bass_or_treble?70:100;
//		printf("para 0:%d 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d \n", para[0], para[1], para[2], para[3],
//			para[4], para[5], para[6]);
        swa_audio_audproc_eq(AUDPROC_LIST_MIX, (ae_eq_para *)para);
        swa_audio_audproc_set(AUDPROC_LIST_MIX, AUDPROC_EQ);
    }
}


void handle_bass_treble(int mode, int level)
{
    //�Ƿ��Ѿ���ʼ����
    bool bass_or_treble = false;
    int *pLevel = &level;
	if (mode > 1)
	{
		bass_or_treble = false; //treble
	}
	else
	{
		bass_or_treble = true;//bass
	}
	//printf("handle_bass_treble plevel:%d bass_or_treble %d\n", *pLevel, bass_or_treble);
    if(level >= BASS_TREBLE_LEVEL_MIN && level <= BASS_TREBLE_LEVEL_MAX)
    {
        //�����µļ���
		if (bass_or_treble)
		{
			bass_level = *pLevel;
		}
		else
		{
			treble_level = *pLevel;
		}
		//printf("%s:bass_level = %d.treble_level = %d\n",__func__, bass_level, treble_level);
        //����BASS��TREBLE��ֵ
        update_bass_treble(pLevel, bass_or_treble);
    }
}



/****************************************************************************
 * Name: ui_handle_power
 *
 * Description:
 *    开关机处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_power(int power_on_off)
{
	unsigned char temp;

	printf("POWER==%d\r\n",power_on_off);

	ui_power=power_on_off;

	if (ui_power == POWER_ON)
	{
#if (FREQ_SWITCH)
		enter_dynamic();
		printf("out of low power\n");
#endif
		printf("POWERon%d\r\n");

		//启动蓝牙模块通信线程
		if (bt_state_tid < 0)
		{
			if(pthread_create(&bt_state_tid, NULL, (pthread_startroutine_t)bt_read_state, NULL) == 0)
			{
				printf("%s:Create bluetooth communitcate thread success\n", __func__);
			}
			else
			{
				printf("%s:Create bluetooth communitcate thread fail\n", __func__);
			}
		}
		else
		{
			printf("%s:pid = %d\n", __func__, bt_state_tid);
		}

		usleep(100);

		//发送信号量开始蓝牙信号检测
		if(sem_post(&bt_start_check_sem) == 0)
		{
			int value = 0;
			sem_getvalue(&bt_start_check_sem, &value);
			printf("%s,%d:Send semaphore to continue bluetooth check state success\n", __func__, value);
		}
		else
		{
			printf("%s:Send semaphore to continue bluetooth check state fail\n", __func__);
		}
#if 0
		//开启蓝牙mute同步看门狗
		if(wdtimer_bt_mute_detect == NULL)
		{
			wdtimer_bt_mute_detect = wd_create();
		}

#endif
		if (NULL == wdtimer_display_updata)
		{
			wdtimer_display_updata = wd_create();
		}

		if(NULL == wdtimer_pa_station_detect)
		{
			wdtimer_pa_station_detect = wd_create();
		}

#if 0
		if(NULL == wdtimer_enter_testmode_detect)
		{
			wdtimer_enter_testmode_detect = wd_create();
		}
#endif

		if(NULL == wdtimer_change_mode_unmute)
		{
			wdtimer_change_mode_unmute = wd_create();
		}

		if(NULL == wdtimer_hdmion_send)
		{
			wdtimer_hdmion_send = wd_create();
		}

		/////////////////////////////////////////////

		frist_hdmi_init = true;

		//////////////////////////////////////////////

		//  led7_open();
		pa_io_ret_set(true);
		pa_static_check();
		//sl_func_add();
		dis_play_update();
		enter_othermode_check();
		change_mode_unmute();

		read_player_info();
		read_mix_vol();
		read_trebass_level(BASS_MODE);
		read_trebass_level(TREBLE_MODE);	
		set_bass_treble_vol(BASS_MODE,bass_vol,0);
		set_bass_treble_vol(TREBLE_MODE,treble_vol,0);
		//ui_source_select = SOURCE_SELECT_START;
		ui_handle_mode(ui_source_select, false);
		bt_cmd_source_select(ui_source_select);
		bt_cmd_current_bass(bass_vol); //bass
		bt_cmd_current_treble(treble_vol); //treble

#if 0
		usleep(500000);
		sc8836_action_hdmi_on();
		usleep(1000);
		sc8836_action_hdmi_poweron();
		usleep(1000);
#endif

	}
	else
	{
		printf("POWEofff%d\r\n");
		pa_mute_ctrl(true);
		display_ui_power(0);
		//save_player_info();

		usleep(500000);
		player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);
		usleep(100);

#if BYPASS_MODE
		silan_set_linebypass_disable();
		usleep(100);
#endif

#if (BYPASS_MODE || INNERADC_MODE)
		player_process_cmd(NP_CMD_LINEIN_OFF, NULL, 0, NULL, NULL);
		usleep(100);
#else
		player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 1, NULL, NULL);
		usleep(100);
#endif

		player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
		usleep(100);

		//清除EQ功能
		ui_handle_eq(-1);

		//关闭延时显示音源看门狗
		if (wdtimer_goback_mode != NULL)
		{
			wd_cancel(wdtimer_goback_mode);
		}


		if(wdtimer_pa_station_detect != NULL)
		{
			wd_cancel(wdtimer_pa_station_detect );
		}
#if 0
		if(wdtimer_enter_testmode_detect != NULL)
		{
			wd_cancel(wdtimer_enter_testmode_detect );
		}

#endif
		if(wdtimer_change_mode_unmute != NULL)
		{
			wd_cancel(wdtimer_change_mode_unmute );
		}


		if(wdtimer_display_updata != NULL)
		{
			wd_cancel(wdtimer_display_updata );
		}

		//清除蓝牙状态标志
		bt_connected = false;

		//ui_source_select = SOURCE_SELECT_START;

		//关闭LED 7段屏
		//led7_off();

		display_ui_power(2);
		usleep(500000);
		//while(1)
			//pa_mute_ctrl(true);

#if (FREQ_SWITCH)
		printf("enter low power\n");
		enter_dynamic();
#endif
	}
}



/****************************************************************************
 * Name: ui_handle_mode
 *
 * Description:
 *    模式切换处理
 *
 * Parameters:
 *    source 要切换的模式
 *    notify 是否需要通知3266
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_mode(int source, bool notify)
{
#ifdef SL_UI_DBG
    printf("%s.....\n", __func__);
#endif


     if (SOURCE_SELECT_INC <= source && source < SOURCE_SELECT_END)
    {

        if(source == SOURCE_SELECT_INC)
        {
            //切换模式
            if (ui_source_select < SOURCE_SELECT_END - 2)
            {
                ui_source_select++;
            }
            else
            {
                ui_source_select = SOURCE_SELECT_START + 1;
            }
        }
        else if(SOURCE_SELECT_START < source && source < SOURCE_SELECT_END)
        {
            ui_source_select = source;
        }

       printf("start_play_ui_source_select==%d\r\n",ui_source_select);

    }

}





/****************************************************************************
 * Public Functions
 ****************************************************************************/
void bt_mute_detect(void)
{
    static int count = 0;
    uint32_t value;
    //获取IO口状态
    zhuque_bsp_gpio_set_mode(BT_MUTE_DETECT_PIN, GPIO_IN, PULLING_HIGH);
    zhuque_bsp_gpio_get_value(BT_MUTE_DETECT_PIN, &value);
    if(value != bt_mute_last_state)
    {   //IO口有跳变
        count = 0;
        bt_mute_last_state = value;
    }
    else
    {   //IO口没变化
        if(count < 5)
        {
            count++;
        }
        if(count == 5)
        {
            count++;
            bt_mute_last_state = value;
            if(value == 0)
            {
                //关闭功放
                pa_mute_ctrl(true);
                printf("%s:AMP off.\n", __func__);
            }
            else
            {
                //打开功放
                pa_mute_ctrl(false);
                printf("%s:AMP on.\n", __func__);
            }
        }
    }

    if(wdtimer_bt_mute_detect != NULL)
    {
        //设置5ms检测一次
        wd_start(wdtimer_bt_mute_detect, CLOCKS_PER_SEC/200, (wdentry_t)bt_mute_detect, 0);
    }
}




/****************************************************************************
 * Name: aux_set_connect_state
 *
 * Description:
 *    设置aux连接状态
 *
 * Parameters:
 *    state true连接，false未连接
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void aux_set_connect_state(bool state)
{
    aux_connected = state;

    //自动模式切换处理
    if(ui_source_select != SOURCE_SELECT_LINEIN && state)
    {//非LINEIN状态
        //切换到LINEIN状态
        ui_handle_mode(SOURCE_SELECT_LINEIN, false);
    }
}



/****************************************************************************
 * Name: bt_cmd_translate
 *
 * Description:
 *    状AT命令转换志UI命令
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
enum ui_cmd_e bt_cmd_translate(AT_CMD cmd)
{
    const int UI_CMDS[] = {
        UI_CMD_NULL,        //进入配对
        UI_CMD_NULL,        //连接最后配对的设备
        UI_CMD_NULL,        //断开连接
        UI_CMD_NULL,        //ACK
        UI_CMD_NULL,        //拒接电话
        UI_CMD_NULL,        //挂断电话
        UI_CMD_NULL,        //重拨最后一个电话
        UI_CMD_VOLUME_INC,       //音量加
        UI_CMD_VOLUME_DEC,     //音量减
        UI_CMD_NULL,        //清除记忆列表
        UI_CMD_NULL,        //
        UI_CMD_NULL,        //语音拨号
        UI_CMD_NULL,        //
        UI_CMD_PLAY_PAUSE,       //播放/暂停
        UI_CMD_NULL,        //暂停
        UI_CMD_NEXT,     //下一曲
        UI_CMD_PREV,     //上一曲
        UI_CMD_NULL,        //下一个动作
        UI_CMD_NULL,        //倒带
        UI_CMD_NULL,        //开机
        UI_CMD_NULL,        //关机
        UI_CMD_GET_MODE,    //获取当前模式
        UI_CMD_GET_VOLUME,  //获取当前音量值
        UI_CMD_MODE,     //蓝牙模式
        UI_CMD_MODE,     //AUX模式
        UI_CMD_NULL,        //退出模式
        UI_CMD_MODE,     //下一个模式
        //蓝牙状态命令
        UI_CMD_BLUETOOTH_CONNECT,       //设备连接成功
        UI_CMD_BLUETOOTH_DISCONNECT,    //设备断开连接
        UI_CMD_NULL,        //蓝牙播放暂停
        UI_CMD_NULL,        //蓝牙开始播放
        UI_CMD_NULL,        //查询蓝牙连接状态
        UI_CMD_NULL,        //AUX连接状态查询
        UI_CMD_AUX_CONNECT, //AUX连接
        UI_CMD_AUX_DISCONNECT,  //AUX断开连接
        UI_CMD_NULL,        //复位I2S
        //带参数命令
        UI_CMD_NULL,        //模式设置
        UI_CMD_NUMBER,      //数字
        UI_CMD_NULL, 	    //BT 版本
        UI_CMD_VOLUME_SET,  //音量设置
        UI_CMD_VOLUME_MUTE,       //音效设置
        UI_CMD_PLAY_TONE,   //TONE播放
    };

    if(AT_START < cmd && cmd < AT_MAX)
    {//有效的命令
        return UI_CMDS[cmd];
    }

    return UI_CMD_NULL;
}


typedef struct app_value_s
{
    char app_value[20];
    enum ui_cmd_e cmd;
    enum ui_cmd_e cmd_dn;
    enum ui_cmd_e cmd_up;
} APP_VALUE_S;

APP_VALUE_S APP_CMD[] =
{
	{"MBTBT\r\n",UI_CMD_GO_TO_BT},      //BT
	{"MBTCA\r\n",UI_CMD_GO_TO_COA},      //COA
	{"MBTHI\r\n",UI_CMD_GO_TO_HDMI},      //HDMI
	{"MBTFM\r\n",UI_CMD_GO_TO_FM},      //FM
	{"MBTUB\r\n",UI_CMD_GO_TO_USB},      //USB
	{"MBTAX\r\n",UI_CMD_GO_TO_AUX},      //AUX
	{"MBTOT\r\n",UI_CMD_GO_TO_SPDIF},      //OPT
	{"MTMICO\r\n",UI_CMD_APP_MIC_ON},      //MIC ON 
	{"MTMICC\r\n",UI_CMD_APP_MIC_OFF},      //MIC OFF
	{"MTMOVO\r\n",UI_CMD_APP_MOVIE_ON},      //MOVIE ON
	{"MTMOVC\r\n",UI_CMD_APP_MOVIE_OFF},      //MOVIE OFF
	{"BTTUNEA\r\n",UI_CMD_FM_TUNE_ADD},      //TUNE +
	{"BTTUNES\r\n",UI_CMD_FM_TUNE_SUB},      //TUNE -
	{"BTSCAN\r\n",UI_CMD_FM_SCAN},      //FM SCAN
	{"MTNEXT\r\n",UI_CMD_NEXT},      //NEXT
	{"MTPREV\r\n",UI_CMD_PREV},      //PREV
	{"MTPAUSE\r\n",UI_CMD_APP_PAUSE},      //PAUSE
	{"MTPLAY\r\n",UI_CMD_APP_PLAY},      //PLAY
	{"BTMEM\r\n",UI_CMD_FM_MANUAL_SAVE},      //MEM 
	{"BTGUSB\r\n",UI_CMD_GET_USB_PLAY_STATUS},  //GET USB PLAY STATUS  
};

int str_cmp_80(char *str1,char *str2)
{
	int i = 0;
	int count_ss1 = 0;
	int count_ss2 = 0;
	int len1 = strlen(str1);
	int len2 = strlen(str2);

	if(len1 == len2)
	{
		if(strstr(str1,str2) != NULL)
			return 1;
		else
			return 0;
	}
	else if(len1 < len2)
	{
		while(count_ss2 < len2)
		{
			if(str1[count_ss1] == str2[count_ss2])
			{
				count_ss1++;
				i++;
			}
			count_ss2++;
		}
		
		if(i > 12)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

enum ui_cmd_e app_cmd_translate(char *cmd)
{
    int i;

    for(i = 0; i< sizeof(APP_CMD)/sizeof(APP_CMD[0]); i++)
    {
        //if(strstr(cmd,BT_CMD[i].bt_value) != NULL)
        if(str_cmp_80(cmd,APP_CMD[i].app_value))
        {
			//printf("%s:cmd = %d\n",__func__,APP_CMD[i].cmd);
			return APP_CMD[i].cmd;
        }
    }

    return UI_CMD_NULL;
}


/****************************************************************************
 * Name: bt_init_sem
 *
 * Description:
 *    蓝牙同步信号量初始化
 *
 * Parameters:
 *    en 1-使能，0-禁用
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void bt_init_sem(void)
{
    //初始化蓝牙开始检测状态信号量
    sem_init(&bt_start_check_sem, 0, 1);
}

int bt_cmd_check(char *buf_recv)
{
    ui_cmd_t cmd = {0};
    cmd.cmd = UI_CMD_NULL;
    int value;
    int index = bt_chk_AT_cmd(buf_recv, &value);

	if(index > AT_START && index < AT_MAX)
    {
		printf("%s:cmd.arg2=%d\n",__func__, value);
        //int ui_cmd = bt_cmd_translate(index);
        if ((index == AT_PLAY_OR_PAUSE)||(index == AT_PLAY_PAUSE))
        {
            cmd.cmd = UI_CMD_PLAY_PAUSE;
			cmd.arg2 = 0;
        }
		else if((index == AT_NEXT_TRACE)||(index == AT_PLAY_NEXT)) 
		{
			cmd.cmd = UI_CMD_NEXT;
			cmd.arg2 = 0;
		}
		else if((index == AT_PREV_TRACE)||(index == AT_PLAY_PREV))  
		{
			cmd.cmd = UI_CMD_PREV;
			cmd.arg2 = 0;
		}
        else if (index == AT_DEVICE_CONNECTED)
        {//设备连接
        	printf("BT conneted\n");
			cmd.cmd = UI_CMD_BLUETOOTH_CONNECT;
			cmd.arg2 = true;
        }
        else if (index == AT_DEVICE_DISCONNECTED)
        {//设备断开连接
        	bt_wait_cnt = 0;
			bt_wait_flag = false;
			cmd.cmd = UI_CMD_BLUETOOTH_DISCONNECT;
			cmd.arg2 = false;
        	printf("BT disconneted\n");
        }  
		else if(index == AT_VERSION)
        {
			//BT 版本号
			bt_version_num = value;
            printf("bt_version_num = %d\n", value);
        }
		else if(index == AT_SET_MAINVOL)
        {
			cmd.cmd = UI_CMD_VOLUME_SET;
			cmd.arg2 = value;
        }
		else if(index == AT_SET_TREBLE)
        {
			cmd.cmd = UI_CMD_TREBLE_SET;
			cmd.arg2 = value;
        }
		else if(index == AT_SET_BASS)
        {
			cmd.cmd = UI_CMD_BASS_SET;
			cmd.arg2 = value;
        }
		else if(index == AT_SET_ECHO)
        {
			cmd.cmd = UI_CMD_ECHO_SET;
			cmd.arg2 = value;
        }
		else if(index == AT_SET_MICVOL)
        {
			cmd.cmd = UI_CMD_MICVOL_SET;
			cmd.arg2 = value;
        }

		if((index == AT_MIC_ON)||(index == AT_MIC_OFF)||(index == AT_MOVIE_ON)||(index == AT_MOVIE_ON))
		{
			cmd.cmd = UI_CMD_NULL;
		}
		 
		if(cmd.cmd != UI_CMD_NULL)
        {
            send_cmd_2_ui(&cmd);
			cmd.cmd = UI_CMD_NULL;
        }
    }
	return cmd.cmd;
}

int app_cmd_check(char *buf_recv)
{
    return app_cmd_translate(buf_recv);
}



/****************************************************************************
 * Name: bt_read_state
 *
 * Description:
 *    蓝牙状态检测线程入口函数
 *
 * Parameters:
 *    en 1-使能，0-禁用
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void bt_read_state(void)
{
    const int buf_szie = 64;
    int ret;
	int long_time_count = 0;
    char buf_recv[65] = {0};
    char * ptr = buf_recv;
    ui_cmd_t cmd = {0};
    cmd.cmd = UI_CMD_NULL;

    while (1)
    {
        if(ui_power == POWER_ON)
        {
            ret = bt_read(ptr, buf_szie);
            if (ret > 0)
            {
				printf("%s:%s",__func__,buf_recv);
                int ui_cmd = bt_cmd_check(buf_recv);
                if(ui_cmd == UI_CMD_NULL)
                {
                    ui_cmd = app_cmd_check(buf_recv);
					if(ui_cmd != UI_CMD_NULL)
	                {
	                    cmd.cmd = ui_cmd;
	                    send_cmd_2_ui(&cmd);
						ui_cmd = UI_CMD_NULL;
	                }
                }
            }

            usleep(1000);
        }
    }
}


/****************************************************************************
 * Name: bt_set_connect_state
 *
 * Description:
 *    设置蓝牙连接状态
 *
 * Parameters:
 *    state true连接，false未连接
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void bt_set_connect_state(bool state)
{
    bt_connected = state;
}

void bt_cmd_fmscan_start(void)
{
	handle_bt_cmd(AT_FMSCAN_START, 0);
	usleep(60000);
}


void bt_cmd_fmscan_end(void)
{
	handle_bt_cmd(AT_FMSCAN_END, 0);
	usleep(60000);
}


void bt_cmd_usb_playstatus(char status)
{
	if(status)//play
	{
		handle_bt_cmd(AT_PLAY, 0);
	}
	else//pause
	{
		handle_bt_cmd(AT_STOP, 0);
	}
	usleep(60000);
}


void bt_cmd_mic_status(char status)
{
	if(status)//on
	{
		handle_bt_cmd(AT_MIC_ON, 0);
	}
	else//off
	{
		handle_bt_cmd(AT_MIC_OFF, 0);
	}
	usleep(60000);
}

void bt_cmd_movie_status(char status)
{
	if(status)//on
	{
		handle_bt_cmd(AT_MOVIE_ON, 0);
	}
	else//off
	{
		handle_bt_cmd(AT_MOVIE_OFF, 0);
	}
	usleep(60000);
}


void bt_cmd_current_mainvol(void)
{
    //通知bt当前的音量
    handle_bt_cmd(AT_CUR_MAINVOL, bt_mix_vol);
	usleep(60000);
}

void bt_cmd_current_treble(int vol)
{
    //通知bt当前的音量
    handle_bt_cmd(AT_CUR_TREBLE, vol+5);
	usleep(60000);
}

void bt_cmd_current_bass(int vol)
{
    //通知bt当前的音量
    handle_bt_cmd(AT_CUR_BASS, vol+5);
	usleep(60000);
}

void bt_cmd_current_echo(void)
{
    //通知bt当前的音量
    handle_bt_cmd(AT_CUR_ECHO, echo_vol_lev);
	usleep(60000);
}


void bt_cmd_current_micvol(void)
{
    //通知bt当前的音量
    handle_bt_cmd(AT_CUR_MICVOL, mic_vol_lev);
	usleep(60000);
}




void bt_cmd_source_select(int source)
{
	switch(source)
	{
		case SOURCE_SELECT_COA:
			handle_bt_cmd(AT_MODE_COA, 0);
			break;

		case SOURCE_SELECT_FM:
			handle_bt_cmd(AT_MODE_FM, 0);
			break;

		case SOURCE_SELECT_BT:
			handle_bt_cmd(AT_MODE_BT, 0);
			break;

		case SOURCE_SELECT_USB:
			handle_bt_cmd(AT_MODE_USB, 0);
			break;

		case SOURCE_SELECT_LINEIN:
			handle_bt_cmd(AT_MODE_AUX, 0);
			break;

		case SOURCE_SELECT_SPDIFIN:
			handle_bt_cmd(AT_MODE_OPT, 0);
			break;

		case SOURCE_SELECT_HDMI:
			handle_bt_cmd(AT_MODE_HDMI, 0);
			break;
	}

	usleep(60000);
}


void bt_cmd_dis_connect(void)
{
	//printf("%s : %d\n", __func__, __LINE__);
	bt_set_connect_state(false);
	/*直接显示蓝牙图标闪烁，无需等接收到ATWC，因为接收有时不准*/
	handle_bt_cmd(AT_DISCONNECT, 0);
	usleep(60000);
}


/***********************************************************


************************************************************/
void bt_cmd_get_version(void)
{
     handle_bt_cmd(AT_GET_VERSION, 0);
	 usleep(60000);
}

void bt_cmd_reset_iis(void)
{
	handle_bt_cmd(AT_IS, 0);
	usleep(60000);
}

/***********************************************************


************************************************************/
void bt_cmd_play_pause(void)
{
     handle_bt_cmd(AT_PLAY_OR_PAUSE, 0);
	 usleep(60000);
}

/***********************************************************


************************************************************/
void bt_cmd_next_song(void)
{
	handle_bt_cmd(AT_NEXT_TRACE, 0);
	usleep(60000);
}

/***********************************************************


************************************************************/
void bt_cmd_prev_song(void)
{
	handle_bt_cmd(AT_PREV_TRACE, 0);
	usleep(60000);
}

/****************************************************************************
 * Name: ui_handle_get_volume
 *
 * Description:
 *    获取音量消息处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void bt_handle_get_volume(void)
{
    //通知3266当前的音量
    handle_bt_cmd(AT_SET_VOL, mix_vol/VOL_STEP);
	usleep(60000);
}


/****************************************************************************
 * Name: ui_handle_get_mode
 *
 * Description:
 *    获取模式消息处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void bt_handle_get_mode(void)
{
    //通知3266当前的模式
    handle_bt_cmd(AT_SET_MODE, ui_source_select);
	usleep(60000);
}


/****************************************************************************
 * Name: ui_handle_prev_action
 *
 * Description:
 *    快进处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_bt_rewind(void)
{
	handle_bt_cmd(AT_REWIND_ACTION, 0);
	usleep(60000);
}






/****************************************************************************
 * Name: ui_handle_next_action
 *
 * Description:
 *    快进处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_bt_forward(void)
{
	handle_bt_cmd(AT_FORWARD_ACTION, 0);
	usleep(60000);
}



#ifdef CONFIG_PM
/****************************************************************************
 * Name: enter_dynamic
 *
 * Description:
 *    ��Ƶ�л�����
 *
 * Parameters:
 *    arg �߳����
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
 void enter_dynamic(void)
{
    int fd, ret = 0;

    fd = open(CONFIG_PM_DEV_PATH, O_RDWR);
    if (fd < 0)
    {
        printf("%s:open /dev/pm error\n", __func__);
    }
    else
    {
        if (cur_freq == 204000000)
        {
            cur_freq = 6000000;
            ret = ioctl(fd, PMIOC_DYNAMICFREQ, 6000000);
            printf("%s:Switch to 6 MHz freq\n", __func__);
        }
        else if (cur_freq == 6000000)
        {
            cur_freq = 204000000;
            ret = ioctl(fd, PMIOC_DYNAMICFREQ, 204000000);
            printf("%s:Switch to 204 MHz freq\n", __func__);
        }

        /* update time throld to 60s */
        if (ret != 0)
        {
            printf("%s:Switch freq fail\n", __func__);
            pm_sync(STANDBY_DELAY_SEC);
        }

        close(fd);

    }
    //return NULL;
}

/****************************************************************************
 * Name: handle_mode_dynamic
 *
 * Description:
 *    �����߳̽�����Ƶ�л�����
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void handle_mode_dynamic(void)
{
    int ret;
    pthread_t tid;
    ret = pthread_create(&tid, NULL, enter_dynamic, NULL);
    ret = pthread_detach(tid);
}
#endif


#if 0
#if (FREQ_SWITCH)
/****************************************************************************
 * Name: enter_dynamic
 *
 * Description:
 *    主频切换处理
 *
 * Parameters:
 *    arg 线程入参
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void enter_dynamic(int freq)
{
    /*当前系统主频*/
    static int cur_freq = 300000000;

    int fd, ret = 0;

    fd = open(CONFIG_PM_DEV_PATH, O_RDWR);
    if (fd < 0)
    {
        printf("%s:open /dev/pm error\n", __func__);
    }
    else
    {
        if (cur_freq == 300000000 && freq == 6000000)
        {
            cur_freq = 6000000;
            //断开usb
            usb_manual_disconnect();
            sysfs_usb_umount();

            //断开sd卡
            //sysfs_sd_umount();
            ret = ioctl(fd, PMIOC_DYNAMICFREQ, 6000000);
        }
        else if (cur_freq == 6000000 && freq == 300000000)
        {
            cur_freq = 300000000;
            ret = ioctl(fd, PMIOC_DYNAMICFREQ, 300000000);

            silan_usb_initialize();
        }

        /* update time throld to 60s */
        if (ret != 0)
        {
            printf("%s:Switch freq fail\n", __func__);

            pm_sync(STANDBY_DELAY_SEC);
        }

        close(fd);
    }
}

#endif
#endif

void save_usb_play_time(void)
{

    ui_info_t player_info;
    player_get_info(&player_info);
    if (SOURCE_SELECT_USB == ui_source_select)
    {
        usb_playtime = player_info.curtime;
    }
}

/****************************************************************************
 * Name: get_all_play_info
 *
 * Description:
 *    获取播放信息
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
ui_info_t sc8836_get_all_play_info(void)
{
    int ui_media = playerInfo.ui_media;
	player_get_info(&playerInfo);
    playerInfo.ui_media = ui_media;
	return playerInfo;
}



/****************************************************************************
 * Name: set_ui_media
 *
 * Description:
 *    设置当前有UI媒介
 *
 * Parameters:
 *    source 源类型
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void set_ui_media(int source)
{

/*
改到opt模式时，插u盘再拔，会出现无声的问题。
*/
    //设置播放媒介信息
	switch(source)
	{
		case SOURCE_SELECT_BT:
			playerInfo.ui_media = MEDIA_BT;
			break;
		case SOURCE_SELECT_USB:
			playerInfo.ui_media = MEDIA_USB;
			break;
		case SOURCE_SELECT_SD:
			playerInfo.ui_media = MEDIA_SD;
			break;
		case SOURCE_SELECT_LINEIN:
			playerInfo.ui_media = MEDIA_LINEIN;
			break;
		case SOURCE_SELECT_SPDIFIN:
			playerInfo.ui_media = MEDIA_SPDIF;
			break;
		case SOURCE_SELECT_HDMI:
			playerInfo.ui_media = MEDIA_HDMI;
			break;
		case SOURCE_SELECT_FM:
			playerInfo.ui_media = MEDIA_FM;
			break;
	}
}





/****************************************************************************
 * Name: ui_handle_eq
 *
 * Description:
 *    eq处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_eq(int num)
{
#if 0
    const int size = sizeof(eq_parameter_table)/sizeof(eq_parameter_table[0]);
    const int eq_num = sizeof(eq_parameter_table[0])/sizeof(eq_parameter_table[0][0]);
    int i;

    if(num != eq_type)
    {
        if(num == -2)
        {
            num = eq_type;
            if(++num == size)
            {
                num = 0;
            }
        }

        if(num == -1)
        {
            eq_type = num;
            int para[7] = {0, 0, 0, 0, 0, 0, 6};
            //清除当前EQ的值
            for(i=0;i<eq_num;++i)
            {
                para[0] = i+1;
                swa_audio_audproc_eq(AUDPROC_LIST_MIX, (ae_eq_para *)para);
                swa_audio_audproc_set(AUDPROC_LIST_MIX, AUDPROC_EQ);
            }
            //卸载EQ的值
            swa_audio_audproc_unload(AUDPROC_LIST_MIX, AUDPROC_EQ);
        }
        else if(-1 < num && num < size)
        {
            eq_type = num;
            swa_audio_audproc_load(AUDPROC_LIST_MIX, AUDPROC_EQ);
            for(i=0;i<eq_num;++i)
            {
                swa_audio_audproc_eq(AUDPROC_LIST_MIX, (ae_eq_para *)eq_parameter_table[eq_type][i]);
                swa_audio_audproc_set(AUDPROC_LIST_MIX, AUDPROC_EQ);
            }
        }
    }
#endif
}


/****************************************************************************
 * Name: ui_handle_file_load
 *
 * Description:
 *    处理文件加载
 *
 * Parameters:
 *    total_num 总文件数
 *    folder_num 总文件夹数
 *    url 媒体路径
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_file_load(int total_num, int folder_num, char * url)
{
    if(ui_source_select == SOURCE_SELECT_USB && strcmp(SEARCH_USB_NAME, url) == 0)
    {
        if(usb_last_total_num != total_num || usb_last_folder_num != folder_num)
        {
            usb_last_file_index = 0;
            usb_playtime = 0;
        }
        usb_last_total_num = total_num;
        usb_last_folder_num = folder_num;
        handle_local_music_play(usb_last_file_index, usb_playtime);
    }
    else if(ui_source_select == SOURCE_SELECT_SD && strcmp(SEARCH_SD_NAME, url) == 0)
    {
        if(sd_last_total_num != total_num || sd_last_folder_num != folder_num)
        {
            sd_last_file_index = 0;
            sd_playtime = 0;
        }
        sd_last_total_num = total_num;
        sd_last_folder_num = folder_num;
        handle_local_music_play(sd_last_file_index, sd_playtime);
    }
}





/****************************************************************************
 * Name: ui_handle_load_sd
 *
 * Description:
 *    SD文件列表加载事件处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_load_sd(void)
{
    if(ui_source_select == SOURCE_SELECT_SD)
    {   //sd模式
        //加载USB文件列表
        handle_local(SEARCH_SD_NAME);
    }
    printf("%s:load\n", __func__);
}

/****************************************************************************
 * Name: ui_handle_load_usb
 *
 * Description:
 *    USB文件列表加载事件处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_load_usb(void)
{
    if(ui_source_select == SOURCE_SELECT_USB)
    {   //usb模式
        //加载USB文件列表
        handle_local(SEARCH_USB_NAME);
    }
    printf("%s:load\n", __func__);
}





/****************************************************************************
 * Name: ui_handle_next
 *
 * Description:
 *    下一曲处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_play_num(int num)
{

   if (ui_source_select == SOURCE_SELECT_USB ||
             ui_source_select == SOURCE_SELECT_SD)
    {
		if (mute_state == MUTE)
	    {
	        ui_handle_mute();
	    }

        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
        int *p_playtime = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
        int total = get_file_total();
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
		pa_mute_ctrl(true);
        if( (total > 0)&&(num<=total))
        {
			*p_index = num-1;
			*p_playtime = 0;
			handle_local_music_play(*p_index, *p_playtime);
        }
		else if(num>total)
		{
			*p_index = total-1;
			*p_playtime = 0;
			handle_local_music_play(*p_index, *p_playtime);
		}
		usleep(500000);
		usleep(500000);
		usleep(500000);
		set_channel_mixvol_by_mode(ui_source_select);
		//player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
		pa_mute_ctrl(false);
    }
    printf("%s.\n", __func__);
}



/****************************************************************************
 * Name: ui_handle_pause_play
 *
 * Description:
 *    暂停播放处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_pause_play(char mode,char status)
{
    printf("\n%s\n", __func__);

 	if(ui_source_select == SOURCE_SELECT_USB ||
    	ui_source_select == SOURCE_SELECT_SD)
    {
        //获取播放状态
        ui_info_t player_info;
        memset(&player_info, 0, sizeof(ui_info_t));
        player_get_info(&player_info);
		if(mode == 0)//key
		{
			if (player_info.player_stat == 2)
	        {   //正在播放
	            //发送命令停止播放
	            player_process_cmd(NP_CMD_PAUSE, NULL, 0, NULL, NULL);
				bt_cmd_usb_playstatus(0);
	        }
	        else if (player_info.player_stat == 3)
	        {   //暂停播放
	            //发送命令恢复播放
	            player_process_cmd(NP_CMD_RESUME, NULL, 0, NULL, NULL);
				bt_cmd_usb_playstatus(1);
	        }
		}
		else if(mode == 1)//app
		{
			if (player_info.player_stat > 1)
			{
				switch(status)
				{
					case 0://pause
						player_process_cmd(NP_CMD_PAUSE, NULL, 0, NULL, NULL);
						break;

					case 1://play
						player_process_cmd(NP_CMD_RESUME, NULL, 0, NULL, NULL);
						break;
				}
			}
		}
		else if(mode == 2)//get usb play status
		{
			if (player_info.player_stat == 2)
	        {   
				//正在播放
	            bt_cmd_usb_playstatus(1);
	        }
	        else if (player_info.player_stat == 3)
	        {   
				//暂停播放
	            bt_cmd_usb_playstatus(0);
	        }
		}
       
    }
}

void search_current_music_folder(void)
{
	int i;
	
	if (ui_source_select == SOURCE_SELECT_USB ||
             ui_source_select == SOURCE_SELECT_SD)
    {
        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
		for (i=0; i<folder_total_num; ++i)
		{
			if((*p_index >= folder_index_tab[i][0])&&(*p_index <= folder_index_tab[i][1]))
			{
				folder_index_cnt = i;
				break;
			}
		}
	}
}


/****************************************************************************
 * Name: ui_handle_next
 *
 * Description:
 *    下一曲处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_next(void)
{
 if (ui_source_select == SOURCE_SELECT_USB ||
             ui_source_select == SOURCE_SELECT_SD)
    {
        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
        int *p_playtime = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
        int total = get_file_total();
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
		pa_mute_ctrl(true);
		
        if (total > 0)
        {
            if(++(*p_index) >= total)
            {
                *p_index = 0;
            }
            *p_playtime = 0;
            handle_local_music_play(*p_index, *p_playtime);
        }
		search_current_music_folder();
		usleep(500000);
		usleep(500000);
		usleep(500000);
		if (mute_state == UNMUTE)
		{
			set_channel_mixvol_by_mode(ui_source_select);
			//player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
			pa_mute_ctrl(false);
		}

    }

    printf("%s\n", __func__);
}






/****************************************************************************
 * Name: ui_handle_prev
 *
 * Description:
 *    上一曲处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_prev(void)
{
 if (ui_source_select == SOURCE_SELECT_USB ||
             ui_source_select == SOURCE_SELECT_SD)
    {
		//USB或SD卡模式
        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
        int *p_playtime = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
        int total = get_file_total();
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
		pa_mute_ctrl(true);
        if (total > 0)
        {
            if(--(*p_index) < 0)
            {
                *p_index = total-1;
            }
            *p_playtime = 0;
            handle_local_music_play(*p_index, *p_playtime);
        }
		search_current_music_folder();
		usleep(500000);
		usleep(500000);
		usleep(500000);
		if (mute_state == UNMUTE)
		{
			set_channel_mixvol_by_mode(ui_source_select);
			//player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
			pa_mute_ctrl(false);
		}
    }

    printf("%s\n", __func__);
}

/****************************************************************************
 * Name: ui_handle_tone_finish
 *
 * Description:
 *    切换下一个文件夹播放处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_folder_next(void)
{
   if (ui_source_select == SOURCE_SELECT_USB ||
             ui_source_select == SOURCE_SELECT_SD)
    {
		//USB或SD卡模式
        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
        int *p_playtime = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
        int total = get_file_total();
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
		pa_mute_ctrl(true);
        //if (total > 0)
        {
			if(folder_index_cnt < folder_total_num-1)
			{
				folder_index_cnt++;
			}
			else
			{
				folder_index_cnt = 0;
			}
			*p_index = folder_index_tab[folder_index_cnt][0];
            *p_playtime = 0;
			printf(">>>>>>>>>>p_index:%d--- p_playtime:%d---  folder_index_tab[%d][0]:%d \n",
				*p_index, *p_playtime, folder_index_cnt, folder_index_tab[folder_index_cnt][0]);
			//next_folder_flag = false;
			folder_index_dis = *p_index;
			folder_dis_flag = true;
			//display_ui_usb_folder(0);
			handle_local_music_play(*p_index, *p_playtime);
        }
		usleep(500000);
		usleep(500000);
		usleep(500000);
		if (mute_state == UNMUTE)
		{
			set_channel_mixvol_by_mode(ui_source_select);
			//player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
			pa_mute_ctrl(false);
		}
		display_ui_usb_folder(1);
		usleep(500000);
		folder_dis_flag = false;
    }

    printf("%s\n", __func__);
}

/****************************************************************************
 * Name: ui_handle_tone_finish
 *
 * Description:
 *    切换上一个文件夹播放处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_folder_prev(void)
{
   if (ui_source_select == SOURCE_SELECT_USB ||
             ui_source_select == SOURCE_SELECT_SD)
    {
		//USB或SD卡模式
        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
        int *p_playtime = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
        int total = get_file_total();
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
		pa_mute_ctrl(true);
        //if (total > 0)
        {
			if(folder_index_cnt > 0)
			{
				folder_index_cnt--;
			}
			else
			{
				folder_index_cnt = folder_total_num-1;
			}
			*p_index = folder_index_tab[folder_index_cnt][0];
            *p_playtime = 0;
			printf(">>>>>>>>>>p_index:%d--- p_playtime:%d---  f0lder_index_tab[%d][0]:%d \n",
				*p_index, *p_playtime, folder_index_cnt, folder_index_tab[folder_index_cnt][0]);
			//prev_folder_flag = false;
			folder_index_dis = *p_index;
			folder_dis_flag = true;
			//display_ui_usb_folder(0);
			handle_local_music_play(*p_index, *p_playtime);
        }
		usleep(500000);
		usleep(500000);
		usleep(500000);
		if (mute_state == UNMUTE)
		{
			set_channel_mixvol_by_mode(ui_source_select);
			//player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
			pa_mute_ctrl(false);
		}
		display_ui_usb_folder(1);
		usleep(500000);
		folder_dis_flag = false;
    }

    printf("%s\n", __func__);
}

/****************************************************************************
 * Name: ui_handle_tone
 *
 * Description:
 *    播放指定的tone
 *
 * Parameters:
 *    tone_num tone的序号
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_tone(int tone_num)
{
    if(TONE_START < tone_num && tone_num < TONE_MAX)
    {
        pa_mute_ctrl(false);
        em_player_tone(tone_num);
    }
}

/****************************************************************************
 * Name: ui_handle_tone_finish
 *
 * Description:
 *    播放播放结束处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_tone_finish(void)
{
    if (ui_source_select == SOURCE_SELECT_USB || ui_source_select == SOURCE_SELECT_SD)
    {   //USB或SD卡模式
        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
        int *p_playtime = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
        int total = get_file_total();
        if (total > 0)
        {
            handle_local_music_play(*p_index, *p_playtime);
        }
    }
    else if(ui_source_select == SOURCE_SELECT_BT || ui_source_select == SOURCE_SELECT_LINEIN)
    {
        if(bt_mute_last_state == 0)
        {
            pa_mute_ctrl(true);
        }
    }
}

/****************************************************************************
 * Name: ui_handle_sd_unload
 *
 * Description:
 *    SD卡卸载处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_sd_unload(void)
{
    if(ui_source_select == SOURCE_SELECT_SD)
    {
        player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);
        usleep(100);

        //通知USB拔出
        struct input_event ui_event;
        memset(&ui_event,0,sizeof(struct input_event));
        ui_event.type = EV_UI;
        ui_event.value = 0;
        ui_event.code = CODE_UI_UPDATE_DONE;
        input_add_event(&ui_event);
        display_set_source(ui_source_select);
    }

    if(ui_source_select != SOURCE_SELECT_USB)
    {
        //复原播放列表
        reset_playlist();
    }
}



/****************************************************************************
 * Name: ui_handle_usb_out
 *
 * Description:
 *    USB拔出事件处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_usb_out(void)
{
    if(ui_source_select == SOURCE_SELECT_USB)
    {
		player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);
		usleep(100);

		//通知USB拔出
		struct input_event ui_event;
		memset(&ui_event,0,sizeof(struct input_event));
		ui_event.type = EV_UI;
		ui_event.value = 0;
		ui_event.code = CODE_UI_UPDATE_DONE;
		input_add_event(&ui_event);
		display_set_source(ui_source_select);
    }

    if(ui_source_select != SOURCE_SELECT_SD)
    {
		//复原播放列表
		reset_playlist();
    }
}



/****************************************************************************
 * Name: ui_handle_mute
 *
 * Description:
 *    静音处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
extern int mic_vol;
void ui_handle_mute(void)
{
    printf("\n%s\n", __func__);
    if (mute_state == UNMUTE)
    {
		#if 0
		set_adc_channel_vol(0,0);
		set_adc_channel_vol(1,0);
		set_adc_channel_vol(2,0);
		set_adc_channel_vol(3,0);
		#endif
        player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
        mute_state = MUTE;
        pa_mute_ctrl(true);
		if (ui_source_select == SOURCE_SELECT_HDMI)
		{
			sc8836_action_hdmi_soundbar_mute_tv();
			usleep(100000);
			sc8836_action_hdmi_soundbar_adj_tv_vol();
		}
		
		if (ui_source_select == SOURCE_SELECT_USB)
		{
			save_usb_play_time();
		}

		take_micmute_flag = true;
		mic_open(false);

        #if BYPASS_MODE
        if(ui_source_select != SOURCE_SELECT_SPDIFIN)
        {
            silan_set_linebypass_disable();
        }
        #endif

    }
    else
    {
		#if 0
		set_adc_channel_vol(0,mic_vol);
		if (ui_source_select == SOURCE_SELECT_SPDIFIN ||
		ui_source_select == SOURCE_SELECT_USB ||
		ui_source_select == SOURCE_SELECT_SD ||
		ui_source_select == SOURCE_SELECT_HDMI ||
		ui_source_select == SOURCE_SELECT_COA)
		{
			set_adc_channel_vol(3,(int)mix_vol);
		}
		else if (ui_source_select == SOURCE_SELECT_BT)
		{
			set_adc_channel_vol(2,(int)mix_vol);
		}
		else if (ui_source_select == SOURCE_SELECT_FM ||
			ui_source_select == SOURCE_SELECT_LINEIN)
		{
			set_adc_channel_vol(1,(int)mix_vol);
		}
		#endif
		set_channel_mixvol_by_mode(ui_source_select);
        //player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
        mute_state = UNMUTE;
       	pa_mute_ctrl(false);
		if (ui_source_select == SOURCE_SELECT_HDMI)
		{
			sc8836_action_hdmi_soundbar_unmute_tv();
			usleep(100000);
			sc8836_action_hdmi_soundbar_adj_tv_vol();
		}
		if (ui_source_select == SOURCE_SELECT_USB)
		{
			save_usb_play_time();
		}

		if(take_micmute_flag ==        true)
		{
			take_micmute_flag = false;
			if(mic_on_flag)
			{
				if(mic_detect_online)
				{
					mic_open(true);
				}
				else
				{
					mic_open(false);
				}
			}
			else
			{
				mic_open(false);
			}
		}
		
		
        #if BYPASS_MODE
        if(ui_source_select != SOURCE_SELECT_SPDIFIN && bypass_in_vol > BYPASS_VOL_IN_MIN)
        {
            silan_set_linebypass_enable();
        }
        #endif

    }

 	display_set_source(ui_source_select);

}





/****************************************************************************
 * Name: ui_handle_up
 *
 * Description:
 *    红外音量加事件处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_up(void)
{
    if (mute_state == MUTE)
    {
        ui_handle_mute();
    }

    //音量增加处理
    ui_process_vol_inc();

#ifdef SL_UI_DBG
    printf("%s, volume_up\n", __func__);
#endif
}



/****************************************************************************
 * Name: ui_handle_down
 *
 * Description:
 *    红外音量减事件处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_down(void)
{
    if (mute_state == MUTE)
    {
        ui_handle_mute();
    }

    ui_process_vol_dec();

#ifdef SL_UI_DBG
    printf("%s, volume down\n", __func__);
#endif
}



/****************************************************************************
 * Name: ui_handle_vol_set
 *
 * Description:
 *    音量设置处理
 *
 * Parameters:
 *    vol 设置的音量（0-100）
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_set(int vol)
{
    //if(vol >= 0 && vol <= (84.0 / VOL_STEP))

    if(vol >= 0 && vol <= 30)
    {
		printf("set vol %d\n",vol);
		bt_mix_vol = vol;
        //mix_vol = (float)vol * VOL_STEP;
        select_mixvol_table();

		if (ui_source_select == SOURCE_SELECT_SPDIFIN ||
		ui_source_select == SOURCE_SELECT_USB ||
		ui_source_select == SOURCE_SELECT_SD ||
		ui_source_select == SOURCE_SELECT_HDMI ||
		ui_source_select == SOURCE_SELECT_COA)
		{
			set_adc_channel_vol(3,(int)mix_vol);
		}
		else if (ui_source_select == SOURCE_SELECT_BT)
		{
			set_adc_channel_vol(2,(int)mix_vol);
		}
		else if (ui_source_select == SOURCE_SELECT_FM ||
			ui_source_select == SOURCE_SELECT_LINEIN)
		{
			set_adc_channel_vol(1,(int)mix_vol);
		}
        //player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
        //显示音量信息
		display_ui_vol(vol);
		save_mix_vol();
    }
}

/****************************************************************************
 * Name: ui_handle_vol_dec_long_press
 *
 * Description:
 *    ?????????????????????
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_dec_long_press(void)
{
    if (NULL == wdtimer_vol_longpress)
    { //??????
        wdtimer_vol_longpress = wd_create();
    }
    else
    { //???????????
        wd_cancel(wdtimer_vol_longpress);
    }

    //???????????????
    ui_cmd_t cmd;
    cmd.cmd = UI_CMD_VOLUME_DEC;
    send_cmd_2_ui(&cmd);

    if (NULL != wdtimer_vol_longpress)
    { //??��??????????
        //????????????????
        wd_start(wdtimer_vol_longpress, CLOCKS_PER_SEC / 4, (wdentry_t)ui_handle_vol_dec_long_press, 0);
    }
}

/****************************************************************************
 * Name: ui_handle_vol_inc_long_press
 *
 * Description:
 *    ???????????????????????
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_inc_long_press(void)
{
    if (NULL == wdtimer_vol_longpress)
    { //??????
        wdtimer_vol_longpress = wd_create();
    }
    else
    { //???????????
        wd_cancel(wdtimer_vol_longpress);
    }

    //???????????????
    ui_cmd_t cmd;
    cmd.cmd = UI_CMD_VOLUME_INC;
    send_cmd_2_ui(&cmd);

    if (NULL != wdtimer_vol_longpress)
    { //??��??????????
        //????????????????
        wd_start(wdtimer_vol_longpress, CLOCKS_PER_SEC / 4, (wdentry_t)ui_handle_vol_inc_long_press, 0);
    }
}

/****************************************************************************
 * Name: ui_handle_vol_long_press_up
 *
 * Description:
 *    ?????????????????????????
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_long_press_up(void)
{
    if (NULL != wdtimer_vol_longpress)
    { //???????????
        wd_cancel(wdtimer_vol_longpress);
    }
}


/****************************************************************************
 * Name: ui_process_vol_dec
 *
 * Description:
 *    音量减处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void ui_process_vol_dec(void)
{
	int vol_temp = 0;
    //mix_vol -= VOL_STEP;
    //if(mix_vol <= 0)
    //{
        //mix_vol = 0;
    //}
    if(enter_tre_set == true)
    {
		tre_bass_cnt = 0;
		treble_vol--;
		if(treble_vol <= BASS_TREBLE_LEVEL_MIN)
		{
			treble_vol = BASS_TREBLE_LEVEL_MIN;
		}
		printf("UI_CMD_EQ_TRB_SUB:%d\n",treble_vol);
		set_bass_treble_vol(TREBLE_MODE,treble_vol,1);
		save_trebass_level(TREBLE_MODE);
		bt_cmd_current_treble(treble_vol); //treble
		//display_ui_bass_vol(2,treble_vol);
	}
	else  if(enter_bass_set == true)
    {
		tre_bass_cnt = 0;
		bass_vol--;
		if(bass_vol <= BASS_TREBLE_LEVEL_MIN)
		{
			bass_vol = BASS_TREBLE_LEVEL_MIN;
		}
		printf("UI_CMD_EQ_BASS_SUB:%d\n",bass_vol);
		set_bass_treble_vol(BASS_MODE,bass_vol,1);
		save_trebass_level(BASS_MODE);
		bt_cmd_current_bass(bass_vol); //bass
		//display_ui_bass_vol(0,bass_vol);
	}
	else
	{
		bt_mix_vol -= 1;

		if(bt_mix_vol <=0)
		{
			bt_mix_vol = 0;
			take_micmute_flag = true;
			mic_open(false);
		}
			

		select_mixvol_table();

		bt_cmd_current_mainvol();
#if 1
		if (ui_source_select == SOURCE_SELECT_SPDIFIN ||
			ui_source_select == SOURCE_SELECT_USB ||
			ui_source_select == SOURCE_SELECT_SD ||
			ui_source_select == SOURCE_SELECT_HDMI ||
			ui_source_select == SOURCE_SELECT_COA)
		{
			set_adc_channel_vol(3,(int)mix_vol);
		}
		else if (ui_source_select == SOURCE_SELECT_BT)
		{
			set_adc_channel_vol(2,(int)mix_vol);
		}
		else if (ui_source_select == SOURCE_SELECT_FM ||
			ui_source_select == SOURCE_SELECT_LINEIN)
		{
			set_adc_channel_vol(1,(int)mix_vol);
		}
#else
		/*直接调channel音量，会导致功率过早到达额定功率，所以固定channel音量，调节总音量*/
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
#endif

		if(ui_source_select == SOURCE_SELECT_HDMI)
		{
			sc8836_action_hdmi_soundbar_adj_tv_vol();
		}
		
		printf("%s:mix_vol = %d\n", __func__, mix_vol);


		display_ui_vol(bt_mix_vol);

		save_mix_vol();

	    #if BYPASS_MODE
	    bypass_in_vol -= BYPASS_VOL_IN_STEP;
	    if(bypass_in_vol <= BYPASS_VOL_IN_MIN || mix_vol < VOL_STEP)
	    {
	        bypass_in_vol = BYPASS_VOL_IN_MIN;
	        if(ui_source_select != SOURCE_SELECT_SPDIFIN)
	        {
	            silan_set_linebypass_disable();
	        }
	    }
	    else
	    {
	        if(ui_source_select != SOURCE_SELECT_SPDIFIN)
	        {
	            float vol = bypass_in_vol + bypass_vol_offset;
	            silan_set_hardware_gilr_volume(vol, vol);
	        }
	    }
	    printf("%s:bypass_in_vol=%d\n", __func__, bypass_in_vol);
	    #endif
	}

	if (ui_source_select == SOURCE_SELECT_USB)
	{
		save_usb_play_time();
	}

}

/****************************************************************************
 * Name: ui_process_vol_inc
 *
 * Description:
 *    音量增加处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void ui_process_vol_inc(void)
{
	int vol_temp = 0;

	if(enter_tre_set == true)
    {
		tre_bass_cnt = 0;

		treble_vol++;
		if(treble_vol >= BASS_TREBLE_LEVEL_MAX)
		{
			treble_vol = BASS_TREBLE_LEVEL_MAX;
		}
		printf("UI_CMD_EQ_TRB_ADD:%d\n",treble_vol);
		set_bass_treble_vol(TREBLE_MODE,treble_vol,1);
		save_trebass_level(TREBLE_MODE);
		bt_cmd_current_treble(treble_vol); //treble
		//display_ui_bass_vol(2,treble_vol);
	}
	else  if(enter_bass_set == true)
    {
		tre_bass_cnt = 0;

		bass_vol++;
		if(bass_vol >= BASS_TREBLE_LEVEL_MAX)
		{
			bass_vol = BASS_TREBLE_LEVEL_MAX;
		}
		printf("UI_CMD_EQ_BASS_ADD:%d\n",bass_vol);
		set_bass_treble_vol(BASS_MODE,bass_vol,1);
		save_trebass_level(BASS_MODE);
		bt_cmd_current_bass(bass_vol); //bass
		//display_ui_bass_vol(0,bass_vol);
	}
	else
	{
		//mix_vol += VOL_STEP;
	    //if(mix_vol >= 100)
	    //{
	    //    mix_vol = 100;
	    //}

		//bt_mix_vol += 1;
		//if(bt_mix_vol >=60)
		//	bt_mix_vol = 60;
		bt_mix_vol += 1;

		if(bt_mix_vol >= MIX_LEV_CNT)
			bt_mix_vol = MIX_LEV_CNT;

		select_mixvol_table();

		bt_cmd_current_mainvol();
#if 1

		if (ui_source_select == SOURCE_SELECT_SPDIFIN ||
			ui_source_select == SOURCE_SELECT_USB ||
			ui_source_select == SOURCE_SELECT_SD ||
			ui_source_select == SOURCE_SELECT_HDMI ||
			ui_source_select == SOURCE_SELECT_COA)
		{
			set_adc_channel_vol(3,mix_vol);
		}
		else if (ui_source_select == SOURCE_SELECT_BT)
		{
			set_adc_channel_vol(2,mix_vol);
		}
		else if (ui_source_select == SOURCE_SELECT_FM ||
			ui_source_select == SOURCE_SELECT_LINEIN)
		{
			set_adc_channel_vol(1,mix_vol);
		}
#else
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
#endif

		if(take_micmute_flag ==        true)
		{
			take_micmute_flag = false;
			if(mic_on_flag)
			{
				if(mic_detect_online)
				{
					mic_open(true);
				}
				else
				{
					mic_open(false);
				}
			}
			else
			{
				mic_open(false);
			}
		}
		
		if(ui_source_select == SOURCE_SELECT_HDMI)
		{
			sc8836_action_hdmi_soundbar_adj_tv_vol();
		}

		printf("%s:mix_vol = %d\n", __func__, mix_vol);

		display_ui_vol(bt_mix_vol);

		save_mix_vol();

	    #if BYPASS_MODE
	    if(bypass_in_vol == BYPASS_VOL_IN_MIN)
	    {
	        if(ui_source_select != SOURCE_SELECT_SPDIFIN)
	        {
	            silan_set_linebypass_enable();
	        }
	    }

	    bypass_in_vol += BYPASS_VOL_IN_STEP;
	    if(bypass_in_vol > BYPASS_VOL_IN_MAX || mix_vol > 100)
	    {
	        bypass_in_vol = BYPASS_VOL_IN_MAX;
	        if(ui_source_select != SOURCE_SELECT_SPDIFIN)
	        {
	            float vol = bypass_in_vol + bypass_vol_offset;
	            silan_set_hardware_gilr_volume(vol, vol);
	        }
	    }
	    else
	    {
	        if(ui_source_select != SOURCE_SELECT_SPDIFIN)
	        {
	            float vol = bypass_in_vol + bypass_vol_offset;
	            silan_set_hardware_gilr_volume(vol, vol);
	        }
	    }
	    printf("%s:bypass_in_vol=%d\n", __func__, bypass_in_vol);
	    #endif
	}

	if (ui_source_select == SOURCE_SELECT_USB)
	{
		save_usb_play_time();
	}

}

void set_adc_channel_vol(int ch, int vol)
{
    int val[6]={0}; /* 6 channels */

	printf("%s vol:%d \n", __func__, vol);
    if (vol > 100)
	{
        vol = 100;
    }
	else if (vol < 0)
	{
        vol = 0;
	}

    val[0] = vol;
	val[1] = vol;
	val[2] = vol;
	val[3] = vol;
	val[4] = vol;
	val[5] = vol;

	switch(ch)
	{
		case 0://adc 0
				swa_audio_audproc_load(AUDPROC_LIST_ADC0, AUDPROC_VOLUMECTL);
				swa_audio_audproc_vol(AUDPROC_LIST_ADC0, 0, val);
				swa_audio_audproc_set(AUDPROC_LIST_ADC0, AUDPROC_VOLUMECTL);
			break;
		case 1://adc 1
				swa_audio_audproc_load(AUDPROC_LIST_ADC1, AUDPROC_VOLUMECTL);
				swa_audio_audproc_vol(AUDPROC_LIST_ADC1, 0, val);
				swa_audio_audproc_set(AUDPROC_LIST_ADC1, AUDPROC_VOLUMECTL);
			break;
		case 2://adc 2
				swa_audio_audproc_load(AUDPROC_LIST_ADC2, AUDPROC_VOLUMECTL);
				swa_audio_audproc_vol(AUDPROC_LIST_ADC2, 0, val);
				swa_audio_audproc_set(AUDPROC_LIST_ADC2, AUDPROC_VOLUMECTL);
			break;
		case 3://decoder
				swa_audio_audproc_load(AUDPROC_LIST_DEC, AUDPROC_VOLUMECTL);
				swa_audio_audproc_vol(AUDPROC_LIST_DEC, 0, val);
				swa_audio_audproc_set(AUDPROC_LIST_DEC, AUDPROC_VOLUMECTL);
			break;
		default:
			break;
	}

}

void sl_ui_set_reqrate(void)
{
	player_process_cmd(NP_CMD_SET_REQRATE, NULL, 44100, NULL, NULL);
	//player_process_cmd(NP_CMD_SET_REQRATE, NULL, 48000, NULL, NULL);
	usleep(1000);
}

void sl_ui_system_reset(void)
{
	char reset_clear_str[] = {NUM_OFF,NUM_OFF, NUM_OFF, NUM_OFF, NUM_OFF};

	display_ui_clear();
	ht1633_updata_display();
	//usleep(500000);
	pa_mute_ctrl(true);
	mute_state = UNMUTE;
	bt_mix_vol = Frist_MIX_LEV;
	select_mixvol_table();
	bt_cmd_current_mainvol();
	bass_vol = 0;
	treble_vol = 0;
	echo_vol_lev = 5;
	mic_vol_lev = 15;

	fm_clear();

	save_mix_vol();

	player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);

	usleep(10000);
	set_channel_vol_by_mode(ui_source_select);

	set_bass_treble_vol(BASS_MODE,bass_vol,0);
	save_trebass_level(BASS_MODE);
	bt_cmd_current_bass(bass_vol); //bass
	usleep(10000);
	set_bass_treble_vol(TREBLE_MODE,treble_vol,0);
	save_trebass_level(TREBLE_MODE);
	bt_cmd_current_treble(treble_vol); //treble
	//display_ui_clear();
	//display_str(reset_clear_str);
	//ht1633_updata_display();
	usleep(10000);
	pa_mute_ctrl(false);
	usleep(500000);
	usleep(500000);
	set_channel_mixvol_by_mode(ui_source_select);
	//player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
	//display_set_source(ui_source_select);
}

/*****************************************************




*****************************************************/




#ifdef CONFIG_CEC

int sc8836_action_hdmion_send(void)
{
	cec_process_cmd(CEC_CMD_ARCON, NULL);
	usleep(1000);
}

int sc8836_action_hdmi_on( void)
{
#if 1
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

	int cec_cmd_gpio = SL_HDMI_CEC_PIN;  //GPIO1_  change

	printf("cec_cmd_gpio=%d\r\n",cec_cmd_gpio);
	cec_process_cmd(CEC_CMD_ARC_INIT,&cec_cmd_gpio);
	usleep(1000);
	cec_process_cmd(CEC_CMD_ARCON, NULL);
	usleep(1000);
	//player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
	return SL_UI_ERROR_NULL;
#else

	int cec_cmd_gpio = SL_HDMI_CEC_PIN;
	printf("%s %d\n", __func__, __LINE__);

    cec_process_cmd(CEC_CMD_ARC_INIT, &cec_cmd_gpio);
	usleep(1000);
	cec_process_cmd(CEC_CMD_ARCON, NULL);
	usleep(1000);
	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
	usleep(1000);
	player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
	usleep(1000);

	return SL_UI_ERROR_NULL;

#endif
}

int sc8836_action_hdmi_off(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

	cec_process_cmd(CEC_CMD_ARCOFF, NULL);
	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);

	return SL_UI_ERROR_NULL;
}
int sc8836_action_hdmi_standby(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

	cec_process_cmd(CEC_CMD_STANDBY, NULL);
	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
	return SL_UI_ERROR_NULL;
}
int sc8836_action_hdmi_poweron(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

	cec_process_cmd(CEC_CMD_POWERON, NULL);
	//cec_process_cmd(CEC_CMD_ARCON, NULL);

	return SL_UI_ERROR_NULL;
}



int sc8836_ui_handle_cec_inactive_source( void)
{
#if 1//def SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
	//set_sl_ui_cmd(NP_CMD_STOP, NULL, 0);
	sc8836_action_hdmi_off();
	sys_power_control();
	return 	-SL_UI_ERROR_NO_DIALOG;
}

int sc8836_ui_handle_cec_active_source( void)
{
#if 1//def SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
	sc8836_action_hdmi_on();
	//set_sl_ui_cmd(NP_CMD_STOP, NULL, 0);
	//sys_power_control();
	return 	-SL_UI_ERROR_NO_DIALOG;
}

int sc8836_ui_handle_cec_volume_key_up(struct ui_cmd_s *cmd)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
	//set_sl_ui_cmd(NP_CMD_STOP, NULL, 0);
	return 	-SL_UI_ERROR_NO_DIALOG;
}

int sc8836_ui_handle_cec_volume_key_down(struct ui_cmd_s *cmd)
{
#if 1//def SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
	//set_sl_ui_cmd(NP_CMD_STOP, NULL, 0);
	return 	-SL_UI_ERROR_NO_DIALOG;
}

int sc8836_ui_handle_cec_volume_key_release(struct ui_cmd_s *cmd)
{
#if 1//def SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
	//set_sl_ui_cmd(NP_CMD_STOP, NULL, 0);
	return 	-SL_UI_ERROR_NO_DIALOG;
}

int sc8836_ui_handle_cec_mute_key(struct ui_cmd_s *cmd)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

	return 	-SL_UI_ERROR_NO_DIALOG;
}


int sc8836_action_hdmi_soundbar_adj_tv_vol(void)
{
	int cur_volum;
	
	if(bt_mix_vol <= 20)
	{
		cur_volum = bt_mix_vol*3;
	}
	else
	{
		//cur_volum = 60+(bt_mix_vol - 20)*4;
		cur_volum = (4*bt_mix_vol)-20;
	}
	
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
    cec_process_cmd(CEC_CMD_SDB_ADJ_TV_VOL, &cur_volum);
}

int sc8836_action_hdmi_soundbar_mute_tv(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
    cec_process_cmd(CEC_CMD_SDB_MUTE_TV, NULL);
}

int sc8836_action_hdmi_soundbar_unmute_tv(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

    cec_process_cmd(CEC_CMD_SDB_UNMUTE_TV, NULL);
}
#endif



