/****************************************************************************
 * hardware/id.bsp/configs/sc6138a-xj-1/src/sc6138_adckey.c
 *
 * Copyright (C) 2015 The YunOS Open Source Project
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *      http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
 *
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/arch.h>
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

#if 0//def CONFIG_BOARD_SHENZHEN
#include "silan_addrspace.h"
#include "silan_resources.h"
#endif
#define KEY_DEBUG
#ifdef KEY_DEBUG
//#define PRINTF(fmt, arg...) LOGD("key", fmt, ##arg)
#define PRINTF printf
#else
#define PRINTF(fmt, arg...)
#endif

#ifdef CONFIG_BOARD_SHENZHEN
#define ADC_CHANNEL 0x03
#else
#define ADC_CHANNEL 0x00
#endif
#define SC6138_ADCKEY_TIMEOUT   1    //10ms
#define KEY_RANGE(a, b) (((a > b - 100) && (a < b + 100))? 1 : 0)

struct s_key {
    WDOG_ID wdog_id;
    int level_value;
    int level_detect_number;
    int ch_num;
    int key_num;
    int fd;
    pid_t adc_pid;
};
static struct s_key key_priv;
#if CONFIG_CHIP_SC6138
#ifdef CONFIG_BOARD_SHENZHEN
static int key_value_tbl[] = {0x7, 0x95, 0x14a, 0x28a, 0x380, 0x51b, 0x744, 0xa85};
#else
static int key_value_tbl[] = {0x17b, 0x328, 0x4d2, 0x61e, 0x7ba, 0x929, 0xac9, 0xc1b};
#endif
#elif CONFIG_CHIP_AP1508
#ifdef CONFIG_BOARD_AP_8835C
static int key_value_tbl[] = {0x0, 0x500, 0x6d6, 0x845, 0x2d7, 0xb45, 0x9c9, 0xdf3, 0xcc9};
#else
static int key_value_tbl[] = {0x2d0, 0x0, 0x4f0, 0x820, 0x6c0, 0xca0, 0xdc0, 0x9b0, 0xb20};
#endif
#endif


extern sem_t de_i2c_sem;
extern void button_add_event(struct input_event *event);
static void key_detect_low_level(int argc, uint32_t irq_num, ...);
static int key_find_channel(void)
{
#if(TOUCH_KEY_ENABLE==1)

	unsigned char touch_key_value;
	uint32_t value;
	zhuque_bsp_gpio_get_value(KEY_INT_PIN, &value);
	if(value)
	{
		// sem_wait(&de_i2c_sem);
		if(touch_key_read(&touch_key_value,1))
		{
			//printf("%s:touch_key_value=%x\r\n",__func__,touch_key_value);
			switch(touch_key_value)
			{
				case 0x01:
					key_priv.key_num=0;
					break;

				case 0x02:
					key_priv.key_num=1;
					break;

				case 0x04:
					key_priv.key_num=2;
					break;

				case 0x08:
					key_priv.key_num=3;
					break;

				case 0x10:
					key_priv.key_num=4;
					break;
			}

		}
		if(key_priv.key_num !=-1)
		{
			key_priv.ch_num = 0;
			return 0;
		}
	}
	else
	{
		key_priv.key_num =-1;
		key_priv.ch_num =-1;
	}

	//sem_post(&de_i2c_sem);
#else
    char *p;
    int i, j, k, m;
    unsigned short buf[ADC_MAX_CH_NUM];

    p = (char *)buf;
    memset(p, 0xff, ADC_MAX_CH_NUM*2);
    silan_adc_read(key_priv.fd, p, ADC_MAX_CH_NUM*2);
    for (i = 0; i < ADC_MAX_CH_NUM; i++) {
        j = (ADC_CHANNEL>>i)&0x01;
        if (j) {
            for (k = 0; k < ADC_MAX_CH_VAL; k++) {
                m = KEY_RANGE(buf[i], key_value_tbl[k]);
                if (m)
                {
                    key_priv.key_num = k;
                    break;
                }
            }
            if (key_priv.key_num != -1)
            {
                key_priv.ch_num = i;
                return 0;
            }
        }
    }
#endif
    return -1;
}

static int key_detect_channel(void)
{

#if(TOUCH_KEY_ENABLE==1)
    uint32_t value;
    zhuque_bsp_gpio_get_value(KEY_INT_PIN, &value);
   if(value)
     return 0;
   else
     return 1;
#else
    char *p;
    int m;
    unsigned short buf[ADC_MAX_CH_NUM];

    p = (char *)buf;
    memset(p, 0xff, ADC_MAX_CH_NUM*2);
    silan_adc_read(key_priv.fd, p, ADC_MAX_CH_NUM*2);
    m = KEY_RANGE(buf[key_priv.ch_num], key_value_tbl[key_priv.key_num]);
    if (m)
        return 0;
    else
        return 1;
#endif
}

static void key_detect_reset(void)
{
    key_priv.level_value = 0;
    key_priv.level_detect_number = 0;
    key_priv.ch_num = -1;
    key_priv.key_num = -1;
}
#if 0
//  because new black board use adc to replace the button to operate the flow control.
static int key_num_translate(int key_num,char chan)
{
	int key_ret=0;

	if(chan&0x01)
	{
		switch (key_num)
		{
			case 0:
				key_ret = UI_CMD_PLAY_PAUSE;
				break;

			case 1:
				key_ret = UI_CMD_POWER;
				break;

			case 2:
				key_ret = UI_CMD_MODE;
				break;

			case 3:
				key_ret = UI_CMD_VOLUME_INC;
				break;

			case 4:
				key_ret = UI_CMD_VOLUME_DEC;
				break;

			case 5:
				key_ret = 0;
				break;

			case 6:
				key_ret = 0;
				break;

			case 7:
				key_ret = 0;
				break;

		}

	}
	else if(chan&0x02)
	{
		switch (key_num)
		{
			case 0:
				key_ret=	0;
				break;

			case 1:
				key_ret=	0;
				break;

			case 2:
				key_ret=	0;
				break;

			case 3:
				key_ret=	0;
				break;

			case 4:
				key_ret=	0;
				break;

			case 5:
				key_ret=	0;
				break;

			case 6:
				key_ret=	0;
				break;

			case 7:
				key_ret=	0;
				break;

		}
	}


	return key_ret;

}
#else
/****************************************************************************
 * Name: key_num_translate
 *
 * Description:
 *    键值转换函数
 *
 * Parameters:
 *     key_num 按键序号
 *
 * Returned Value:
 *    0-没有找到对应的键值，非0-按键键值
 *
 * Assumptions:
 *
 ****************************************************************************/
