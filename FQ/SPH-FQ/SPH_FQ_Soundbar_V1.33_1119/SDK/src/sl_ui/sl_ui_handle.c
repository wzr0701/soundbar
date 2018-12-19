/****************************************************************************
 * Included Files
 ****************************************************************************/
#ifdef CONFIG_BLUETOOTH
#include "bluetooth.h"
#endif
#include "bt_uart.h"
#include <errno.h>
#include <fcntl.h>
#include <nuttx/power/pm.h>
#include <nxplayer.h>
#include "player_cmd.h"
#include <pthread.h>
#include <silan_gpio.h>
#include <silan_addrspace.h>
#include <silan_resources.h>
#include <nuttx/audio/sladsp_ae.h>
#include "filelist.h"
#include "sl_lcd.h"
#include "sl_ui_cmd.h"
#include "sl_ui_handle.h"
#include "sl_ui_local.h"
#include <string.h>
#include <sys/ioctl.h>
#include "4052.h"
#include "sl_bsp.h"
#include "vol_ctrl.h"
#include <silan_cec.h>
#include "led_api.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*调试打印开关*/
#define SL_UI_DBG
/*休眠standby*/
#define STANDBY_DELAY_SEC 10

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

/****************************************************************************
 * Public Data
 ****************************************************************************/
/*外部*/
extern int ui_source_select;
extern int ui_source_ir_select;
/*内部*/
/*常量*/
/*显示数据数组*/
static const char aux_str[] = "aux";
static const char bt_str[] = "bluetooth";
static const char opti_str[] = "spdifin";
static const char power_off_str[] = "power off";
static const char power_on_str[] = "Silan";
static const char sd_str[] = "sd";
static const char usb_str[] = "usb";

/*变量*/
/*蓝牙连接状态*/
bool bt_connected = false;
bool bt_force_disconnect = false;
/*蓝牙播放状态*/
bool bt_play_status = false;
/*蓝牙mute脚状态*/
static int bt_mute_last_state = 0;
/*蓝牙开始检测状态信号量*/
static sem_t bt_start_check_sem;
/*蓝牙同步信号量*/
static sem_t bt_state_sem;
/*蓝牙状态获取线程ID*/
static pthread_t bt_state_tid = -1;
/*bypass模式音量*/
static float bypass_in_vol = -12;
/*bypass音量补偿*/
static float bypass_vol_offset = 0;
/*当前系统主频*/
static int cur_freq = 300000000;
/*音量模式标志*/
static bool is_volume_mode = false;
/*mix音量*/
static float mix_vol = 20; //
/*AUX Status*/
bool aux_in_status = false;
bool frist_hdmi_init = false;
//bool frist_hdmi_det = false;

bool min_ok_flag = false;
bool max_ok_flag = false;

int hdmi_poweroff_cnt = 2000;

/*设置音量*/
static unsigned char set_vol = 8;
/*静音状态标志*/
static int mute_state = 0;
static int stop_state = 0;
static int bt_pause_state = 0;
static int aux_pause_state = 0;
/*SD上一次播放的歌曲序号*/
static int sd_last_file_index = -1;
/*SD有音频文件的文件夹数量*/
static int sd_last_folder_num = -1;
/*SD总文件夹数量*/
static int sd_last_total_num = -1;
/*播放时间*/
static int sd_playtime = 0;
/*USB上一次播放的歌曲序号*/
static int usb_last_file_index = -1;
/*USB有音频文件的文件夹数量*/
static int usb_last_folder_num = -1;
/*USB总文件夹数量*/
static int usb_last_total_num = -1;
/*播放时间*/
static int usb_playtime = 0;
/*延时显示模式字符串看门狗*/
static WDOG_ID wdtimer_goback_mode;
/*长按物理音量键处理看门狗*/
static WDOG_ID wdtimer_vol_longpress = NULL;
/*检测软件版本看门狗*/
static WDOG_ID wdtimer_vesion;

WDOG_ID wdtimer_resetdsp_init = NULL;
WDOG_ID wdtimer_hdmion_send = NULL;


bool bt_connected_flag = false;
bool mute_state_flag = false;
bool frist_hdmi_det_online = false;


unsigned short ui_select_fm_freq = 0;	//

extern int hdmi_unmute_cnt;


/*全局*/
/*电源管理相关*/
#ifdef CONFIG_PM
pthread_cond_t stop_cond = PTHREAD_COND_INITIALIZER;
#endif
/*开关机状态*/
int ui_power = POWER_START;
int play_count = 0;


extern int irkey_detect;

bool locked_mode_flag = false;

/*Flash一页大小*/
#define FLASH_PAGE_SIZE (256)
/*Flash用户数据区开始地址*/
#define FLASH_USER_SPACE_ADDR  0x00213100 //(0x00213000)

/*当前TREBLE级别*/
static int treble_level = 0;
/*当前BASS级别*/
static int bass_level = 0;
/* adc 读取旧值*/

int hdmisend_count = 0;
int unmute_count = 0;
int pad_test = 100;

bool volume_save_flag = false;
int save_vol_cnt = 0;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/*外部*/
extern int fat_get_unicode_size(WORD *ustr);
extern int fat_utf8_to_unicode(const char* str, WORD *ustr, int len);
extern void send_cmd_2_ui(ui_cmd_t *ui_cmd);
extern void sysfs_sd_umount(void);
extern void sysfs_usb_umount(void);
extern void time2str(int curtime, int totaltime, char *str);
extern void usb_manual_disconnect(void);

/*内部*/
static void lcd_display_set_source(int num);
static void lcd_display_str(const char *dis_str, const int line);
static void ui_process_vol_dec(void);
static void ui_process_vol_inc(void);
static bool update_params_2_eeprom(void);

static bool update_params_from_eeprom(void);

static bool write_fmparams_2_eeprom(unsigned char *pbuf, int len);
static bool update_fmparams_from_eeprom(void);
extern void yunos_bsp_flash_erase(int addr, int cmd);
extern int zhuque_bsp_flash_page_program(int addr, int length, int buf);
extern int yunos_bsp_flash_read(int offset, int nbytes, unsigned char *buffer);

/****************************************************************************
 * Public Functions
 ****************************************************************************/
int iis_sample_rate = IIS_SAMP_44K;
void sl_ui_set_samp(bool on_off )
{
#if 1
	char *samp_str;
	if(on_off)
	{
		samp_str = "0 2 2 48000 48000";
		iis_sample_rate = IIS_SAMP_48K;
	}
	else
	{
		samp_str = "0 2 2 44100 44100";
		iis_sample_rate = IIS_SAMP_44K;
	}
	player_process_cmd(NP_CMD_SET_SAMPLE, samp_str, 0, NULL, NULL);
	usleep(1000);
#endif
	player_process_cmd(NP_CMD_SET_REQRATE, NULL, 44100, NULL, NULL);
	usleep(1000);

	printf("\n%s %s \n", __func__, samp_str);
}

void sl_ui_set_reqrate(void)
{
	player_process_cmd(NP_CMD_SET_REQRATE, NULL, 44100, NULL, NULL);
	usleep(1000);
}


void ui_handle_set_samp(bool flag)
{
	if (ui_source_select == SOURCE_SELECT_BT)
	{
 		char *samp_str;
		if (flag) //48k
		{
			if (iis_sample_rate != IIS_SAMP_48K)
			{

				samp_str = "0 2 2 48000 48000";
				iis_sample_rate = IIS_SAMP_48K;

				player_process_cmd(NP_CMD_SET_SAMPLE, samp_str, 0, NULL, NULL);
				usleep(1000);

				//player_process_cmd(NP_CMD_SET_REQRATE, NULL, 48000, NULL, NULL);
				//usleep(1000);

				//player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
				//usleep(1000);
				//player_process_cmd(NP_CMD_I2SIN_OPEN, NULL, 0, NULL, NULL);
				//usleep(1000);
				printf("\n set samp 48000 \n");
			}
		}
		else
		{
			if (iis_sample_rate != IIS_SAMP_44K)
			{

				samp_str = "0 2 2 44100 44100";
				iis_sample_rate = IIS_SAMP_44K;

				player_process_cmd(NP_CMD_SET_SAMPLE, samp_str, 0, NULL, NULL);
				usleep(1000);

				//player_process_cmd(NP_CMD_SET_REQRATE, NULL, 44100, NULL, NULL);
				//usleep(1000);

				//player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
				//usleep(1000);
				//player_process_cmd(NP_CMD_I2SIN_OPEN, NULL, 0, NULL, NULL);
				//usleep(1000);
				printf("\n set samp 44100 \n");
			}
		}
		//printf("%s, %d , %d", __func__, __LINE__, iis_sample_rate);

 	}
}


#if  0
void sl_ui_set_samp(bool on_off )
{
#if 1
		char *samp_str;
		if(on_off)
			samp_str = "0 2 2 48000 48000";
		else
			samp_str = "0 2 2 44100 44100";

		player_process_cmd(NP_CMD_SET_SAMPLE, samp_str, 0, NULL, NULL);
#endif

		//usleep(1000);
		//player_process_cmd(NP_CMD_SET_REQRATE, NULL, 48000, NULL, NULL);
		//player_process_cmd(NP_CMD_SET_REQRATE, NULL, 44100, NULL, NULL);
		//usleep(1000);
}
#endif

