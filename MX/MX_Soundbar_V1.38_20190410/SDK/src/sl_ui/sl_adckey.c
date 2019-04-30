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

 /****************************************************************************
 * Included Files
 ****************************************************************************/
#include <arch/board/board.h>
#include <fcntl.h>
#include "input_key.h"
#include <nuttx/arch.h>
#include <nuttx/clock.h>
#include <nuttx/config.h>
#include <nuttx/input.h>
#include <nuttx/module.h>
#include <poll.h>
#include <silan_adc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include "../sl_ui/sl_ui_cmd.h"
#include "sl_ui_handle.h"


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*ADC按键通道*/
#define ADC_KEY_CHANNEL 0x40

/*ADC按键检测周期*/
#define SC6138_ADCKEY_TIMEOUT 1 //10ms


/*按键调试信息打印开关*/
#define KEY_DEBUG
/*ADC范围检测*/
#define KEY_RANGE(a, b) (((a > b - 100) && (a < b + 100)) ? 1 : 0)
/*打印函数配置*/
#ifdef KEY_DEBUG
#define PRINTF printf
#else
#define PRINTF(fmt, arg...)
#endif


// ADC值检测周期
#define SC6138_ADCDETECT_TIMEOUT	1 //10ms

/****************************************************************************
 * Public Types
 ****************************************************************************/
int irkey_detect    = -1;

struct s_key
{
    WDOG_ID wdog_id;
    int level_value;
    int level_detect_number;
    int ch_num;
    int key_num;
    int fd;
    pid_t adc_pid;
};

/****************************************************************************
 * Public Data
 ****************************************************************************/
static struct s_key key_priv;

//static int key_value_tbl[] = {0x398,0x5dc,0x7,0x76c, 0x845};
static int key_value_tbl[] = {0x7,0x3b5,0x776, 0xa18,0xbd2,0xcea,0xdaf};



#define POWER_SOURCE_BUTTON		13//

static WDOG_ID adc_wdt;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
//内部函数
static void key_detect_low_level(int argc, uint32_t irq_num, ...);
static void key_detect_reset(void);
static int key_num_translate(int key_num);

//外部函数
extern void button_add_event(struct input_event *event);
extern void msleep(int ms);
extern void silan_adc_read(int fd, char* buf, int size);
extern void silan_adc_ioctl(int fd, int cmd, int channel);
/****************************************************************************
 * Public Functions
 ****************************************************************************/
 static void Adc_Get_Value(void);

 /****************************************************************************
 * Name: adc_stop
 *
 * Description:
 *    启动ADC进程
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
int adc_stop(void)
{
    int ret;
    ret = wd_cancel(key_priv.wdog_id);
    if (ret == ERROR)
    {
        printf("wd_cancel failed.\n");
    }
    wd_delete(key_priv.wdog_id);
    key_priv.wdog_id = NULL;
    ret = close(key_priv.fd);
    if (ret < 0)
    {
        printf("close adc fd failed.\n");
    }
    key_priv.fd = -1;
    task_delete(key_priv.adc_pid);
    return ret;
}

/****************************************************************************
 * Name: key_detect_channel
 *
 * Description:
 *    获取按键当前的状态
 *
 * Parameters:
 *
 * Returned Value:
 *    0-按下，1-弹起
 *
 * Assumptions:
 *
 ****************************************************************************/
static int key_detect_channel(void)
{
    int m;
    unsigned short buf[ADC_MAX_CH_NUM];
    char * p = (char *)buf;
    //初始化ADC接收缓存区
    memset(p, 0xff, ADC_MAX_CH_NUM * 2);
    //读取各个通道的ADC值
    silan_adc_read(key_priv.fd, p, ADC_MAX_CH_NUM * 2);

    //检查是有有按键
    m = KEY_RANGE(buf[key_priv.ch_num], key_value_tbl[key_priv.key_num]);
    if (m)
    {   //有按键
        return 0;
    }
    else
    {   //没有按键
        return 1;
    }
}

/****************************************************************************
 * Name: key_find_channel
 *
 * Description:
 *    获取当前按键属于那个ADC通道和按键在键值表中的序号
 *
 * Parameters:
 *
 * Returned Value:
 *    -1 没有找到对应的按键序号，非-1对应的按键序号
 *
 * Assumptions:
 *
 ****************************************************************************/
