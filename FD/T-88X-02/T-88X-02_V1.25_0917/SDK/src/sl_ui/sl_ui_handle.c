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

static int cur_freq = 300000000;

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
 WDOG_ID wdtimer_enter_testmode_detect = NULL;

 /*切换模式解MUTE看门狗*/
WDOG_ID wdtimer_change_mode_unmute = NULL;

/*延时显示模式字符串看门狗*/

/*长按物理音量键处理看门狗*/
static WDOG_ID wdtimer_vol_longpress = NULL;


 WDOG_ID wdtimer_goback_mode;

 WDOG_ID wdtimer_display_updata;

/*��ǰTREBLE����*/
static int treble_level = 0;
/*��ǰBASS����*/
static int bass_level = 0;

bool next_folder_flag = false;
bool prev_folder_flag = false;

/*全局*/
/*开关机状态*/
int ui_power = POWER_OFF;



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

/*内部*/
extern int mic_vol;
extern int echo_vol_lev ;
extern int mic_vol_lev;
extern int tre_bass_cnt;
extern bool enter_tre_set;
extern bool enter_bass_set;



static void ui_process_vol_dec(void);
static void ui_process_vol_inc(void);

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

        if (*pLevel <= 0)
        {
            para[3] = (*pLevel) * BASS_TREBLE_GAIN_STEP;
        }
        else
        {
            int i;
            int gain_max = BASS_TREBLE_GAIN_MAX;
			for(i=0;i<bass_treble_gains_size;++i)
            {
                if (mix_vol <= bass_treble_gains[i][0])
                {
                    gain_max = bass_treble_gains[i][1];
                    break;
                }
            }
            para[3] = (*pLevel) * (gain_max / 30);
        }

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
        para[4] = bass_or_treble?100:10000;
		para[5] = bass_or_treble?100:300;
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
		enter_dynamic(300000000);
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

		//开启蓝牙mute同步看门狗
		if(wdtimer_bt_mute_detect == NULL)
		{
			wdtimer_bt_mute_detect = wd_create();
		}

		if(wdtimer_pa_station_detect == NULL)
		{
			wdtimer_pa_station_detect = wd_create();
		}

		if(wdtimer_enter_testmode_detect == NULL)
		{
			wdtimer_enter_testmode_detect = wd_create();
		}

		if(wdtimer_change_mode_unmute == NULL)
		{
			wdtimer_change_mode_unmute = wd_create();
		}
		/////////////////////////////////////////////

		frist_hdmi_init = true;

		//////////////////////////////////////////////

		//  led7_open();
		pa_io_ret_set(true);
		pa_static_check();
		enter_testmode_check();
		change_mode_unmute();

		read_player_info();
		read_mix_vol();
		// ui_source_select = SOURCE_SELECT_START;
		ui_handle_mode(ui_source_select, false);