/****************************************************************************
 * Name: bt_chk_and_disp
 *
 * Description:
 *    蓝牙连接状态检查并显示
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void bt_chk_and_disp(void)
{
	unsigned long value;
    static bool isDisp = true;
	static bool isMuteFlash = true;
    if(ui_source_select == SOURCE_SELECT_BT)
    {
		if(!is_volume_mode)
		{
			if(bt_connected)
			{
				//printf("bt_chk_and_disp..........bt_connected\n");
				if(bt_play_status == true)
				{
					play_count++;
					if(play_count == 10)
					{
						play_count = 0;
						if(!isDisp)
						{
							//printf("play_blueled_on\n");
							show_led_mode(SOURCE_SELECT_BT);
							isDisp = true;
						}
						else
						{
							//printf("play_blueled_off\n");
							close_led_mode(SOURCE_SELECT_BT);
							isDisp = false;
						}
					}
				}
				else
				{
					//mute_led_on();
					show_led_mode(SOURCE_SELECT_BT);
				}
			}
			else
			{
				if(!isDisp)
				{
					//printf("wait_blueled_on\n");
					show_led_mode(SOURCE_SELECT_BT);
					isDisp = true;
				}
				else
				{
					//printf("wait_blueled_off\n");
					close_led_mode(SOURCE_SELECT_BT);
					isDisp = false;
				}
			}
		}
	}
}

int bt_mute_det_cnt = 0;
void ui_bt_mute_handle(void)
{
	static uint32_t ioval = 0;
    if(ui_source_select == SOURCE_SELECT_BT && ui_power == POWER_ON )
    {
		//if (mute_state == UNMUTE)
		{
			zhuque_bsp_gpio_get_value(HW_BT_MUTE_PIN, &ioval);
			if (ioval)
			{
				if (++bt_mute_det_cnt >= 30)
				{
					Hw_Amp_UnMute();
					bt_mute_det_cnt = 0;
				}
			}
			else
			{
				bt_mute_det_cnt = 0;
				Hw_Amp_Mute();
			}
		}
    }
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
    //初始化蓝牙发送同步信号量
    sem_init(&bt_state_sem, 0, 1);
    //初始化蓝牙开始检测状态信号量
    sem_init(&bt_start_check_sem, 0, 1);
}

void bt_playsta_mute(void)
{
#if 0
	usleep(500000);
	usleep(500000);
	usleep(500000);
	if(bt_play_status == false)
	{
		if((mute_state == UNMUTE)&&(mute_state_flag == false))
		{
			ui_handle_mute();
		}
	}
#endif
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
    int ret;
    char buf_rev[16];
    bool vol_is_init = false;
	int vol1 = 0;
	int vol2 = 0;
	int vol = 0;
	ui_cmd_t cmd;

    while (1)
    {
        if(ui_power == POWER_ON)
        {
            ret = bt_read(buf_rev);

            if (ret > 0)
            {
                buf_rev[15] = '\0';

                if (strstr(buf_rev, "VOL+") != NULL)
                {
                    printf("buf_rev %s\n", buf_rev);
                    printf("recv vol set value %c %c\n", buf_rev[7], buf_rev[8]);

				    if('0' <= buf_rev[7] && buf_rev[7] <= '9')
	                {
	                    vol1 = buf_rev[7] - '0';
	                }

				    if('0' <= buf_rev[8] && buf_rev[8] <= '9')
	                {
	                    vol2 = (buf_rev[8] - '0');
	                }
					vol = vol1*10 + vol2;
					if (vol >= 0 && vol <= 16)
					{
						printf("bt value %d \n", vol);
						cmd.cmd = UI_CMD_BT_SET_VOLUME;
						cmd.arg2 = (int)(vol);
						send_cmd_2_ui(&cmd);
					}

				}

				#if 0
                if (strstr(buf_rev, "ON") != NULL)
                {
					cmd.cmd = UI_CMD_BT_SEND_VOLUME;
					cmd.arg2 = 0;
					send_cmd_2_ui(&cmd);
                    printf("%s:bluetooth power on.\n", __func__);
                }
				#endif

                if (strstr(buf_rev, "MB\n") != NULL)
                {
                    bt_play_status = true;
					#if 0
					if((mute_state == MUTE)&&(mute_state_flag == false))
					{
						ui_handle_mute();
					}
					#endif
                    printf("%s:bluetooth play.\n", __func__);
                }
                if (strstr(buf_rev, "MA\n") != NULL)
                {	
                    bt_play_status = false;
					#if 0
					cmd.cmd = UI_CMD_PLAYSTA_MUTE;
					send_cmd_2_ui(&cmd);
					#endif				
                    printf("%s:bluetooth pause.\n", __func__);
					
                }

                if (strstr(buf_rev, "PAPR\n") != NULL)
                {
                    irkey_detect = KEY_POWER;
                    printf("%s:power key detect.\n", __func__);
                }

                if (strstr(buf_rev, "MU\n") != NULL)
                {

					irkey_detect = IR_MUTE;
					if (mute_state == MUTE)
					{
						mute_state_flag = false;
					}
					else
					{
						mute_state_flag = true;
					}

                    printf("%s:AT+MU IRkey detect.\n", __func__);
                }

                if (strstr(buf_rev, "EQ1\n") != NULL)
                {
                    irkey_detect = IR_EQ1;
                    printf("%s:AT+EQ1 IRkey detect.\n", __func__);
                }

                if (strstr(buf_rev, "EQ2\n") != NULL)
                {
                    irkey_detect = IR_EQ2;
                    printf("%s:AT+EQ2 IRkey detect.\n", __func__);
                }

                if (strstr(buf_rev, "EQ3\n") != NULL)
                {
                    irkey_detect = IR_EQ3;
                    printf("%s:AT+EQ3 IRkey detect.\n", __func__);
                }

                if (strstr(buf_rev, "LINE1\n") != NULL)
                {
                    irkey_detect = IR_LINE;
					bt_connected_flag = true;
                    printf("%s:AT+LINE IRkey detect.\n", __func__);
                }

                if (strstr(buf_rev, "HDMI1\n") != NULL)
                {
                    irkey_detect = IR_HDMI;
					bt_connected_flag = true;
                    printf("%s:AT+HDMI IRkey detect.\n", __func__);
                }

                if (strstr(buf_rev, "AUX1") != NULL)
                {
                    irkey_detect = IR_AUX;
                    printf("%s:AT+AUX IRkey detect.\n", __func__);
                }

                if (strstr(buf_rev, "VOL_UP\n") != NULL)
                {
					irkey_detect = IR_VOL_UP;
                    printf("%s:AT+VOL_UP  IRkey detect.\n", __func__);
                }

                if (strstr(buf_rev, "VOL_DOWN\n") != NULL)
                {
					irkey_detect = IR_VOL_DOWN;
                    printf("%s:AT+VOL_DOWN IRkey detect.\n", __func__);
                }

                if (strstr(buf_rev, "BT1") != NULL)
                {
                    irkey_detect = IR_BT;
                    printf("%s:AT+BT IRkey detect.\n", __func__);
                }

				if (strstr(buf_rev, "PA\n") != NULL)
                {
					if(ui_source_select != SOURCE_SELECT_BT)
					{
						irkey_detect = IR_PLAY;
					}

                    printf("%s:AT+PA IRkey detect.\n", __func__);
                }

				if ((strstr(buf_rev, "PD\n") != NULL)||(strstr(buf_rev, "PE\n") != NULL))
                {
					if(ui_source_select == SOURCE_SELECT_BT)
					{
						if (mute_state == MUTE)
					    {
					        ui_handle_mute();
					    }
					}
                    printf("%s:AT+PE IRkey detect.\n", __func__);
                }

                if (strstr(buf_rev, "AUXIN\n") != NULL)
                {
                    aux_in_status = true;
					if(locked_mode_flag == true)
					{
						locked_mode_flag = false;
					}
					else
					{
						irkey_detect = IR_AUX;
					}
                    printf("%s:AUXIN...aux_in_status = true.\n", __func__);
                }
                if (strstr(buf_rev, "AUXOUT\n") != NULL)
                {
                    aux_in_status = false;
					if (ui_source_select == SOURCE_SELECT_LINEIN)
					{
						irkey_detect = IR_BT;
					}

                    printf("%s:AUXOUT...aux_in_status = false.\n", __func__);
                }

				if (strstr(buf_rev, "IDLE\n") != NULL)
                {
					ui_power = POWER_OFF;
					show_led_off();
					player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
					usleep(100);
					/*if (ui_source_select == SOURCE_SELECT_HDMI)
					{
						action_hdmi_standby();
					}*/
                    printf("%s:AT+IDLE.\n", __func__);
                }

				if (strstr(buf_rev, "OFF\n") != NULL)
                {
					mute_state = UNMUTE;
                    ui_handle_mute();
                    printf("%s:AT+OFF.\n", __func__);
                }
