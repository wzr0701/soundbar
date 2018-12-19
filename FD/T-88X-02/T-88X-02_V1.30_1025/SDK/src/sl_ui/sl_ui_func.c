#include <nuttx/clock.h>
#include <nuttx/module.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <debug.h>
#include <arch/board/board.h>
#include <poll.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <nuttx/input.h>
#include <stdlib.h>
#include <fcntl.h>
#include <silan_adc.h>
#include "input_key.h"
#include "nuttx/wqueue.h"
#include "queue.h"
#include "sl_ui_touch_key.h"
#include "sl_ui_cmd.h"


#define SC6138_FUNC_TIMEOUT   5    //50ms

struct s_func {
    WDOG_ID wdog_id;
    int level_value;
    int level_detect_number;
    int ch_num;
    int key_num;
    int fd;
    pid_t adc_pid;
};

static struct s_func func_priv;
extern int tre_bass_cnt;
extern int bt_wait_cnt;
extern int auto_input_cnt;
extern int save_ir_cnt;
extern int fm_manual_save_cnt;
extern int usb_play_cnt;
extern int  mix_vol;
extern struct input_event save_ir_event;
extern bool enter_tre_set;
extern bool enter_bass_set;
extern bool ir_short_flag;
extern bool bt_wait_flag;


static void func_add_detect(int argc, uint32_t irq_num, ...)
{
	ui_cmd_t cmd;

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


	if(usb_play_cnt == 30)
	{
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, mix_vol, NULL, NULL);
	}

	enter_testmode_check();

	wd_start(func_priv.wdog_id, SC6138_FUNC_TIMEOUT, func_add_detect, 1, 0);
}



static int func_add_main(int argc, FAR char *argv[])
{
	int ret;
	func_priv.wdog_id = wd_create();
    ret = wd_start(func_priv.wdog_id, SC6138_FUNC_TIMEOUT, func_add_detect, 1, 0);
    while (1)
		usleep(10000);

    return 0;
}


int sl_func_add(void)
{
    func_priv.adc_pid = task_create("func_add", 5, 8192, func_add_main, NULL);
    if (func_priv.adc_pid < 0)
	{
        LOGE("FUNC", "Failed to start task (%d)", errno);
        return -errno;
    }

    return OK;
}