#if 0
		usleep(500000);
		action_hdmi_on();
		usleep(1000);
		action_hdmi_poweron();
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
		player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
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

		if(wdtimer_enter_testmode_detect != NULL)
		{
			wd_cancel(wdtimer_enter_testmode_detect );
		}

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
		enter_dynamic(6000000);
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
    char buf_recv[65] = {0};
    char * ptr = buf_recv;

    while (1)
    {
        if(ui_power == POWER_ON)
        {
            ret = bt_read(ptr, buf_szie);

            ui_cmd_t cmd = {0};
            cmd.cmd = UI_CMD_NULL;
            int value;
            int index = bt_chk_AT_cmd(buf_recv, &value);
            if(index > AT_START && index < AT_MAX)
            {
                int ui_cmd = bt_cmd_translate(index);
                if (index == AT_NEXT_MODE)
                {//下一个模式
                    cmd.arg2 = -2;
                    cmd.mode = true;
                }
                else if (index == AT_DEVICE_CONNECTED)
                {//设备连接
                	printf("BT conneted\n");
                    cmd.arg2 = true;
                }
                else if (index == AT_DEVICE_DISCONNECTED)
                {//设备断开连接
                	printf("BT disconneted\n");
                    cmd.arg2 = false;
                }
                else if (index == AT_AUX_CONNECTED)
                {
                    cmd.arg2 = true;
                    printf("AUX connected\n");
                }
                else if (index == AT_AUX_DISCONNECTED)
                {
                    cmd.arg2 = false;
                    printf("AUX disconnected\n");
                }
                else if(index == AT_BT)
                {//进入蓝牙模式
                    cmd.arg2 = SOURCE_SELECT_BT;
                    cmd.mode = true;
                }
                else if(index == AT_AUX)
                {//进入AUX模式
                    cmd.arg2 = SOURCE_SELECT_LINEIN;
                    cmd.mode = true;
                }
                else if(index >= AT_SET_MODE)
                {//模式设置
                    printf("cmd.arg2=%d\n", value);
                    cmd.arg2 = value;
                }

                if(ui_cmd != UI_CMD_NULL)
                {
                    cmd.cmd = ui_cmd;
                    send_cmd_2_ui(&cmd);
                }
            }

            if (ret > 0)
            {
                //printf("uart buffer: %s\n", buf_recv);

                ptr += ret;
                if(buf_recv - ptr + buf_szie < 16)
                {   //超出当前存储范围
                    ptr = buf_recv;
                    memset(buf_recv, 0, buf_szie);
                }
            }

            usleep(1000);
        }
        else
        {
            bt_uart_close();
            sem_wait(&bt_start_check_sem);
            printf("%s:Recover from semaphore\n", __func__);
            bt_uart_init();
            usleep(100000);
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

void bt_cmd_dis_connect(void)
{
	//printf("%s : %d\n", __func__, __LINE__);
	bt_set_connect_state(false);
	/*直接显示蓝牙图标闪烁，无需等接收到ATWC，因为接收有时不准*/
	handle_bt_cmd(AT_DISCONNECT, 0);
	usleep(200000);
}



/***********************************************************


************************************************************/
void bt_cmd_play_pause(void)
{
     handle_bt_cmd(AT_PLAY_OR_PAUSE, 0);
}

/***********************************************************


************************************************************/
void bt_cmd_next_song(void)
{
	handle_bt_cmd(AT_NEXT_TRACE, 0);
}

/***********************************************************


************************************************************/
void bt_cmd_prev_song(void)
{
	handle_bt_cmd(AT_PREV_TRACE, 0);
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
static pthread_addr_t enter_dynamic(pthread_addr_t arg)
{
    int fd, ret = 0;

    fd = open(CONFIG_PM_DEV_PATH, O_RDWR);
    if (fd < 0)
    {
        printf("%s:open /dev/pm error\n", __func__);
    }
    else
    {
        if (cur_freq == 300000000)
        {
            cur_freq = 6000000;
            ret = ioctl(fd, PMIOC_DYNAMICFREQ, 6000000);
            printf("%s:Switch to 6 MHz freq\n", __func__);
        }
        else if (cur_freq == 6000000)
        {
            cur_freq = 300000000;
            ret = ioctl(fd, PMIOC_DYNAMICFREQ, 300000000);
            printf("%s:Switch to 300 MHz freq\n", __func__);
        }

        /* update time throld to 60s */
        if (ret != 0)
        {
            printf("%s:Switch freq fail\n", __func__);
            pm_sync(STANDBY_DELAY_SEC);
        }

        close(fd);

    }
    return NULL;
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
ui_info_t get_all_play_info(void)
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
        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
        int *p_playtime = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
        int total = get_file_total();
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
		pa_mute_ctrl(true);
        if( (total > 0)&&(num<=total))
        {
			*p_index = num;
			*p_playtime = 0;
			handle_local_music_play(*p_index, *p_playtime);
        }
		usleep(500000);
		usleep(500000);
		usleep(500000);
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
		pa_mute_ctrl(false);
    }
    printf("%s", __func__);
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
void ui_handle_pause_play(void)
{
    printf("\n%s\n", __func__);

 if(ui_source_select == SOURCE_SELECT_USB ||
            ui_source_select == SOURCE_SELECT_SD)
    {
        //获取播放状态
        ui_info_t player_info;
        memset(&player_info, 0, sizeof(ui_info_t));
        player_get_info(&player_info);
        if (player_info.player_stat == 2)
        {   //正在播放
            //发送命令停止播放
            player_process_cmd(NP_CMD_PAUSE, NULL, 0, NULL, NULL);
        }
        else if (player_info.player_stat == 3)
        {   //暂停播放
            //发送命令恢复播放
            player_process_cmd(NP_CMD_RESUME, NULL, 0, NULL, NULL);
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
		usleep(500000);
		usleep(500000);
		usleep(500000);
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
		pa_mute_ctrl(false);
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
    {   //USB或SD卡模式
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
		usleep(500000);
		usleep(500000);
		usleep(500000);
		pa_mute_ctrl(false);
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
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
    {   //USB或SD卡模式
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
			handle_local_music_play(*p_index, *p_playtime);
        }
		usleep(500000);
		usleep(500000);
		usleep(500000);
		pa_mute_ctrl(false);
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
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
    {   //USB或SD卡模式
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
			handle_local_music_play(*p_index, *p_playtime);
        }
		usleep(500000);
		usleep(500000);
		usleep(500000);
		pa_mute_ctrl(false);
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
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
        player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
        mute_state = UNMUTE;
       	pa_mute_ctrl(false);
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
    if(vol >= 0 && vol <= 60)
    {
		printf("set vol %d\n",vol);
        //mix_vol = (float)vol * VOL_STEP;
        mix_vol = mix_vol_table[vol];

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
		if(treble_vol <= -15)
		{
			treble_vol = -15;
		}
		printf("UI_CMD_EQ_TRB_SUB:%d\n",treble_vol);
		set_bass_treble_vol(2,treble_vol);
		display_ui_bass_vol(2,treble_vol);
	}
	else  if(enter_bass_set == true)
    {
		tre_bass_cnt = 0;
		bass_vol--;
		if(bass_vol <= -15)
		{
			bass_vol = -15;
		}
		printf("UI_CMD_EQ_BASS_SUB:%d\n",bass_vol);
		set_bass_treble_vol(0,bass_vol);
		display_ui_bass_vol(0,bass_vol);
	}
	else
	{
		bt_mix_vol -= 1;

		if(bt_mix_vol <=0)
			bt_mix_vol = 0;

		mix_vol = mix_vol_table[bt_mix_vol];
#if 0
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

		if (ui_source_select == SOURCE_SELECT_USB)
		{
			save_usb_play_time();
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
		if(treble_vol >= 15)
		{
			treble_vol = 15;
		}
		printf("UI_CMD_EQ_TRB_ADD:%d\n",treble_vol);
		set_bass_treble_vol(2,treble_vol);
		display_ui_bass_vol(2,treble_vol);
	}
	else  if(enter_bass_set == true)
    {
		tre_bass_cnt = 0;

		bass_vol++;
		if(bass_vol >= 15)
		{
			bass_vol = 15;
		}
		printf("UI_CMD_EQ_BASS_ADD:%d\n",bass_vol);
		set_bass_treble_vol(0,bass_vol);
		display_ui_bass_vol(0,bass_vol);
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
		mix_vol = mix_vol_table[bt_mix_vol];
#if 0

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
		if (ui_source_select == SOURCE_SELECT_USB)
		{
			save_usb_play_time();
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

}

void set_adc_channel_vol(int ch, int vol)
{
    int val[6]={0}; /* 6 channels */
	char parm[64] = {0};
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
				//swa_audio_audproc_unload(AUDPROC_LIST_ADC0, AUDPROC_VOLUMECTL);
				swa_audio_audproc_load(AUDPROC_LIST_ADC0, AUDPROC_VOLUMECTL);
				swa_audio_audproc_vol(AUDPROC_LIST_ADC0, 0, val);
				swa_audio_audproc_set(AUDPROC_LIST_ADC0, AUDPROC_VOLUMECTL);
			break;
		case 1://adc 1
				//swa_audio_audproc_unload(AUDPROC_LIST_ADC0, AUDPROC_VOLUMECTL);
				swa_audio_audproc_load(AUDPROC_LIST_ADC1, AUDPROC_VOLUMECTL);
				swa_audio_audproc_vol(AUDPROC_LIST_ADC1, 0, val);
				swa_audio_audproc_set(AUDPROC_LIST_ADC1, AUDPROC_VOLUMECTL);
			break;
		case 2://adc 2
				//swa_audio_audproc_unload(AUDPROC_LIST_ADC0, AUDPROC_VOLUMECTL);
				swa_audio_audproc_load(AUDPROC_LIST_ADC2, AUDPROC_VOLUMECTL);
				swa_audio_audproc_vol(AUDPROC_LIST_ADC2, 0, val);
				swa_audio_audproc_set(AUDPROC_LIST_ADC2, AUDPROC_VOLUMECTL);
			break;
		case 3://decoder
				//swa_audio_audproc_unload(AUDPROC_LIST_DEC, AUDPROC_VOLUMECTL);
				swa_audio_audproc_load(AUDPROC_LIST_DEC, AUDPROC_VOLUMECTL);
				swa_audio_audproc_vol(AUDPROC_LIST_DEC, 0, val);
				swa_audio_audproc_set(AUDPROC_LIST_DEC, AUDPROC_VOLUMECTL);
			break;
		default:
			break;
	}
//	sprintf(parm, "0 0 %d %d %d %d %d %d", val[0], val[1], val[2], val[3], val[4], val[5]);
//	player_process_cmd(NP_CMD_POSTVOL, parm, 0, NULL, NULL);

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
	mix_vol = Frist_MIX_VOL;
	bass_vol = 0;
	treble_vol = 0;
	mic_vol = 27;
	echo_vol_lev = 15;
	mic_vol_lev = 15;

	#if 0
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
	player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);

	save_mix_vol();

	usleep(10000);
	set_bass_treble_vol(0,bass_vol);
	usleep(10000);
	set_bass_treble_vol(2,treble_vol);
	//display_ui_clear();
	//display_str(reset_clear_str);
	//ht1633_updata_display();
	usleep(500000);
	pa_mute_ctrl(false);
	usleep(500000);
	usleep(500000);
	player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
	display_set_source(ui_source_select);
}

/*****************************************************




*****************************************************/




#ifdef CONFIG_CEC

int action_hdmion_send(void)
{
	cec_process_cmd(CEC_CMD_ARCON, NULL);
	usleep(1000);
}

int action_hdmi_on( void)
{
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
}

int action_hdmi_off(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

	cec_process_cmd(CEC_CMD_ARCOFF, NULL);
	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);

	return SL_UI_ERROR_NULL;
}
int action_hdmi_standby(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

	cec_process_cmd(CEC_CMD_STANDBY, NULL);
	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
	return SL_UI_ERROR_NULL;
}
int action_hdmi_poweron(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

	cec_process_cmd(CEC_CMD_POWERON, NULL);
	//cec_process_cmd(CEC_CMD_ARCON, NULL);

	return SL_UI_ERROR_NULL;
}



int sl_ui_handle_cec_inactive_source( void)
{
#if 1//def SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
	//set_sl_ui_cmd(NP_CMD_STOP, NULL, 0);
	sys_power_control();
	return 	-SL_UI_ERROR_NO_DIALOG;
}

int sl_ui_handle_cec_active_source( void)
{
#if 1//def SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
	//set_sl_ui_cmd(NP_CMD_STOP, NULL, 0);
	//sys_power_control();
	return 	-SL_UI_ERROR_NO_DIALOG;
}
#endif