#if 0
				if (strstr(buf_rev, "MIN+OK\n") != NULL)
                {
                    min_ok_flag      = true;
					set_vol = 0;
					ui_process_vol_dec();
                    printf("%s:MIN+OK.\n", __func__);
                }

				if (strstr(buf_rev, "MAX+OK\n") != NULL)
                {
                    max_ok_flag      = true;
					set_vol =	16;
					ui_process_vol_inc();
                    printf("%s:MAX+OK.\n", __func__);
                }

#endif
				if (strstr(buf_rev, "C1\n") != NULL)
                {
                    if (!bt_connected)
                    {
                    	bt_connected = true;
						#if 0
						if(set_vol < 16)
						{
							cmd.cmd = UI_CMD_BT_SEND_VOLUME;
							cmd.arg2 = 0;
							send_cmd_2_ui(&cmd);
						}
						#endif
					}
                    printf("%s:bluetooth is connected.\n", __func__);
                }

                if (strstr(buf_rev, "C0\n") != NULL)
                {
					bt_play_status = false;
					if(bt_connected_flag ==    true)
					{
						bt_connected_flag = false;
						if((aux_in_status == false)||(hdmi_det_online == 0))
						{

						}
						else
						{
							bt_connected = false;
                    		printf("%s:bluetooth is disconnect.\n", __func__);
						}
					}
					else
					{
						bt_connected = false;
                    	printf("%s:bluetooth is disconnect.\n", __func__);
					}

                }

				if (strstr(buf_rev, "44K\n") != NULL) //"T+48K"
                {

					cmd.cmd = UI_CMD_SET_44K;
					cmd.arg2 = 0;
					send_cmd_2_ui(&cmd);

                    printf("%s:44K.\n", __func__);
                }

				if (strstr(buf_rev, "48K\n") != NULL) //"T+48K"
                {

					cmd.cmd = UI_CMD_SET_48K;
					cmd.arg2 = 0;
					send_cmd_2_ui(&cmd);

                    printf("%s:48K.\n", __func__);
                }

            }

            usleep(100000);
        }
        else
        {
//			int semval;
//			sem_getvalue(&bt_state_sem, &semval);
//			printf("%s %d semval :%d \n", __func__, __LINE__, semval);
//            sem_wait(&bt_state_sem);
//            bt_uart_close();
//            sem_post(&bt_state_sem);
//
//			sem_getvalue(&bt_start_check_sem, &semval);
//			printf("%s %d semval :%d \n", __func__, __LINE__, semval);
//            sem_wait(&bt_start_check_sem);
//            printf("%s:Recover from semaphore\n", __func__);
//            bt_uart_init();
//            vol_is_init = false;
//            usleep(100000);
        }
    }
}

/****************************************************************************
 * Name: bt_power
 *
 * Description:
 *    蓝牙模块电源控制
 *
 * Parameters:
 *    en 1-使能，0-禁用
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void bt_power(bool en)
{
    //zhuque_bsp_gpio_set_mode(16, GPIO_OUT, PULLING_HIGH);
    //zhuque_bsp_gpio_set_value(16, en?1:0);
}

/****************************************************************************
 * Name: display_volume
 *
 * Description:
 *    显示音量值
 *
 * Parameters:
 *    vol 要显示的音量值
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void display_volume(float vol)
{
    int vol_i = vol;
    char volume[10] = {0};
    sprintf(volume, "Vol %d", vol_i);

    //显示音量字符串
    //lcd_display_str(volume, MEDIA_LINE);

//	ui_led_disp_volume(vol_i);
//	ui_led_update_display();

    //设置延时一段时间后显示音源
//    ui_goback_source(900);
}

#ifdef CONFIG_PM
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
 *    启动线程进行主频切换处理
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

/****************************************************************************
 * Name: lcd_display_clear_line
 *
 * Description:
 *    清除指定行的显示
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void lcd_display_clear_line(int line)
{
    WORD show[INFO_LEN];
    memset(show, 0, INFO_LEN*sizeof(WORD));
    lcd_display(0, line, show, 0, INFO_LEN);
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
static void lcd_display_set_source(int num)
{
    printf(" >>>>>> %s:%d\n", __func__, num);
    lcd_clear();

    switch (num)
    {
        case SOURCE_SELECT_BT:
            lcd_display_str(bt_str, MEDIA_LINE);
            break;
//        case SOURCE_SELECT_SPDIFIN:
//            lcd_display_str(opti_str, MEDIA_LINE);
//            break;
        case SOURCE_SELECT_LINEIN:
            lcd_display_str(aux_str, MEDIA_LINE);
            break;
        case SOURCE_SELECT_USB:
            lcd_display_str(usb_str, MEDIA_LINE);
            break;
        case SOURCE_SELECT_SD:
            lcd_display_str(sd_str, MEDIA_LINE);
            break;
    }
}

/****************************************************************************
 * Name: lcd_display_str
 *
 * Description:
 *    字符串显示
 *
 * Parameters:
 *    dis_str 要显示的字符串数组
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void lcd_display_str(const char *dis_str, const int line)
{
    if (dis_str != NULL)
    {
        WORD show[INFO_LEN];
        memset(show, 0, INFO_LEN*sizeof(WORD));
	    fat_utf8_to_unicode(dis_str, show, MAX_STRING_LEN);
        lcd_display(0, line, show, 0, fat_get_unicode_size(show)/2);
    }
}

/****************************************************************************
 * Name: play_local
 *
 * Description:
 *    音源切换播放处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static int play_local(void)
{
#ifdef SL_UI_DBG
    printf("%s.....\n", __func__);
#endif

    switch (ui_source_select)
    {
    	/*
        case SOURCE_SELECT_SPDIFIN:
        {
			printf(">>>>>>>>>> opt mode <<<<<<<<<<\n");
			player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_INNER_IN, 2, 0);
			usleep(1000);
			player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
            player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
            usleep(1000);
            player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
            usleep(1000);
            break;
        }
        */
        case SOURCE_SELECT_HDMI:
        {
			printf(">>>>>>>>>> hdmi mode <<<<<<<<<<\n");

			/*if(frist_hdmi_init == true)
			{
				frist_hdmi_init = false;
				usleep(2000000);
			}*/

			//player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_INNER_IN, 2, 1);
			//player_paramter_set_init(AUDIO_EXTRA_CODEC_SLAVE_OUT, AUDIO_INNER_IN, 2, 1);
			//usleep(1000);

            sem_wait(&bt_state_sem);
            handle_bt_cmd(AT_D8836);
            sem_post(&bt_state_sem);
			usleep(20000);

        	set_hdmi_gain(3);
			usleep(1000);
			//if (mute_state != MUTE)	Hw_Amp_UnMute();
			sl_ui_set_reqrate();
			usleep(1000);
			//player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
			//usleep(1000);
			//HDMI
			action_hdmi_on();
			usleep(50000);
			player_paramter_set_init(AUDIO_EXTRA_CODEC_SLAVE_OUT, AUDIO_EXTRA_SLAVE_IN, 2, 1);
			usleep(50000);
			player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
			usleep(50000);
			wd_start(wdtimer_hdmion_send, 900, ui_hdmion_send, 0);

            break;
        }
        case SOURCE_SELECT_BT:
        {
			printf(">>>>>>>>>> bluetooth mode <<<<<<<<<<\n");

			//player_paramter_set_init(AUDIO_EXTRA_CODEC_SLAVE_OUT, AUDIO_EXTRA_SLAVE_IN, 2, 0);
			//usleep(10000);
		    //Bt_Power_On();
			//usleep(100000);

			switch_4052_function(BT_4052);
			
            sem_wait(&bt_state_sem);
            handle_bt_cmd(BT_MODE);
            sem_post(&bt_state_sem);
			usleep(20000);

			//sem_wait(&bt_state_sem);
			//handle_bt_send_volume(set_vol);
			//sem_post(&bt_state_sem);

			set_adc0_vol(50);

			//player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_EXTRA_MASTER_IN, 2, 0);
			//player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_EXTRA_SLAVE_IN, 2, 0);

        	set_hdmi_gain(6);

        	//ui_handle_mute();
            //if (mute_state != MUTE)	ui_handle_mute();
			//player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
            is_volume_mode = false;

			sl_ui_set_samp(0);

			//player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
            //usleep(1000);
			player_process_cmd(NP_CMD_I2SIN_OPEN, NULL, 0, NULL, NULL);
            usleep(1000);
#if 0
            if (bt_state_tid < 0)
            {
                if(pthread_create(&bt_state_tid, NULL, (pthread_startroutine_t)bt_read_state, NULL) == 0)
                {
                    printf("%s:Create check bluetooth connect state thread success.bt_state_tid = %d\n", __func__,bt_state_tid);
                }
                else
                {
                    printf("%s:Create check bluetooth connect state thread fail\n", __func__);
                }
            }
            else
            {
                printf("%s:pid = %d\n", __func__, bt_state_tid);
            }

            usleep(100);
#endif
            if(!bt_connected)
            {   //蓝牙未连接
				int semval;
				sem_getvalue(&bt_state_sem, &semval);
				printf("%s %d semval :%d \n", __func__, __LINE__, semval);

				if (bt_force_disconnect)
				{
					bt_force_disconnect = false;
	                sem_wait(&bt_state_sem);
	                handle_bt_cmd(BT_DIS_CONNECT);
	                sem_post(&bt_state_sem);
				}
				else
				{
	                sem_wait(&bt_state_sem);
	                handle_bt_cmd(BT_AUTO_CONNECT);
	                sem_post(&bt_state_sem);
				}
            }

            break;
        }
        case SOURCE_SELECT_LINEIN:
        {

            printf(">>>>>>>>>> aux mode <<<<<<<<<<\n");

			//player_paramter_set_init(AUDIO_EXTRA_CODEC_SLAVE_OUT, AUDIO_EXTRA_SLAVE_IN, 2, 0);
			//usleep(1000);

			switch_4052_function(AUX1_4052);
			//usleep(100000);
			
            sem_wait(&bt_state_sem);
            handle_bt_cmd(AUX_MODE);
            sem_post(&bt_state_sem);
			usleep(20000);

			//player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_EXTRA_MASTER_IN, 2, 0);
			//player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_EXTRA_SLAVE_IN, 2, 0);
			set_adc0_vol(100);

        	set_hdmi_gain(3);

			//if (mute_state != MUTE)	Hw_Amp_UnMute();
			//player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);

			sl_ui_set_samp(0);

			//player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
            //usleep(1000);
			player_process_cmd(NP_CMD_I2SIN_OPEN, NULL, 0, NULL, NULL);
            usleep(1000);


            break;

        }
        case SOURCE_SELECT_LINEIN1:
        {
			printf(">>>>>>>>>> aux2 mode <<<<<<<<<<\n");

			//player_paramter_set_init(AUDIO_EXTRA_CODEC_SLAVE_OUT, AUDIO_EXTRA_SLAVE_IN, 2, 0);
			//usleep(1000);
			
			switch_4052_function(AUX2_4052);
			//usleep(100000);

            sem_wait(&bt_state_sem);
            handle_bt_cmd(AUX_MODE);
            sem_post(&bt_state_sem);
			usleep(20000);

			//player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_EXTRA_MASTER_IN, 2, 0);
			//player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_EXTRA_SLAVE_IN, 2, 0);

			set_adc0_vol(100);
			
        	set_hdmi_gain(3);

			//if (mute_state != MUTE)	Hw_Amp_UnMute();
			//player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);

			sl_ui_set_samp(0);

			//player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
            //usleep(1000);
			player_process_cmd(NP_CMD_I2SIN_OPEN, NULL, 0, NULL, NULL);
            usleep(1000);

            break;
        }
    }
    return 0;
}