static int key_find_channel(void)
{
    int i, j, k, m;
    unsigned short buf[ADC_MAX_CH_NUM];
    char *p = (char *)buf;
    //初始化缓存区
    memset(p, 0xff, ADC_MAX_CH_NUM * 2);
    //读取当前ADC通道值
    silan_adc_read(key_priv.fd, p, ADC_MAX_CH_NUM * 2);
	
    for (i = 0; i < ADC_MAX_CH_NUM; i++)
    {
        j = (ADC_KEY_CHANNEL >> i) & 0x01;
        if (j)
        {   //找到对应的ADC通道
            //检查当前ADC值是否在某个设定的值的范围内
			//printf("ADC_Value:0x%x\n", buf[i]);
            for (k = 0; k < ADC_MAX_CH_VAL; k++)
            {
                m = KEY_RANGE(buf[i], key_value_tbl[k]);
                if (m)
                {   //当前的ADC值在某个设定值的范围内
                    //记录值的序号
                    key_priv.key_num = k;
                    break;
                }
            }
            if (key_priv.key_num != -1)
            {   //有按键
                //设置ADC通过号
                key_priv.ch_num = i;
                //检测到按键
                return 0;
            }
        }
    }
	
    //没有检测到按键
    return -1;
}

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

    //更新按键检测状态
    value = key_detect_channel();
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
        //发送按键释放消息
        button_add_event(&key_event);
        //按键检复位
        key_detect_reset();
        //启动按键低状态检测看门狗计时器
        wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
        return;
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
		//printf("key_priv.key_num:%d\n", key_priv.key_num);
		//printf("key_event.value:%d\n", key_event.value);
        //发送按键按键下消息
        button_add_event(&key_event);
        irkey_detect = -1;
        //启动按键高状态检测看门狗计时器
        wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_high_level, 1, 0);
        return;
    }

    //去抖动还未成功，继续检测按键低状态
    wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
}

/****************************************************************************
 * Name: key_detect_reset
 *
 * Description:
 *    按键检查复位
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void key_detect_reset(void)
{
    key_priv.level_value = 0;
    key_priv.level_detect_number = 0;
    key_priv.ch_num = -1;
    key_priv.key_num = -1;
}

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

 /****************************************************************************
 * Name: adc_main
 *
 * Description:
 *    ADC进程主循环，设置ADC并启动看门狗计时检测按键状态
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static int adc_main(int argc, FAR char *argv[])
{
    //打开ADC设备
    key_priv.fd = open(ADC_DEV_NAME, O_RDONLY);

    if (key_priv.fd < 0)
    {   //ADC设备打开失败
        return -1;
    }
    //设置ADC参数
    silan_adc_ioctl(key_priv.fd, CMD_ADC_SET_CHANNEL, ADC_KEY_CHANNEL); //ADC_CHANNEL ADC_KEY_CHANNEL
    //按键检测复位
    key_detect_reset();
    //为按键生成一个看门狗计时器
    key_priv.wdog_id = wd_create();
    //启动看门狗计时检查按键低状态
    wd_start(key_priv.wdog_id, SC6138_ADCKEY_TIMEOUT, key_detect_low_level, 1, 0);
//	//ADC监测看门狗
//	adc_wdt = wd_create();
//   //启动看门狗计时获取ADC值
//   wd_start(adc_wdt, SC6138_ADCDETECT_TIMEOUT, Adc_Get_Value, 1, 0);
	printf("%s adc init ", __func__);
    //进入死循环
    while (1)
    {
        msleep(10);
    }
    return 0;
}

 /****************************************************************************
 * Name: adc_start
 *
 * Description:
 *    启动ADC进程
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void adc_start(void)
{
    key_priv.adc_pid = task_create("adckey", 5, 8192, adc_main, NULL);
    if (key_priv.adc_pid < 0)
    {
        LOGE("ADC", "Failed to start task (%d)", errno);
    }
}

/****************************************************************************
 * Name: sc6138_adcinitialize
 *
 * Description:
 *    ADC进程启动函数
 *
 * Parameters:
 *
 * Returned Value:
 *    总是OK
 *
 * Assumptions:
 *
 ****************************************************************************/
static int sc6138_adcinitialize(void)
{
    //adc初始化
    silan_adc_probe();
    //启动ADC按键检测进程
    #if (MIX_6_CH_MIC)
    #else
    key_priv.adc_pid = task_create("adckey", 5, 8192, adc_main, NULL);
    if (key_priv.adc_pid < 0)
    {
        LOGE("ADC", "Failed to start task (%d)", errno);
        return -errno;
    }
    #endif

    return OK;
}
//设置上电执行ADC进程启动函数
zhuque_device_init(sc6138_adcinitialize);

static void Adc_Get_Value(void)
{
//	char *p = (char *)Adc_Read_Buf;
//  	silan_adc_read(key_priv.fd, p, ADC_MAX_CH_NUM * 2);
//
//    wd_start(adc_wdt, SC6138_ADCDETECT_TIMEOUT, Adc_Get_Value, 1, 0);
}







