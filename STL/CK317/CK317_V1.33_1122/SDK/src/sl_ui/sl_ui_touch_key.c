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
#include "input_key.h"
#include "nuttx/wqueue.h"
#include "queue.h"
#include "iic.h"
#include "sl_ui_touch_key.h"

#define TOUCH_KEY_ADD  0x22

#define  TOUCH_KEY_VALUE  0X26

#define TOUCH_KEY_CLK_PIN     32

#define TOUCH_KEY_DATA_PIN   27


/****************************************************************************
 * Name: TOUCH_KEY_read
 *
 * Description:
 *    从ad82589设备中通过IIC从00地址开始读出指定长度的数据
 *
 * Parameters:
 *    buf       缓存区
 *    length    要读出的数据长度
 *
 * Returned Value:
 *    true成功，false失败
 *
 * Assumptions:
 *
 ****************************************************************************/
 bool touch_key_read(unsigned char * buf, int length)
{
    bool ret = false;
    iic_init(TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);

    if(NULL != buf && length > 0)
    {
        //发送开始信号
        iic_start(TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
        //发送带写信号的设备ID
        if(touch_key_send_device_addr(false))
        {//发送带写信号的设备ID成功
            //发送寄存器地址
            if(touch_key_send_sub_addr(TOUCH_KEY_VALUE))
            {//发送寄存器地址成功
                //发送开始信号
                iic_start(TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
                //发送带读信号的设备ID
                if(touch_key_send_device_addr(true))
                {//发送带读信号的设备ID成功

                    int i;
                    for(i=0;i<length;++i)
                    {
                        buf[i] = iic_read_byte(TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
                        if(i!=length-1)
                        {
                            iic_send_ack(TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
                        }
                        else
                        {
                            iic_send_nack(TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
                        }
                    }

                    ret = true;
                }
            }
        }
        //停止之前的操作
        iic_stop(TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
    }

    return ret;
}

/****************************************************************************
 * Name:  touch_key_send_device_addr
 *
 * Description:
 *    发送设备ID
 *
 * Parameters:
 *    w_r true读，false写
 *
 * Returned Value:
 *    true成功，false失败
 *
 * Assumptions:
 *
 ****************************************************************************/
 bool touch_key_send_device_addr(bool w_r)
{
    iic_send_byte(TOUCH_KEY_ADD|(w_r?1:0), TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
    return iic_recv_ack(TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
}

/****************************************************************************
 * Name: touch_key_send_sub_addr
 *
 * Description:
 *    发送寄存器地址
 *
 * Parameters:
 *    addr 寄存器地址
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
 bool touch_key_send_sub_addr(unsigned char addr)
{
    iic_send_byte(addr, TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
    return iic_recv_ack(TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
}


/*************************************************************************



***************************************************************************/
void touch_key_int(void)
{
#if 0
    zhuque_bsp_gpio_set_mode(KEY_INT_PIN, GPIO_IN, PULLING_DOWN);
    iic_init(TOUCH_KEY_CLK_PIN, TOUCH_KEY_DATA_PIN);
#endif
}