/****************************************************************************
 * Name: save_sd_play_time
 *
 * Description:
 *    保存SD的播放时间
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void save_sd_play_time(void)
{
    //获取播放器信息
    ui_info_t player_info;
    player_get_info(&player_info);
    if (ui_source_select == SOURCE_SELECT_SD)
    {
        //记录播放时间
        sd_playtime=player_info.curtime;
    }
}

/****************************************************************************
 * Name: save_usb_play_time
 *
 * Description:
 *    保存USB的播放时间
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void save_usb_play_time(void)
{
    //获取播放器信息
    ui_info_t player_info;
    player_get_info(&player_info);
    if (ui_source_select == SOURCE_SELECT_USB)
    {
        //记录播放时间
        usb_playtime=player_info.curtime;
    }
}

void ui_hdmi_unmute(void)
{
	usleep(100000);
	sem_wait(&bt_state_sem);
    handle_bt_cmd(AT_UNMUTE);
    sem_post(&bt_state_sem);
	usleep(20000);
	player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
	usleep(1000);
}

void ui_hdmion_send(void)
{
#if 0
	static int send_count1=0,send_count2=0;
	ui_cmd_t cmd;
/////////////////////////////////////////////////////
	hdmisend_count++;

	if(hdmisend_count  <= 6)
	{
		cmd.cmd = UI_CMD_HDMION_SEND;
		send_cmd_2_ui(&cmd);
	}
	#if 0
	send_count1++;
	if(send_count1 > 100)
	{
		send_count1 = 0;
		send_count2++;

		cmd.cmd = UI_CMD_HDMION_SEND;
		send_cmd_2_ui(&cmd);
		//action_hdmi_on();
	}

	if(send_count2 > 4)
	{
		wd_cancel(wdtimer_hdmion_send);
	}
	#endif

	if(wdtimer_hdmion_send           != NULL)
    {
		wd_start(wdtimer_hdmion_send, 6000,ui_hdmion_send, 0);
    }

    #ifdef SL_UI_DBG
    printf("%s\n",__func__);
    #endif
#else
	static int send_count1=0,send_count2=0;
	ui_cmd_t cmd;

	if(NULL != wdtimer_hdmion_send)
	{
		wd_start(wdtimer_hdmion_send, 1000, ui_hdmion_send, 0);
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

#endif
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
 static int wdt_enter_times = 0;
static bool wdt_led_flash = false;
static void ui_display_source(int argc, wdparm_t arg1, ...)
{
    if(NULL != wdtimer_goback_mode)
    {
        wd_cancel(wdtimer_goback_mode);
    }

	if (++wdt_enter_times < 6)
	{
		if (wdt_led_flash == true)
		{
			show_led_mode(ui_source_select);
		}
		else
		{
			close_led_mode(ui_source_select);
		}
		wdt_led_flash = wdt_led_flash ? false : true;
        wd_start(wdtimer_goback_mode, 50, ui_display_source, 0);
	}
	else
	{
		wdt_enter_times = 0;
		is_volume_mode = false;
		wdt_led_flash = false;
        wd_cancel(wdtimer_goback_mode);
		show_led_mode(ui_source_select);
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
        wd_start(wdtimer_goback_mode, 50, ui_display_source, 0);
    }

    #ifdef SL_UI_DBG
    printf("%s\n",__func__);
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
void ui_handle_down(void)
{
    if ((mute_state == MUTE)&&(set_vol >  0))
    {
		usleep(50000);
        ui_handle_mute();
    }

	if (ui_source_select == SOURCE_SELECT_BT)
	{
		if(bt_play_status == false)
		{
			sem_wait(&bt_state_sem);
			handle_bt_cmd(BT_PAUSE);
			sem_post(&bt_state_sem);
		}
	}

    ui_process_vol_dec();
#ifdef SL_UI_DBG
    printf("%s, volume down\n", __func__);
#endif
}

/****************************************************************************
 * Name: ui_handle_file_load
 *
 * Description:
 *    处理文件加载
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_file_load(int total_num, int folder_num)
{
	printf(">>>>>>>>>>>>>>>>>>>> %d total:%d fold:%d sd_total:%d sd_fold:%d usd_total:%d usb_fold:%d\n",
		ui_source_select, total_num, folder_num, sd_last_total_num, sd_last_folder_num,
		usb_last_total_num, usb_last_folder_num);
    if(ui_source_select == SOURCE_SELECT_USB)
    {
		is_usb_load = true;
        if(usb_last_total_num != total_num || usb_last_folder_num != folder_num)
        {
            usb_last_file_index = 0;
            usb_playtime = 0;
        }
        usb_last_total_num = total_num;
        usb_last_folder_num = folder_num;
        //将参数写入EEPROM
        //update_params_2_eeprom();
        handle_local_music_play(usb_last_file_index, usb_playtime);
    }
    else if(ui_source_select == SOURCE_SELECT_SD)
    {
		is_sd_load = true;
        if(sd_last_total_num != total_num || sd_last_folder_num != folder_num)
        {
            sd_last_file_index = 0;
            sd_playtime = 0;
        }
        sd_last_total_num = total_num;
        sd_last_folder_num = folder_num;
        //update_params_2_eeprom();
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

	is_sd_load = true;
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
	is_usb_load = true;
}

void sync_dsp_startup(void)
{
	int dsp_on = -1;
	while(1)
	{
		dsp_on = swa_audio_decode_init();
		if (dsp_on == 0 || dsp_on == 1)
		{
			break;
		}
		usleep(1);
	}

}

/****************************************************************************
 * Name: ui_handle_mode
 *
 * Description:
 *    模式切换处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_mode(void)
{
	//mute_state = UNMUTE;
	//ui_handle_mute();
	//Hw_Amp_Mute();
	player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
	usleep(1000);
	sem_wait(&bt_state_sem);
    handle_bt_cmd(AT_MUTE);
    sem_post(&bt_state_sem);
	usleep(20000);
#ifdef SL_UI_DBG
    printf("%s.....%d\n", __func__, ui_source_select);
#endif
#if 0
    //发送命令停止对应功能
    if (ui_source_select == SOURCE_SELECT_LINEIN)
    {
         player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
    }
	else if(ui_source_select == SOURCE_SELECT_LINEIN1)
    {
	    player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
		//set_hdmi_gain(5);
    }
	else if (ui_source_select == SOURCE_SELECT_BT)
    {
        //player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
		//set_hdmi_gain(5);
    	if (!bt_force_disconnect)
    	{
			sem_wait(&bt_state_sem);
			handle_bt_cmd(BT_DIS_CONNECT);
			sem_post(&bt_state_sem);
			//usleep(100000);
        	//Bt_Power_Off();
    	}
		bt_connected = 0;

    }
    else if (ui_source_select == SOURCE_SELECT_SPDIFIN)
    {
        player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
    }
    else if (ui_source_select == SOURCE_SELECT_HDMI)
    {
		player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
    	//hdmi
		player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
        usleep(1000);
		cec_process_cmd(CEC_CMD_ARCOFF, NULL);
		//set_hdmi_gain(5);
    }
    else if (ui_source_select == SOURCE_SELECT_USB ||
		ui_source_select == SOURCE_SELECT_SD)
    {
        player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);
    }

#endif
	if (ui_source_select == SOURCE_SELECT_BT)
	{
		//player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
		//set_hdmi_gain(5);
		if (!bt_force_disconnect)
		{
			sem_wait(&bt_state_sem);
			handle_bt_cmd(BT_DIS_CONNECT);
			sem_post(&bt_state_sem);
			//usleep(100000);
			//Bt_Power_Off();
		}
		bt_connected = 0;
	}
	else if (ui_source_select == SOURCE_SELECT_HDMI)
	{
		if (hdmi_det_online)
		{
			//�ر�hdmi
			action_hdmi_off();
			usleep(1000);
		}
	}

	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
	usleep(1000);
	player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);
	usleep(1000);
	player_process_cmd(NP_CMD_I2SIN_CLOSE, NULL, 0, NULL, NULL);
	usleep(1000);
	bt_play_status = false;

    usleep(1000);

	if (ui_source_ir_select >= 0)
	{
		ui_source_select = ui_source_ir_select;
		ui_source_ir_select = -1;
	}
	else
	{
		//切换模式
	    if (ui_source_select < SOURCE_SELECT_END - 1)
	    {
	        ui_source_select++;
	    }
	    else
	    {
	        ui_source_select = SOURCE_SELECT_START + 1;
	    }
        #if 1
        if(ui_source_select == SOURCE_SELECT_LINEIN)
        {
	        if(aux_in_status == false)
	        {
	             ui_source_select = SOURCE_SELECT_LINEIN1;
	        }
        }
        if(ui_source_select == SOURCE_SELECT_HDMI)
        {
            if(hdmi_det_online == 0)
            {
                 ui_source_select = SOURCE_SELECT_BT;
            }
        }
        #endif
	}
#if 0
    ui_source_select_temp = ui_source_select;

	if (update_params_2_flash())
	{
		printf("handle mode save success !\n");
	}
	else
	{
		printf("handle mode save fail !\n");
	}
#endif

	//显示音源
	show_led_mode(ui_source_select);

	if (update_params_2_flash())
	{
		printf("write 2 flash success !\n");
	}
	else
	{
		printf("write 2 flash fail !\n");
	}

    //模式切换处理
    play_local();

	if(ui_source_select != SOURCE_SELECT_HDMI)
	{
		//usleep(500000);
		//usleep(500000);
		//usleep(500000);
		usleep(500000);
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
		usleep(1000);
		sem_wait(&bt_state_sem);
	    handle_bt_cmd(AT_UNMUTE);
	    sem_post(&bt_state_sem);
		usleep(20000);		
	}

	#if 0
	if(ui_source_select != SOURCE_SELECT_HDMI)
	{
		usleep(500000);
		usleep(500000);
		usleep(500000);
		usleep(500000);
		ui_handle_mute();
		//Hw_Amp_UnMute();
	}
	else
	{
		hdmi_unmute_cnt = 0;
	}
	#endif

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
void ui_handle_mute(void)
{
    printf("\n%s\n", __func__);
    if (mute_state == UNMUTE) //静音
    {
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);

		mute_state = MUTE;

		//Hw_Amp_Mute();

        //mute_led_on();

        #if BYPASS_MODE
        if(ui_source_select != SOURCE_SELECT_SPDIFIN)
        {
            silan_set_linebypass_disable();
        }
        #endif
    }
    else						//取消静音
    {
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);

        mute_state = UNMUTE;

		//Hw_Amp_UnMute();

        #if BYPASS_MODE
        if(ui_source_select != SOURCE_SELECT_SPDIFIN && bypass_in_vol > BYPASS_VOL_IN_MIN)
        {
            silan_set_linebypass_enable();
        }
        #endif
    }
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
    if (ui_source_select == SOURCE_SELECT_BT)
    {   //蓝牙模式
		int semval;

		if (mute_state == MUTE)
	    {
	        ui_handle_mute();
	    }

		sem_getvalue(&bt_state_sem, &semval);
		printf("%s %d semval :%d \n", __func__, __LINE__, semval);
        sem_wait(&bt_state_sem);
        handle_bt_cmd(BT_PRV);
        sem_post(&bt_state_sem);
    }
    else if (ui_source_select == SOURCE_SELECT_USB ||
             ui_source_select == SOURCE_SELECT_SD)
    {
        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
        int *p_playtime = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
        int total = get_file_total();
        if (total > 0)
        {
            if(--(*p_index) < 0)
            {
                *p_index = total-1;
            }
        }
        else
        {
            *p_index = 0;
        }
        *p_playtime = 0;
        handle_local_music_play(*p_index, *p_playtime);

		//参数写入EEPROM
		//update_params_2_eeprom();

    }

    printf("%s", __func__);
}


/****************************************************************************
 * Name: ui_handle_finish
 *
 * Description:
 *    播放结束控制
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_finish(void)
{
    if (ui_source_select == SOURCE_SELECT_USB ||
             ui_source_select == SOURCE_SELECT_SD)
    {
        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
        int *p_playtime = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
        int total = get_file_total();
        if (total > 0)
        {
            if(++(*p_index) >= total)
            {
                *p_index = 0;
            }
	        *p_playtime = 0;
            handle_local_music_play(*p_index, *p_playtime);
			//参数写入EEPROM
			//update_params_2_eeprom();
        }

    }
	else
	{
		ui_handle_next();
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
unsigned char  ui_test_setvolume=0;
void ui_handle_next(void)
{
    if (ui_source_select == SOURCE_SELECT_BT)
    {
    	int val;
		if (mute_state == MUTE)
	    {
	        ui_handle_mute();
	    }
		sem_getvalue(&bt_state_sem, &val);
		printf("value :%d\n", val);
        sem_wait(&bt_state_sem);
        handle_bt_cmd(BT_NEXT);
        sem_post(&bt_state_sem);
		printf("%s sempost  :%d\n",__func__, val);
    }
    else if (ui_source_select == SOURCE_SELECT_USB ||
             ui_source_select == SOURCE_SELECT_SD)
    {
        int *p_index = (ui_source_select == SOURCE_SELECT_USB?&usb_last_file_index:&sd_last_file_index);
        int *p_playtime = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
        int total = get_file_total();
        if (total > 0)
        {
            if(++(*p_index) >= total)
            {
                *p_index = 0;
            }
            *p_playtime = 0;
            handle_local_music_play(*p_index, *p_playtime);

			//参数写入EEPROM
			//update_params_2_eeprom();
        }
    }

    printf("%s", __func__);
}

/****************************************************************************
 * Name: ui_handle_next_long_pressdown
 *
 * Description:
 *    FM长按NEXT向后搜台
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_next_long_pressdown(void)
{

}

/****************************************************************************
 * Name: ui_handle_prev_long_pressdown
 *
 * Description:
 *    FM长按prev向前搜台
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_prev_long_pressdown(void)
{

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

    if ((ui_source_select == SOURCE_SELECT_LINEIN) ||
		(ui_source_select == SOURCE_SELECT_LINEIN1) ||
        (ui_source_select == SOURCE_SELECT_SPDIFIN) ||
        (ui_source_select == SOURCE_SELECT_HDMI))
    {
    	if (aux_pause_state) //播放
    	{
			player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
			//Hw_Amp_UnMute();
			aux_pause_state = 0;
		}
		else 				//pause
		{
			player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
			//Hw_Amp_Mute();
			aux_pause_state = 1;
		}
    }
    else if(ui_source_select == SOURCE_SELECT_BT)
    {   //蓝牙模式
		int semval;
		sem_getvalue(&bt_state_sem, &semval);
		printf("%s %d semval :%d \n", __func__, __LINE__, semval);
        //向蓝牙模式发送暂停播放命令
       	#if 0
        /*if (bt_pause_state == 0) //暂停播放
        {
			player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
			Hw_Amp_Mute();
			bt_pause_state = 1;
			sem_wait(&bt_state_sem);
			handle_bt_cmd(BT_PAUSE);
			sem_post(&bt_state_sem);
        }
		else 					//恢复播放
		{
			player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
			Hw_Amp_UnMute();
			bt_pause_state = 0;
			sem_wait(&bt_state_sem);
			handle_bt_cmd(BT_PAUSE);
			sem_post(&bt_state_sem);
		}*/
