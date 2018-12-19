/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "bt_uart.h"
#include <fcntl.h>
#include <nuttx/module.h>
#include <nuttx/audio/silan_audio_api.h>
#include <nxplayer.h>
#include <mqueue.h>
#include <player_cmd.h>
#include "sl_lcd.h"
#include "sl_ui_cmd.h"
#include "sl_ui_handle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef CONFIG_SYSTEM_AIRPLAY
#include "swa_airplay_api.h"
#endif
#include <termios.h>
#include "vol_ctrl.h"
#include "silan_resources.h"
#include "silan_addrspace.h"
#include <nuttx/audio/sladsp_ae.h>
#include "sl_bsp.h"
#include <silan_adc.h>
#include "vol_ctrl.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*调试信息打印开关*/
#define SL_UI_DBG
/*休眠计时*/
#define SLEEP_COUNT 20
#define UI_UPDATE_COUT 5
/*Check sum检查开关*/
//#define SLUI_MEM_CHECK 1
#define ADC_DETECT_COUT 1
#define EN_ADC_COUT		20
#define TIME_COUNT_100MS  10
/****************************************************************************
 * Public Types
 ****************************************************************************/
/*执行函数类型定义*/
typedef void (exec_func_t)(void);

/****************************************************************************
 * Public Data
 ****************************************************************************/
//文件局部变量
#ifdef SLUI_MEM_CHECK
static int mem_cnt = 0;
#endif
/*重复按键控制标志*/
static bool should_exec = true;
/*延迟执行看门狗ID*/
static WDOG_ID wdtimer_exec;
/*ui进程信息*/
static struct ui_s *Ui = NULL;

//全局变量
/*输入声源选择*/
int ui_source_select = -1;
int ui_source_select_temp = -1;
int ui_source_ir_select = -1;
/* 使能ADC检测 待系统稳定后开启检测*/
bool en_adc_detect_flag = false;
bool hdmi_det_flag = false;

//外部变量
/**/
#ifdef CONFIG_PM
extern pthread_cond_t stop_cond;
#endif
/*声音显示延迟显示看们狗ID*/
extern WDOG_ID wdtimer_source;
bool is_usb_load=false;
bool is_sd_load=false;
/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/*lcd驱动初始化*/
extern int lcd_main(void);
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: sc6138_exec_handle
 *
 * Description:
 *    清除执行计时
 *
 * Parameters:
 *    argc  参数个数
 *    arg1  参数列表
 *    ....  不定参数列表
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void sc6138_exec_handle(int argc, wdparm_t arg1, ...)
{
    should_exec = true;
    wd_cancel(wdtimer_exec);
}

