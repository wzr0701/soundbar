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
//#include <sc6138_led7.h>
#include "silan_resources.h"
#include "silan_addrspace.h"
#include <zhuque_bsp_gpio.h>
#include "ht1633.h"
#include "sl_ui_io_led.h"
#include "sl_ui_touch_key.h"
#include "sl_ui_cmd_deal.h"
#include "sl_ui_fm.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*电台搜索间隔*/
#define RADIO_VALID_COUNT 5
/*调试信息打印开关*/
#define SL_UI_DBG 1
/*休眠计时*/
#define SLEEP_COUNT 50
#define SLEEP_COUNT1 100

/*Check sum检查开关*/
//#define SLUI_MEM_CHECK 1

/****************************************************************************
 * Public Types
 ****************************************************************************/
/*执行函数类型定义*/
typedef void (*exec_func_0)(void);
typedef void (*exec_func_1)(int arg1);
typedef void (*exec_func_2)(int arg1, int arg2);
typedef void (*exec_func_3)(int arg1, int arg2, int arg3);
/*处理信息*/
typedef struct {
    const int cmd;
    const void * func;
    const char * str;
    const int argn;
} HANDLE_INFO;

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
struct ui_s *Ui = NULL;

//全局变量
/*输入声源选择*/
int ui_source_select = -1;


//外部变量
/*声音显示延迟显示看们狗ID*/
extern WDOG_ID wdtimer_source;

bool usb_online=0;
bool sd_online=0;
bool aux_online=0;
extern  char dis_other_mode;
sem_t de_i2c_sem;
unsigned char g_FMType_RX;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/*adckey按键初始化初始化*/
extern int sc6138_adcinitialize(void);

/****************************************************************************
 * Public Functions
 ****************************************************************************/





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
static int sl_ui_thread(int argc, char **argv)
{
	struct mq_attr attr;
	int msgsize, prio;
	int count = 0;
	int count1 = 0;
	bool running = true;
	ui_cmd_t cmdq;
	int ret = 0;
	int handle_wd_delay;

	///////////////////////////////////
	padmux_init();
	pa_mute_ctrl(true);
	/////////////////////////////////////

	//DSP同步
	do
	{
		ret = swa_audio_decode_init();
		usleep(1);
	}
	while(ret == -1);
	ret = 0;
	//printf("DSP sync finish\n");
	//引脚功能初始化
	//printf("#############################################20180929\n");
#ifdef CONFIG_CEC
	cec_entry();
#endif
	///////////////////////////////////
	pa_io_ret_set(false);
	bt_power_crt(false);
	//aux_fm_channel_choose(true);
	pcm1803_power_crt(true);
	///////////////////////////////
	touch_key_int();
	sc6138_adcinitialize();
	//////////////////////////////
	ht1633_init();
	usleep(1000);
	display_ui_power(1);

	g_FMType_RX = fm_rx_ID();   //����ID�ж�FM����оƬ����
	PCM5242_Init();

	//sem_init(&de_i2c_sem, 0, 1);

	////////////////////////////////////////////////////

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
		{
			//消息队列打开失败
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

			//执行开机动作
			ui_handle_power(POWER_ON);

			//UI主循环
			while (running)
			{
				switch(ui_source_select)
				{
					case SOURCE_SELECT_BT:
						source_mode_bt();
						break;

					case SOURCE_SELECT_USB:
						source_mode_usb();
						break;

					case SOURCE_SELECT_SD:
						source_mode_sd();
						break;

					case SOURCE_SELECT_FM:
						source_mode_fm();
						break;

					case SOURCE_SELECT_LINEIN:
						source_mode_aux();
						break;

					case SOURCE_SELECT_SPDIFIN:
						source_mode_spdifin();
						break;

					case SOURCE_SELECT_HDMI:
						source_mode_hdmi();
						break;

					case SOURCE_SELECT_COA:
						source_mode_coaxial();
						break;

					case SOURCE_SELECT_TEST:
						source_mode_test();
						break;

					case SOURCE_SELECT_START:
						source_mode_start();////////poser_off
						break;

				}

			}
			//关闭消息队列
			mq_close(Ui->mq);
		}
		//释放分配的内存
		free(Ui);
	}
	return ret;
}


/**************************************************


**************************************************/
ui_cmd_t  get_mq_msg(void)
{
	struct mq_attr attr;
	int msgsize, prio;
	ui_cmd_t cmdq;
	msgsize = mq_receive(Ui->mq, (char *)&cmdq, sizeof(ui_cmd_t), &prio);
	if (msgsize == sizeof(ui_cmd_t))
	{
		// printf("cmdq.cmd==%d\r\n",cmdq.cmd);
	}
	else
	{
		cmdq.cmd= UI_CMD_NULL;
	}
	return cmdq ;
}
/**************************************************



**************************************************/
void  put_ui_msg(int ui_cmd)
{
	ui_cmd_t cmd;
	cmd.cmd = ui_cmd;
	send_cmd_2_ui(&cmd);
}

/*******************************************************



*******************************************************/
void mq_msg_clear(void)
{
	unsigned char i=0;
	ui_cmd_t cmdq;
	for(i=0;i<64;i++)
	{
		cmdq=get_mq_msg();
	}
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
	return 	SL_UI_ERROR_NULL;
}

//设置开机执行ui进程启动程序
app_entry(sl_ui_main);