//		stop_state = 0;
		#else
		sem_wait(&bt_state_sem);
		handle_bt_cmd(BT_PAUSE);
		sem_post(&bt_state_sem);
		#endif

	}
    else if(ui_source_select == SOURCE_SELECT_USB ||
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
			usleep(20000);
			Hw_Amp_UnMute();
        }

        if (stop_state) //else if (player_info.player_stat == 0)
        {
			Hw_Amp_UnMute();
			if (ui_source_select == SOURCE_SELECT_USB)
			{
				handle_local_music_play(usb_last_file_index, usb_playtime);
			}
			else
			{
				handle_local_music_play(sd_last_file_index, sd_playtime);
			}
			stop_state = 0;
        }
//		printf("player_stat %d", player_info.player_stat);
    }
}


/****************************************************************************
 * Name: ui_handle_pause_play
 *
 * Description:
 *    停止播放处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_stopplay(void)
{
    printf("\n%s\n", __func__);

	//Hw_Amp_Mute();
    if (ui_source_select == SOURCE_SELECT_LINEIN ||
        ui_source_select == SOURCE_SELECT_LINEIN1 ||
        ui_source_select == SOURCE_SELECT_SPDIFIN ||
        ui_source_select == SOURCE_SELECT_BT)
    {
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
		aux_pause_state = 1;
    }
    else if(ui_source_select == SOURCE_SELECT_BT)
    {   //蓝牙模式
        //向蓝牙模式发送暂停播放命令
        if (stop_state == 0 && bt_pause_state == 0) //蓝牙处于非暂停状态
        {
			int semval;
			sem_getvalue(&bt_state_sem, &semval);
			printf("%s %d semval :%d \n", __func__, __LINE__, semval);
	        sem_wait(&bt_state_sem);
	        handle_bt_cmd(BT_PAUSE);
	        sem_post(&bt_state_sem);
			bt_pause_state = 1;
        }
    }
    else if(ui_source_select == SOURCE_SELECT_USB)
    {
        //发送命令停止播放
        player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);
		usb_playtime = 0;
    }
	else if(ui_source_select == SOURCE_SELECT_SD)
	{
		//发送命令停止播放
		player_process_cmd(NP_CMD_STOP, NULL, 0, NULL, NULL);
		sd_playtime = 0;
	}
//	Digital_Disp_Status(DISP_STOP);
	stop_state = 1;
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
void ui_handle_power(void)
{
	int ret;
	int value;

#if 0
	sem_wait(&bt_state_sem);
	handle_bt_cmd(PWR_UP);
	sem_post(&bt_state_sem);
#endif

    if (ui_power < POWER_END - 1)
    {
        ui_power++;
    }
    else
    {
        ui_power = POWER_START + 1;
    }

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

	printf("%s %d", __func__, ui_power);

    if (ui_power == POWER_ON)
    {
        if (cur_freq == 6000000)
        {
            //enter_dynamic(NULL);
            printf("out of low power\n");
            //usb恢复
            //silan_usb_reset();
        }

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

		if(NULL == wdtimer_hdmion_send)
		{
			wdtimer_hdmion_send = wd_create();
		}

#if 1
		if(NULL == wdtimer_resetdsp_init)
		{
			wdtimer_resetdsp_init = wd_create();
		}
		dsp_init_check();
#endif

		sem_wait(&bt_state_sem);
		handle_bt_cmd(GET_AUX);
		sem_post(&bt_state_sem);

		//usleep(100000);
		printf("aux_in_status = 0x%x\n", aux_in_status);

		if (update_params_from_flash())
		{
			printf("read flash success !\n");
		}
		else
		{
			printf("read flash fail !\n");
		}
#if 0
		usleep(200000);
		zhuque_bsp_gpio_set_mode(HDMI_DET, GPIO_IN, PULLING_HIGH);
		zhuque_bsp_gpio_get_value(HDMI_DET, &value);
		usleep(200000);

		if (value == 0)
		{
			usleep(20000);
			value = 0xff;
			zhuque_bsp_gpio_get_value(HDMI_DET, &value);
			if (value == 0)
			{
				//frist_hdmi_det_online = true;
				hdmi_det_online = true;
				locked_mode_flag = true;
				printf("\n[HDMI] detected hdmi insert");
				ui_source_select = SOURCE_SELECT_HDMI - 1;
			}
		}
		/*else if(aux_in_status == true)
		{
			printf("\n[AUX] detected hdmi insert");
			ui_source_select = SOURCE_SELECT_LINEIN - 1;
		}*/
		else
		{
			if (ui_source_select <= SOURCE_SELECT_START || ui_source_select >= SOURCE_SELECT_END)
			{
				ui_source_select = SOURCE_SELECT_START;
			}
		}
		//sl_ui_set_samp(0);