static int key_num_translate(int key_num)
{
    return key_num;
}
#endif

#define CONFIG_INPUT_WORKDELAY 1
#define CONFIG_INPUT_WORKQUEUE HPWORK

struct work_s input_work;
struct input_event key_event;
static int input_schedule_work(int qid, struct work_s *work, worker_t worker, struct input_event *priv, uint32_t delay)
{
    int result = OK;

    sched_lock();

    do
    {
        int op_result;

        op_result = work_cancel(qid, work);
        if ((op_result != OK) && (op_result != -ENOENT))
        {
            result = op_result;
            break;
        }

        result = work_queue(qid, work, worker, priv, delay);
        if (result != OK)
        {
            break;
        }
    } while (0);

    sched_unlock();

    return result;
}

static void input_work_func(void *arg)
{
    struct input_event *event;

    event = arg;
    if (event->code == SILAN_INPUTKEY_CODE_BUTTON_UP)
        PRINTF("sc6138 adkey up 0x%x\n", event->value);
    else if (event->code == SILAN_INPUTKEY_CODE_BUTTON_DOWN)
        PRINTF("sc6138 adkey push 0x%x\n", event->value);

    button_add_event(event);
}
#if 0
static void key_detect_high_level(int argc, uint32_t irq_num, ...)
{
	int value;
	int tmp_key_num = 0;

	value = key_detect_channel();
	key_priv.level_value = key_priv.level_value << 1;
	key_priv.level_detect_number++;
	key_priv.level_value |= value;

	if(key_priv.level_detect_number==CCHIP_KEY_LONG_TIME)///89*10==800ms,hold
	{
		// printf("key_long\r\n");
		key_event.type = EV_CCHIP_KEY;
		key_event.code = CODE_KEY_CLICK;
		tmp_key_num = key_num_translate(key_priv.key_num,key_priv.ch_num);
		key_event.value = tmp_key_num|CCHIP_KEY_LONG; //key_poriv.key_num;
		button_add_event(&key_event);
		key_priv.level_detect_number=CCHIP_KEY_HOLD_START_TIME-CCHIP_KEY_HOLD_KEEP_TIME;
	}
	else if(key_priv.level_detect_number>CCHIP_KEY_HOLD_START_TIME)
	{
		//printf("key_hold\r\n");
		key_event.type = EV_CCHIP_KEY;
		key_event.code = CODE_KEY_CLICK;
		tmp_key_num = key_num_translate(key_priv.key_num,key_priv.ch_num);
		key_event.value =  tmp_key_num|CCHIP_KEY_HOLD; //key_poriv.key_num;
		button_add_event(&key_event);
		key_priv.level_detect_number=CCHIP_KEY_HOLD_START_TIME-CCHIP_KEY_HOLD_KEEP_TIME;
	}
	///////////////////////////////////////////////////////
	///////////////////////////////////////////////////////

	if((key_priv.level_value & 0xf) == 0xf)
	{
		key_event.type = EV_CCHIP_KEY;
		key_event.code = CODE_KEY_CLICK;
		tmp_key_num = key_num_translate(key_priv.key_num,key_priv.ch_num);

		if(key_priv.level_detect_number<100)
		key_event.value = tmp_key_num |CCHIP_KEY_SHORT_UP; //key_priv.key_num;
		else
		key_event.value =  tmp_key_num |CCHIP_KEY_LONG_UP; //key_priv.key_num;
		//PRINTF("sc6138 adkey up 0x%x\n", key_event.value);
		button_add_event(&key_event);

		key_detect_reset();
		(void)wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
		return;
	}

	(void)wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_high_level, 1, 0);
}