/****************************************************************************
 * Name: ui_check_on_off_before_exec
 *
 * Description:
 *    检查是否处于开机状态，并执行对应方法
 *
 * Parameters:
 *    func 要执行的函数的指针
 *    func_name 要执行的函数的名字
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void ui_check_on_off_before_exec(exec_func_t func, const char * func_name)
{
    if(ui_power == POWER_ON)
    {
        if(NULL != func)
        {
            func();
        }
        else
        {
            printf("%s:No function to be execute\n", __func__);
        }
    }
    else
    {
        printf("%s:Is power off\n", (NULL != func_name)?func_name:__func__);
    }
}

/****************************************************************************
 * Name: ui_handle_cmd
 *
 * Description:
 *    UI命令处理函数
 *
 * Parameters:
 *    cmd  要处理的UI命令
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void ui_handle_cmd(ui_cmd_t *cmd)
{
    printf("-------------------- %s:cmd:%d\n", __func__, cmd->cmd);

#ifdef SL_UI_DBG
    printf("%s:ui_power:%d\n", __func__, ui_power);
#endif

    switch (cmd->cmd)
    {
    case UI_CMD_STOP:
#ifdef CONFIG_PM
        pthread_cond_signal(&stop_cond);
#endif
        break;
    case UI_CMD_PLAYER_TONE_FINISH:
        break;
    case UI_CMD_BUFFERING:
        break;
    case UI_CMD_BT_UP:
        ui_check_on_off_before_exec(ui_handle_up, "ui_handle_up");
        break;
    case UI_CMD_BT_DOWN:
        ui_check_on_off_before_exec(ui_handle_down, "ui_handle_down");
        break;
    case UI_CMD_PLAYER_FINISH:
        ui_handle_finish();
		break;
    case UI_CMD_BT_NEXT:
        ui_check_on_off_before_exec(ui_handle_next, "ui_handle_next");
        break;
	case UI_CMD_BT_NEXT_LONG_DOWN:
        ui_check_on_off_before_exec(ui_handle_next_long_pressdown, "ui_handle_next_long_pressdown");
		break;
	case UI_CMD_BT_PREV_LONG_DOWN:
        ui_check_on_off_before_exec(ui_handle_prev_long_pressdown, "ui_handle_prev_long_pressdown");
		break;
    case UI_CMD_BT_PREV:
        ui_check_on_off_before_exec(ui_handle_prev, "ui_handle_prev");
        break;
    case UI_CMD_BT_OK:
        ui_check_on_off_before_exec(ui_handle_pause_play, "ui_handle_pause_play");
        break;
    case UI_CMD_BT_PLAY_LONG_DOWN:
        //ui_check_on_off_before_exec(ui_handle_fm_seek_auto_init, "ui_handle_fm_seek_auto_init");
        break;
    case UI_CMD_BT_PLAY_LONG_UP:
        //ui_check_on_off_before_exec(ui_handle_fm_seek_auto, "ui_handle_fm_seek_auto");
        break;
	case UI_CMD_BT_STOP:
		ui_check_on_off_before_exec(ui_handle_stopplay, "ui_handle_stopplay");
		break;
    case UI_CMD_BT_MODE:
        ui_check_on_off_before_exec(ui_handle_mode, "ui_handle_mode");
        break;
    case UI_CMD_BT_POWER:
        ui_handle_power();
        break;
    case UI_CMD_BT_MUTE:
        ui_check_on_off_before_exec(ui_handle_mute, "ui_handle_mute");
        break;
    case UI_CMD_VOLUME_INC_DOWN:
        ui_check_on_off_before_exec(ui_handle_vol_inc_long_press, "ui_handle_vol_inc_long_press");
        break;
    case UI_CMD_VOLUME_INC_UP:
        ui_check_on_off_before_exec(ui_handle_vol_long_press_up, "ui_handle_vol_long_press_up");
        break;
    case UI_CMD_VOLUME_DEC_DOWN:
        ui_check_on_off_before_exec(ui_handle_vol_dec_long_press, "ui_handle_vol_dec_long_press");
        break;
    case UI_CMD_VOLUME_DEC_UP:
        ui_check_on_off_before_exec(ui_handle_vol_long_press_up, "ui_handle_vol_long_press_up");
        break;
	case UI_CMD_BT_PAIR:
		if (ui_source_select != SOURCE_SELECT_BT)
		{
			break;
		}
		bt_connected = false;
        bt_play_status = false;
		bt_force_disconnect = true;
		printf("bt force disconnect !\n");
	case UI_CMD_IR_BT_MODE:
		ui_source_ir_select = SOURCE_SELECT_BT;
		ui_handle_mode();
		break;
	case UI_CMD_IR_OPT_MODE:
		ui_source_ir_select = SOURCE_SELECT_SPDIFIN;
		ui_handle_mode();
		break;
	case UI_CMD_IR_AUX_MODE:
        if(aux_in_status == true)
        {
            ui_source_ir_select = SOURCE_SELECT_LINEIN;
		    ui_handle_mode();
        }
		break;
	case UI_CMD_IR_RCA_MODE:
		ui_source_ir_select = SOURCE_SELECT_LINEIN1;
		ui_handle_mode();
		break;
	case UI_CMD_IR_HDMI_MODE:
		if (hdmi_det_online == 1)
		{
			ui_source_ir_select = SOURCE_SELECT_HDMI;
			ui_handle_mode();
		}
		break;
	case UI_CMD_BT_SET_VOLUME:
		ui_handle_set_volume(cmd->arg2);
		break;
	case UI_CMD_BT_SEND_VOLUME:
		ui_check_on_off_before_exec(ui_handle_send_volume2phone, "ui_handle_send_volume2phone");
		break;
	case UI_CMD_EQ_MUSIC:
		ui_check_on_off_before_exec(ui_handle_eq_music, "ui_handle_eq_music");
		break;
	case UI_CMD_EQ_MOVIE:
		ui_check_on_off_before_exec(ui_handle_eq_movie, "ui_handle_eq_movie");
		break;
	case UI_CMD_EQ_DIALOG:
		ui_check_on_off_before_exec(ui_handle_eq_dialog, "ui_handle_eq_dialog");
		break;
    case UI_CMD_USB_LOAD:
        //ui_check_on_off_before_exec(ui_handle_load_usb, "ui_handle_load_usb");
        break;
    case UI_CMD_USB_OUT:
        //ui_check_on_off_before_exec(ui_handle_usb_out, "ui_handle_usb_out");
        break;
    case UI_CMD_SD_IN:
	case UI_CMD_SD_LOAD:
		//ui_check_on_off_before_exec(ui_handle_load_sd, "ui_handle_load_sd");
		break;
    case UI_CMD_SD_OUT:
    case UI_CMD_SD_UNLOAD:
        //ui_check_on_off_before_exec(ui_handle_sd_unload, "ui_handle_sd_unload");
        break;
    case UI_CMD_FILES_IS_LOAD:
        //if(ui_power == POWER_ON)
        //{   //开机模式
        //    ui_handle_file_load(cmd->arg2, cmd->mode);
        //}
        break;
	case UI_CMD_HDMI_CONNECT:
			//if((ui_source_select!=SOURCE_SELECT_HDMI)||(hdmi_det_flag == false))
			{
				//hdmi_det_flag = true;
				ui_source_ir_select = SOURCE_SELECT_HDMI;
				ui_handle_mode();
			}
	   break;


       case UI_CMD_HDMI_DISCONNECT:
	   		//hdmi_det_flag = false;
			//if(ui_source_select == SOURCE_SELECT_HDMI)
			//{
			//	ui_source_ir_select = SOURCE_SELECT_BT;
			//	ui_handle_mode();
			//}
	   	break;

		case UI_CMD_HDMION_SEND:
			action_hdmion_send();
	   break;

	   case UI_CMD_HDMION_DEINIT:
			action_hdmi_on();
	   break;

		case UI_CEC_INACTIVE_SOURCE:
			sl_ui_handle_cec_inactive_source();
	   	break;

		case UI_CEC_ACTIVE_SOURCE:
			//sl_ui_handle_cec_active_source();
	   	break;

		case UI_CMD_SET_48K:
			ui_handle_set_samp(true);
		break;

		case UI_CMD_SET_44K:
			ui_handle_set_samp(false);
		break;

		case UI_CMD_PLAYSTA_MUTE:
			bt_playsta_mute();
	   	break;



    default:
        break;
    }
}

/****************************************************************************
 * Name: ui_thread
 *
 * Description:
 *    UI进程主函数
 *
 * Parameters:
 *    argc  参数个数
 *    argv  参数列表
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
int adc_fd;
unsigned short Adc_Read_Buf[6];
static int sl_ui_thread(void)
{
    struct mq_attr attr;
    int msgsize, prio;
    int count = 0;
	int cxmode_count = 0;
	int uicount = 0;
	int bt_mute_cnt=0;
	int mci_count=0;
	int delay_disp_cout=0;
    bool running = true;
    ui_cmd_t cmdq;
    int ret = 0;
    int handle_wd_delay;
	bool cplfalg = false;
	int energy_old = 0;
	int energy_cur = 0;
	int energy_temp = 0;
	usleep(1000000);

	//等待DSP初始化完成
	sync_dsp_startup();

#ifdef CONFIG_CEC
	cec_entry();
#endif

	bsp_init();
	get_energy_init();
	//打开消息队列
    Ui = (FAR struct ui_s *)malloc(sizeof(struct ui_s));
    if (Ui == NULL)
    {
        printf("%s:Allocate memery for UI error\n",__func__);
    }
    else
    {
        printf("%s:Allocate memery for UI success\n",__func__);

        memset(Ui, 0, sizeof(struct ui_s));

        //设置消息队列信息
        attr.mq_maxmsg = 64;
        attr.mq_msgsize = sizeof(ui_cmd_t);
        attr.mq_curmsgs = 0;
        attr.mq_flags = 0;
        sprintf(Ui->mqname, "mq_common");
        //打开消息队列
        Ui->mq = mq_open(Ui->mqname, O_RDWR | O_CREAT | O_NONBLOCK, 0644, &attr);
        if (Ui->mq < 0)
        { //消息队列打开失败
            printf("%s:Open message queue for UI fail\n", __func__);
        }
        else
        {
            printf("%s:Open message queue for UI success\n", __func__);

            if(bt_uart_init())
            {
                printf("%s:UART for bluetooth open success\n", __func__);
            }
            else
            {
                printf("%s:UART for bluetooth open fail\n", __func__);
            }

            //初始化蓝牙同步信号量和状态获取线程初始化
            bt_init_sem();
			//hdmi
        	//set_hdmi_gain(0);
            //执行开机动作
            ui_handle_power();

			//UI主循环
            while (running)
            {
                //从消息队列读取消息
                msgsize = mq_receive(Ui->mq, (char *)&cmdq, sizeof(ui_cmd_t), &prio);
				//printf("size %d msg %d \n", msgsize, cmdq.cmd);
                if (msgsize == sizeof(ui_cmd_t))
                {   //有效消息
                    if ((cmdq.cmd == UI_CMD_BT_MODE) || (cmdq.cmd == UI_CMD_BT_POWER))
                    {   //需要重复保护的按键
                        if(should_exec)
                        {   //非重复按键保护状态
                            if (cmdq.cmd == UI_CMD_BT_MODE)
                            {
                                handle_wd_delay = 200;
                            }
							else if (cmdq.cmd == UI_CMD_BT_POWER)
							{
                                handle_wd_delay = 800;
							}
                            else
                            {
                                handle_wd_delay = 150;
                            }

                            if (wdtimer_exec != NULL)
                            {
                                wd_cancel(wdtimer_exec);
                            }
                            else
                            {
                                wdtimer_exec = wd_create();
                            }

                            should_exec = false;
                            wd_start(wdtimer_exec, handle_wd_delay, sc6138_exec_handle, 0);
                            //执行命令
                            ui_handle_cmd(&cmdq);
                        }
                        else
                        {
                            printf("%s:cmd %d is be ignored\n", __func__, cmdq.cmd);
                        }
                    }
                    else
                    {   //不需要出发重复保护的消息且处于开机状态
                        ui_handle_cmd(&cmdq);
                    }
                }
                else if(msgsize >= 0)
                {   //消息大小错误
                    printf("%s:Message size error\n", __func__);
                }

                if (++count >= SLEEP_COUNT)
                {
                    //检查蓝牙连接状态并处对应显示
                    bt_chk_and_disp();
					//更新SB和SD状态
					//chk_usb_sd_disp();
                    //更新播放时间显示
                    //ui_update_music_time();
					count = 0;
                }
				if (++uicount >= UI_UPDATE_COUT)
				{
					//ui_source_detect_handle();
					uicount = 0;
				}
				//ui_bt_mute_handle();
                usleep(6000);
            }
            //关闭消息队列
            mq_close(Ui->mq);
        }
        //释放分配的内存
        free(Ui);
    }
    return ret;
}

/****************************************************************************
 * Name: ui_main
 *
 * Description:
 *    启动UI进程
 *
 * Parameters:
 *    argc  参数个数
 *    argv  参数列表
 *
 * Returned Value:
 *    总是0
 *
 * Assumptions:
 *
 ****************************************************************************/
#include <nuttx/module.h>
int sl_ui_main(int argc, char *argv)
{
	int ret;
	ret = task_create("sl_ui", 100, 16384, sl_ui_thread, NULL);
	if (ret <= 0) {
		printf("%s %d task_create failed!\n", __func__, __LINE__);
		return SL_UI_ERROR_INIT;
	}
	return	SL_UI_ERROR_NULL;
}
//设置开机执行ui进程启动程序
app_entry(sl_ui_main);



int sl_lcd_main(void)
{
}