#endif
		/*
		sem_wait(&bt_state_sem);
		handle_bt_send_volume(set_vol);
		sem_post(&bt_state_sem);
		*/

#if 1
		if ((ui_source_select != SOURCE_SELECT_LINEIN)&&(ui_source_select != SOURCE_SELECT_HDMI))
		{
			ui_handle_mode();
		}
#endif

/*
		if (ui_source_select == SOURCE_SELECT_HDMI)
		{
			action_hdmi_poweron();
		}
*/

    }
    else
    {
 		printf(">>>>>>>>>>>>>>>>>>enter power off ... \n");
		//Hw_Amp_Mute();

		player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);

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
		//Hw_Amp_Mute();
        player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
        usleep(100);

		if(wdtimer_resetdsp_init != NULL)
		{
			wd_cancel(wdtimer_resetdsp_init);
		}


        //关闭延时显示音源看门狗
        if (wdtimer_goback_mode != NULL)
        {
            wd_cancel(wdtimer_goback_mode);
        }

        //清除蓝牙状态标志
        bt_connected = false;
#if 0
		if (update_params_2_flash())
		{
			printf("power down save success !\n");
		}
		else
		{
			printf("power down save fail !\n");
		}
#endif

        ui_source_select = SOURCE_SELECT_START;

        if (cur_freq == 300000000)
        {
            //断开usb
            //usb_manual_disconnect();
            //sysfs_usb_umount();

            //断开sd卡
            //sysfs_sd_umount();

            printf("enter low power\n");
            //enter_dynamic(NULL);
        }

		show_led_off();
		//Bt_Power_Off();
 		printf(">>>>>>>>>>>>>>>>>> power off end !\n");
		while(1)
		{
			//Hw_Amp_Mute();
			usleep(10000);
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
    }

    if(ui_source_select != SOURCE_SELECT_USB)
    {
        //复原播放列表
        reset_playlist();
    }

	is_sd_load = false;
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
void ui_handle_up(void)
{
    if (mute_state == MUTE)
    {
		usleep(50000);
        ui_handle_mute();
    }

	if (ui_source_select == SOURCE_SELECT_BT)
	{
		if(bt_play_status == false)
		{
			sem_wait(&bt_state_sem);
			handle_bt_cmd(BT_PAUSE);
			sem_post(&bt_state_sem);
		}
	}

	//音量增加处理
    ui_process_vol_inc();

#ifdef SL_UI_DBG
    printf("%s, volume_up\n", __func__);
#endif
}

static void ui_post_event(int code, int value)
{
	struct input_event ui_event;
	memset(&ui_event, 0, sizeof(struct input_event));
	ui_event.type = EV_UI;
	ui_event.value = value;
	ui_event.code = code;
	input_add_event(&ui_event);
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
		ui_post_event(CODE_UI_UPDATE_DONE, 0);
    }

    if(ui_source_select != SOURCE_SELECT_SD)
    {
        //复原播放列表
        reset_playlist();
    }

	is_usb_load = false;
}