static void key_detect_low_level(int argc, uint32_t irq_num, ...)
{

    int value;
    int  tmp_key_num = 0;

    if (key_priv.ch_num == -1)
    {
        value = key_find_channel();
        if (value == -1)
        {
            (void)wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
            return;
        }
    }

    value = key_detect_channel();
    key_priv.level_value = key_priv.level_value << 1;
    key_priv.level_detect_number++;
    key_priv.level_value |= !value;


    if (key_priv.level_detect_number > 32)
    {
        key_detect_reset();
        (void)wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
        //PRINTF("key_detect_low_level failed\n");
        return;
    }

    if((key_priv.level_value & 0xf) == 0xf)
    {
        key_priv.level_value = 0;
        key_priv.level_detect_number = 0;

        key_event.type = EV_CCHIP_KEY;
        key_event.code = CODE_KEY_CLICK;
        tmp_key_num = key_num_translate(key_priv.key_num,key_priv.ch_num);
        key_event.value = tmp_key_num;//key_priv.key_num;
        //PRINTF("sc6138 adkey push 0x%x\n", key_event.value);
        button_add_event(&key_event);
       // input_schedule_work(CONFIG_INPUT_WORKQUEUE, &input_work, input_work_func, &key_event, CONFIG_INPUT_WORKDELAY);
        (void)wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_high_level, 1, 0);
        return;
    }

    (void)wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
}
#else
/****************************************************************************
 * Name: key_detect_high_level
 *
 * Description:
 *    按键高状态检测看门狗计时回调函数，检测按键的高状态
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void key_detect_high_level(int argc, uint32_t irq_num, ...)
{
    int value;
    struct input_event key_event;
    int tmp_key_num = 0;
	unsigned char touch_key_value;

    //更新按键检测状态
    if (touch_key_read(&touch_key_value,1))
    {
		//printf("%s:touch_key_value=%x\r\n",__func__,touch_key_value);
		if(touch_key_value)
		{
			value  =  0;
		}
		else
		{
			value  =  1;
		}
		//value = key_detect_channel();
	    key_priv.level_value = key_priv.level_value << 1;
	    key_priv.level_value |= value;
	    //去抖动计时
	    key_priv.level_detect_number++;

	    if ((key_priv.level_value & 0xf) == 0xf)
	    {   //去抖动成功按键释放
	        //设置事件类型
	        key_event.type = EV_KEYDRV;
	        //???e???????????
	        key_event.code = SILAN_INPUTKEY_CODE_BUTTON_UP;
	        //????????
	        tmp_key_num = key_num_translate(key_priv.key_num);
	        //设置按键值
	        key_event.value = (key_priv.ch_num << 4) | tmp_key_num;
			printf("%s:SILAN_INPUTKEY_CODE_BUTTON_UP.\n",__func__);
	        //发送按键释放消息
	        button_add_event(&key_event);
	        //按键检复位
	        key_detect_reset();
	        //启动按键低状态检测看门狗计时器
	        wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
	        return;
	    }

	}
	 //启动按键高状态检测看门狗计时器
	 wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_high_level, 1, 0);
}

/****************************************************************************
 * Name: key_detect_low_level
 *
 * Description:
 *    按键低状态检测看门狗计时回调函数，检测按键的高状态
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void key_detect_low_level(int argc, uint32_t irq_num, ...)
{
    int value;
    struct input_event key_event;
    int tmp_key_num = 0;

    if (key_priv.ch_num == -1)
    {   //无按键状态
        //检查是否有按键
        value = key_find_channel();
        if (value == -1)
        {   //没有按键
            //启动看门继续检测低状态
            wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
            return;
        }
    }

    //有按键
    //更新按键检测状态
    value = key_detect_channel();
    key_priv.level_value = key_priv.level_value << 1;
    key_priv.level_value |= !value;
    //去抖动超时计时
    key_priv.level_detect_number++;
    if (key_priv.level_detect_number > 32)
    {   //去抖动检测失败
        //按键检测复位
        key_detect_reset();
        //继续检测按键低状态
        wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
        return;
    }

    if ((key_priv.level_value & 0xf) == 0xf)
    {   //连接8次=8*2=16ms去抖动成功
        //复位检测状态
        key_priv.level_value = 0;
        //复位去抖动计时
        key_priv.level_detect_number = 0;
        //设置事件类型
        key_event.type = EV_KEYDRV;
        //???e???????????
        key_event.code = SILAN_INPUTKEY_CODE_BUTTON_DOWN;
        //??????
        tmp_key_num = key_num_translate(key_priv.key_num);
        //设置键值
        key_event.value = (key_priv.ch_num << 4) | tmp_key_num;
		printf("key_event.value:%d\n", key_event.value);
        //发送按键按键下消息
        button_add_event(&key_event);
        //启动按键高状态检测看门狗计时器
        wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_high_level, 1, 0);
        return;
    }

    //去抖动还未成功，继续检测按键低状态
    wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
}


#endif
int adc_stop(void)
{
    int ret;
    ret = wd_cancel(key_priv.wdog_id);
    if (ret == ERROR)
        printf("wd_cancel failed.\n");
    wd_delete(key_priv.wdog_id);
    ret = close(key_priv.fd);
    if (ret < 0)
        printf("close adc fd failed.\n");
    task_delete(key_priv.adc_pid);
    return ret;
}

static int adc_main(int argc, FAR char *argv[])
{
#if 0//def CONFIG_BOARD_SHENZHEN
    unsigned long value;
#endif
	int ret;
    key_priv.fd = open(ADC_DEV_NAME, O_RDONLY);
    if (key_priv.fd < 0)
        return -1;//NOT_OK;
#if 0//def CONFIG_BOARD_SHENZHEN
    printf("open adc key CONFIG_BOARD_SHENZHEN.\n");
    value = REG32(KSEG1(SILAN_PADMUX_CTRL2));
    value &= (~(1 << 31));
    REG32(KSEG1(SILAN_PADMUX_CTRL2)) = value;
    value = REG32(KSEG1(SILAN_PADMUX_CTRL));
    value &= (~(1 << 28));
    value &= (~(1 << 29));
    REG32(KSEG1(SILAN_PADMUX_CTRL)) = value;
#endif
    silan_adc_ioctl(key_priv.fd, CMD_ADC_SET_CHANNEL, ADC_CHANNEL);
    key_detect_reset();
    key_priv.wdog_id = wd_create();
    ret = wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
    while (1)
		usleep(10000);

    return 0;
}

void adc_start(void)
{
    key_priv.adc_pid = task_create("adckey", 5, 8192, adc_main, NULL);
    if (key_priv.adc_pid < 0) {
        LOGE("ADC", "Failed to start task (%d)", errno);
        return ;
    }
}

int sc6138_adcinitialize(void)
{
    silan_adc_probe();
    key_priv.adc_pid = task_create("adckey", 5, 8192, adc_main, NULL);
    if (key_priv.adc_pid < 0) {
        LOGE("ADC", "Failed to start task (%d)", errno);
        return -errno;
    }

    return OK;
}
//zhuque_device_init(sc6138_adcinitialize);