/****************************************************************************
 * Name: ui_handle_vol_dec_long_press
 *
 * Description:
 *    音量减物理按键长按处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static bool long_pressup_flag = false;
void ui_handle_vol_dec_long_press(void)
{
    if (NULL == wdtimer_vol_longpress)
    { //首次启动
        wdtimer_vol_longpress = wd_create();
    }
    else
    { //停止当前的看门狗
        wd_cancel(wdtimer_vol_longpress);
    }
    //发送音量控制消息
    ui_cmd_t cmd;
    cmd.cmd = UI_CMD_BT_DOWN;
    send_cmd_2_ui(&cmd);


    if (NULL != wdtimer_vol_longpress)
    { //有效的长按看门狗
        //启动重复看门狗计时器
        wd_start(wdtimer_vol_longpress, CLOCKS_PER_SEC / 4, (wdentry_t)ui_handle_vol_dec_long_press, 0);
    }
}

/****************************************************************************
 * Name: ui_handle_vol_inc_long_press
 *
 * Description:
 *    音量增加物理按键长按处理
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
    { //首次启动
        wdtimer_vol_longpress = wd_create();
    }
    else
    { //停止当前的看门狗
        wd_cancel(wdtimer_vol_longpress);
    }


 	//发送音量控制消息
    ui_cmd_t cmd;
    cmd.cmd = UI_CMD_BT_UP;
    send_cmd_2_ui(&cmd);

    if (NULL != wdtimer_vol_longpress)
    { //有效的长按看门狗
        //启动重复看门狗计时器
        wd_start(wdtimer_vol_longpress, CLOCKS_PER_SEC / 4, (wdentry_t)ui_handle_vol_inc_long_press, 0);
    }
}

/****************************************************************************
 * Name: ui_handle_vol_long_press_up
 *
 * Description:
 *    音量增减物理按键长按弹起处理
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
    {
		//long_pressup_flag = false;
		//停止当前的看门狗
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
	int vol_temp=0;
	if (set_vol >= VOL_STEP)
	{
		set_vol -= VOL_STEP;
	}
	else
	{
		set_vol = 0;	
	}

	//显示音量信息
	//display_volume(set_vol);
	mix_vol = set_vol * (100.0f/40) ;
	player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);

	volume_save_flag = true;
	save_vol_cnt = 0;

	
	sem_wait(&bt_state_sem);
	handle_bt_send_volume(set_vol);
	sem_post(&bt_state_sem);
	
#if 0
	if (ui_source_select == SOURCE_SELECT_BT)
	{

		if(set_vol ==	0)
		{
			if(min_ok_flag == true)
			{
				min_ok_flag = false;
				set_vol = 0;
				mix_vol = set_vol * (100.0f/16) ;
				player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
				mute_state = UNMUTE;
        		ui_handle_mute();

			}
			else
			{
				Hw_Amp_Mute();
				set_vol = 4;
				//显示音量信息
				//display_volume(set_vol);
				mix_vol = set_vol * (100.0f/16) ;
				player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
				set_vol = 0;
				//usleep(500000);
				Hw_Amp_UnMute();

				sem_wait(&bt_state_sem);
				handle_bt_send_volume(0);
				sem_post(&bt_state_sem);

				//usleep(100000);

			}

		}
		else
		{
			//显示音量信息
			//display_volume(set_vol);
			mix_vol = set_vol * (100.0f/16) ;

			player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);

			sem_wait(&bt_state_sem);
			handle_bt_send_volume(set_vol);
			sem_post(&bt_state_sem);
		}
	}
	else
	{
		//显示音量信息
		//display_volume(set_vol);
		mix_vol = set_vol * (100.0f/16) ;
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
	}

#endif
	#if 0
	if (set_vol <= 0)
	{
		wdt_enter_times = 0;
		wdt_led_flash = false;
		ui_goback_source(0);
	}
	#endif

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
    set_vol += VOL_STEP;
    if(set_vol > 16)
    {
        set_vol = 16;
    }
    //显示音量信息
    //display_volume(set_vol);

	//显示音量信息
	//display_volume(set_vol);
	mix_vol = set_vol * (100.0f/40) ;
	player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);

	volume_save_flag = true;
	save_vol_cnt = 0;

	
	sem_wait(&bt_state_sem);
	handle_bt_send_volume(set_vol);
	sem_post(&bt_state_sem);


    #if 0

	if (ui_source_select == SOURCE_SELECT_BT)
	{
		if(set_vol ==	16)
		{
			if(max_ok_flag == true)
			{
				max_ok_flag = false;
				set_vol = 16;
				mix_vol = set_vol * (100.0f/16) ;
				player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
			}
			else
			{
				Hw_Amp_Mute();
				set_vol = 4;
				//显示音量信息
				//display_volume(set_vol);
				mix_vol = set_vol * (100.0f/16) ;
				player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
				set_vol = 16;
				usleep(80000);
				Hw_Amp_UnMute();

				sem_wait(&bt_state_sem);
				handle_bt_send_volume(16);
				sem_post(&bt_state_sem);

				//usleep(100000);

			}

		}
		else
		{
			//显示音量信息
			//display_volume(set_vol);
			mix_vol = set_vol * (100.0f/16) ;

			player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);

			sem_wait(&bt_state_sem);
			handle_bt_send_volume(set_vol);
			sem_post(&bt_state_sem);
		}
	}
	else
	{
		//显示音量信息
		//display_volume(set_vol);
		mix_vol = set_vol * (100.0f/16) ;
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);
	}
	#endif

#if 0
    mix_vol = set_vol * (100.0f/16) ;

	player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);

	if (ui_source_select == SOURCE_SELECT_BT)
	{
		sem_wait(&bt_state_sem);
		handle_bt_send_volume(set_vol);
		sem_post(&bt_state_sem);
	}
#endif

	#if 0
	if (set_vol >= 32)
	{
		wdt_enter_times = 0;
		wdt_led_flash = false;
		ui_goback_source(0);
	}
	#endif

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

#if 0 
 #define eq_set_ch		6
 const int eq_table_1[]={
	//flt_id, type, onoff, gain, fc, q
	1, 1, 1, 0, 50, 100,eq_set_ch, //band1
	2, 0, 1, 0, 150, 100,eq_set_ch, //band2
	3, 0, 1, 0, 350, 300, eq_set_ch,//band3
	4, 0, 1, 0, 700, 300, eq_set_ch,//band4
	5, 0, 1, 0, 1800, 300, eq_set_ch,//band5
	6, 0, 1, 0, 3500, 200, eq_set_ch,//band6
	7, 2, 1, 0, 10000, 200, eq_set_ch//band7
 #if 0
	 //flt_id, type, onoff, gain, fc, q
	 1, 0, 1, 0, 100, 100, eq_set_ch,//band1
	 2, 0, 1, -400, 300, 100, eq_set_ch,//band2
	 3, 0, 1, -1200, 1000, 300, eq_set_ch,//band3
	 4, 0, 1, -1000, 3000, 300, eq_set_ch,//band4
	 5, 0, 1, -400, 8000, 300, eq_set_ch,//band5
	 6, 0, 1, 0, 8000, 200, eq_set_ch,//band6
	 7, 0, 1, 0, 10000, 200, eq_set_ch//band7
#endif
};

const int eq_table_2[]={
	//flt_id, type, onoff, gain, fc, q
	1, 1, 1, 600, 60, 200,eq_set_ch, //band1
	2, 0, 1, 600, 150, 200,eq_set_ch, //band2
	3, 0, 1, 0, 350, 200,eq_set_ch, //band3
	4, 0, 1, 200, 700, 200, eq_set_ch,//band4
	5, 0, 1, 400, 1800, 200, eq_set_ch,//band5
	6, 0, 1, 600, 3500, 200, eq_set_ch,//band6
	7, 2, 1, 400, 7000, 200, eq_set_ch//band7
#if 0
	//flt_id, type, onoff, gain, fc, q
	1, 0, 1, -1000, 100, 100, eq_set_ch,//band1
	2, 0, 1, -800, 300, 100, eq_set_ch,//band2
	3, 0, 1, 0, 1000, 100, eq_set_ch,//band3
	4, 0, 1, -800, 3000, 300, eq_set_ch,//band4
	5, 0, 1, -1200, 8000, 300, eq_set_ch,//band5
	6, 0, 1, 0, 8000, 200, eq_set_ch,//band6
	7, 0, 1, 0, 10000, 200, eq_set_ch//band7
#endif
};

const int eq_table_3[]={
	//flt_id, type, onoff, gain, fc, q
	1, 1, 1, 0, 60, 200, eq_set_ch,//band1
	2, 0, 1, 0, 150, 200,eq_set_ch, //band2
	3, 0, 1, 400, 350, 200,eq_set_ch, //band3
	4, 0, 1, 600, 700, 200, eq_set_ch,//band4
	5, 0, 1, 400, 1800, 200, eq_set_ch,//band5
	6, 0, 1, 0, 3500, 200, eq_set_ch,//band6
	7, 2, 1, 0, 10000, 200, eq_set_ch//band7
#if 0
	//flt_id, type, onoff, gain, fc, q, ch
	1, 0, 1, -1200, 100, 100, eq_set_ch,//band1
	2, 0, 1, -800,  300, 100, eq_set_ch,//band2
	3, 0, 1, -100,  1000, 300, eq_set_ch,//band3
	4, 0, 1, 0,     3000, 300, eq_set_ch,//band4
	5, 0, 1, -800,  8000, 300, eq_set_ch,//band5
	6, 0, 1, 0,     8000, 200, eq_set_ch,//band6
	7, 0, 1, 0, 	10000, 200, eq_set_ch//band7
#endif
};
#else
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

#endif

void ui_handle_eq_music(void)
{
	int i,j;
	char set_eq_text[64];
	swa_audio_audproc_load(AUDPROC_LIST_MIX, AUDPROC_EQ);
	usleep(10);
	for (i = 0, j = 0; i < 5; i++, j += 7)
	{
		swa_audio_audproc_eq(AUDPROC_LIST_MIX, (ae_eq_para *)&eq_table_1[j]);
		swa_audio_audproc_set(AUDPROC_LIST_MIX, AUDPROC_EQ);
	}
	is_volume_mode = true;
	wdt_enter_times = 0;
	ui_goback_source(900);
}

void ui_handle_eq_movie(void)
{
	int i,j;
	char set_eq_text[64];
	swa_audio_audproc_load(AUDPROC_LIST_MIX, AUDPROC_EQ);
	usleep(10);
	for (i = 0, j = 0; i < 5; i++, j += 7)
	{
		swa_audio_audproc_eq(AUDPROC_LIST_MIX, (ae_eq_para *)&eq_table_2[j]);
		swa_audio_audproc_set(AUDPROC_LIST_MIX, AUDPROC_EQ);
	}
//	ui_led_disp_state(3);
	is_volume_mode = true;
	wdt_enter_times = 0;
	ui_goback_source(900);
}

void ui_handle_eq_dialog(void)
{
	int i,j;
	char set_eq_text[64];
	swa_audio_audproc_load(AUDPROC_LIST_MIX, AUDPROC_EQ);
	usleep(10);
	for (i = 0, j = 0; i < 5; i++, j += 7)
	{
		swa_audio_audproc_eq(AUDPROC_LIST_MIX, (ae_eq_para *)&eq_table_3[j]);
		swa_audio_audproc_set(AUDPROC_LIST_MIX, AUDPROC_EQ);
	}
//	ui_led_disp_state(4);
	is_volume_mode = true;
	wdt_enter_times = 0;
	ui_goback_source(900);
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
static int recorddatacnt=0;
static int dispdelaycnt=0;
static bool flashcol=false;
void ui_update_music_time(void)
{
    if(((is_usb_load && ui_source_select == SOURCE_SELECT_USB) ||
        (is_sd_load && ui_source_select == SOURCE_SELECT_SD)) &&
        ui_power == POWER_ON )
    {
		static bool showed = true;
		ui_info_t player_info;
		//获取播放器信息
		player_get_info(&player_info);

		if (player_info.player_stat == 2 ||
		   (player_info.player_stat == 3 && !showed))
		{   //播放状态或暂停显示隐藏状态
		   int * seek_time_p = (ui_source_select == SOURCE_SELECT_USB?&usb_playtime:&sd_playtime);
		   int curtime = player_info.curtime;
		   //防止seek前的声音进行的静音处理
		   if(curtime < *seek_time_p)
		   {
			   curtime = *seek_time_p;
		   }
		   else
		   {
			   if (curtime < *seek_time_p + 5)
			   {
			   	   *seek_time_p = curtime;
		   		   if (++recorddatacnt >= 2)
				   {
					   recorddatacnt = 0;
					   //将参数写入
//					   if(update_params_2_eeprom())
//					   {
//						   //update_params_from_eeprom();
//					   }
//					   else
//					   {
//						   printf("write params to EEPROM fail\n");
//					   }
				   }
			   }
		   }

		   if (!is_volume_mode && mute_state != MUTE && !stop_state)
		   {
//           	   ui_led_disp_music_time(curtime);
//			   if (flashcol == false) ui_led_disp_col2(1);
//			   else ui_led_disp_col2(0);
			   flashcol = flashcol?0:1;
//			   if (showed==false) ui_led_disp_col2(1);
			   showed = true;
//			   ui_led_update_display();
		   	}
		}
		else if (player_info.player_stat == 3 && showed)
		{   //暂停显示状态
		   showed = false;
//		   ui_led_clear();
//		   ui_led_update_display();
		}
    }

}


#define AUDIO_ENERGY_DEBUG

#ifdef AUDIO_ENERGY_DEBUG
int energy[6];
int det_energy_cnt=0;
bool is_det_need_mute = true;
#endif

void get_energy_init(void)
{
#ifdef AUDIO_ENERGY_DEBUG
	int err = swa_audio_audproc_load(AUDPROC_LIST_MIX, AUDPROC_ENERGY);
	printf("swa_audio_audproc_load: %d \n", err);
	err = swa_audio_audproc_energy(AUDPROC_LIST_MIX, 30);
	printf("swa_audio_audproc_energy :%d \n", err);
#endif
}

void ui_source_detect_handle(void)
{
	if (ui_power == POWER_ON && ui_source_select != SOURCE_SELECT_BT)
	{
#ifdef AUDIO_ENERGY_DEBUG
			swa_audio_audproc_get_energy(AUDPROC_LIST_MIX, &energy);
			printf("swa_audio_audproc_get_energy %d %d \n", energy[0], energy[1]);
#endif
			if (energy[0] > 500 || energy[1] > 500)
			{
				if (++det_energy_cnt >= 50)
				{
					det_energy_cnt = 0;
					is_det_need_mute = false;
				}
			}
			else
			{
				det_energy_cnt = 0;
				is_det_need_mute = true;
			}
			if (mute_state == UNMUTE)
			{
				if (!is_det_need_mute)
				{
					//Hw_Amp_UnMute();
				}
				else	//鏃犻煶婧愭椂闈欓煶
				{
					//Hw_Amp_Mute();
				}
			}
	}
}


/****************************************************************************
 * Name: update_params_2_flash
 *
 * Description:
 *    锟斤拷锟斤拷锟斤拷写锟斤拷flash
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
bool update_params_2_flash(void)
{
    bool ret = false;
    int buf[0x100] = {1, 0,  (int)set_vol};
    yunos_bsp_flash_erase(FLASH_USER_SPACE_ADDR, 0xd8);
    int i = 0;
    for(i=0;i<2;++i)
    {
        if(zhuque_bsp_flash_page_program(FLASH_USER_SPACE_ADDR, sizeof(buf), (int)buf) == 0)
        {
            break;
        }
    }

    if (i < 2)
    {
        ret = true;
    }
    return ret;

}


/****************************************************************************
 * Name: update_params_from_flash
 *
 * Description:
 *    锟斤拷flash锟斤拷取锟斤拷锟斤拷
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
bool update_params_from_flash(void)
{
    bool ret = false;
    int i;
    int buf[0x100] = {0};
    int length = sizeof(buf);
    zhuque_bsp_flash_read(FLASH_USER_SPACE_ADDR, length, (unsigned char *)buf);
	printf("%s read buf\n", __func__);

    /*
    for(i=0;i<20;++i)
    {
       printf(" buf[%d]=%x ", i, buf[i]);
    }
    */

    if(buf[0] == 1)
    {
    	if (buf[1] >= SOURCE_SELECT_START && buf[1] < SOURCE_SELECT_END)
    	{
        	//ui_source_select = buf[1];
    	}
		else
		{
			//ui_source_select = 0;
		}

    	if (buf[2] >= 0 && buf[2] <= 16)
    	{
        	set_vol = buf[2];
			mix_vol = set_vol * (100.0f/40) ;
    	}
		else
		{
			set_vol = 8;
		}

		printf("------>>>>%s, set_vol:%d \n", __func__, set_vol);
		ret = true;
    }
    else
    {
        if(update_params_2_flash())
        {
            printf("write params 2 flash success\n");
        }
    }
    return ret;


}


void ui_handle_set_volume(int volume)
{
	set_vol = volume;
    mix_vol = volume * (100.0f/40) ;
	player_process_cmd(NP_CMD_VOLUME_SET, NULL, (int)mix_vol, NULL, NULL);

	if (mute_state == MUTE)
    {
        ui_handle_mute();
    }

	printf("%s set vol %d", __func__, volume);
}

void ui_handle_send_volume2phone(void)
{
	sem_wait(&bt_state_sem);
	handle_bt_send_volume(set_vol);
	sem_post(&bt_state_sem);
}

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
	//int value;

	int cec_cmd_gpio = 11;  //GPIO1_  change

	printf("cec_cmd_gpio=%d\r\n",cec_cmd_gpio);

	//value = REG32(KSEG1(SILAN_PADMUX_CTRL));
	//printf("%s:SILAN_PADMUX_CTRL=0x%x\n",  __func__,value);
	//value = REG32(KSEG1(SILAN_PADMUX_CTRL2));
	//printf("%s:SILAN_PADMUX_CTRL2=0x%x\n",	__func__,value);

	cec_process_cmd(CEC_CMD_ARC_INIT,&cec_cmd_gpio);
	usleep(1000);
	cec_process_cmd(CEC_CMD_ARCON, NULL);
	usleep(1000);
	#if 0
	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
	usleep(1000);
	player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
	usleep(1000);
	#endif
	//player_process_cmd(NP_CMD_SPDIFIN_START, NULL, 0, NULL, NULL);
	return SL_UI_ERROR_NULL;
}

int action_hdmi_off(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

	cec_process_cmd(CEC_CMD_ARCOFF, NULL);
	usleep(1000);
	//player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);

	return SL_UI_ERROR_NULL;
}
int action_hdmi_standby(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
	cec_process_cmd(CEC_CMD_STANDBY, NULL);
	usleep(1000);
	player_process_cmd(NP_CMD_SPDIFIN_STOP, NULL, 0, NULL, NULL);
	return SL_UI_ERROR_NULL;
}
int action_hdmi_poweron(void)
{
#ifdef SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

	//cec_process_cmd(CEC_CMD_ARCON, NULL);
	//usleep(1000);
	cec_process_cmd(CEC_CMD_POWERON, NULL);
	usleep(1000);

	return SL_UI_ERROR_NULL;
}



int sl_ui_handle_cec_inactive_source( void)
{
#if 1//def SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif

#if 0
	ui_power = POWER_OFF;

	show_led_off();

	Hw_Amp_Mute();

	player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
	usleep(100);

	sem_wait(&bt_state_sem);
	handle_bt_cmd(PWR_OFF);
	sem_post(&bt_state_sem);

#endif
	return 	 SL_UI_ERROR_NULL;
}

int sl_ui_handle_cec_active_source( void)
{
#if 1//def SL_UI_DBG
	printf("%s %d\n", __func__, __LINE__);
#endif
	action_hdmi_on();
	//set_sl_ui_cmd(NP_CMD_STOP, NULL, 0);
	return 	-SL_UI_ERROR_NO_DIALOG;
}

void hdmi_poweroff_check(void)
{
	hdmi_poweroff_cnt++;
	if(hdmi_poweroff_cnt == 500)
	{
		sem_wait(&bt_state_sem);
		handle_bt_cmd(PWR_OFF);
		sem_post(&bt_state_sem);
	}
}

#endif




