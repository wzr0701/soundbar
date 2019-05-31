/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "ad82589.h"
#include "iic.h"
#include <typedef.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*RDA5807芯片ID*/
#define DEVICE_ADDR 0x62
/*时钟引脚*/
#define CLK_PIN (32+13)
/*数据引脚*/
#define DATA_PIN (32+10)

/****************************************************************************
 * Public Types
 ****************************************************************************/
 
/****************************************************************************
 * Public Data
 ****************************************************************************/
 
/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
static bool ad82589_read(unsigned char * buf, int length);
static bool ad82589_send_device_addr(bool w_r);
static bool ad82589_send_sub_addr(unsigned char addr);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: ad82589_init
 *
 * Description:
 *    ad82589初始化
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ad82589_init(void)
{

    const unsigned char datas[][2] = {
            {0x03, 0x19},           //DTC & Noise gate enable.
            {0x04, 0x88},           //Bass/Treble on + EQ_CH1CH2_Link
            {0x05, 0x1d},           //CH1 DRC & Power clipping enable RMS mode
            {0x06, 0x1d},           //CH2 DRC & Power clipping enable RMS mode
            {0x07, 0x11},           //CH1 DRC & Power clipping enable RMS mode
            {0x08, 0x11},           //CH2 DRC & Power clipping enable RMS mode
            {0x09, 0x80},           //PVDD_UVP on+4.0V=0x80, off+4.0V=0x00
            {0x0e, 0xff},
            {0x0f, 0xff},
            {0x13, 0x0c},           //Master Vol default=0dB
            {0x14, 0x0c},           //CH1 Vol default=0dB
            {0x15, 0x0c},           //CH2 Vol default=0dB
            {0x16, 0x0c},           //CH3 Vol default=0dB
            {0x17, 0x0c},           //CH4 Vol default=0dB
            {0x28, 0x0e},           //Bass_tone_control 12dB~-12dB 360Hz 2dB
            {0x29, 0x15},           //Treble_tone_control 12dB~-12dB 7KHz -5dB
    };
    int size = sizeof(datas)/sizeof(datas[0]);
    int i;
    //iic初始化
    iic_init(CLK_PIN, DATA_PIN);

    for(i=0;i<size;++i)
    {
        if(!ad82589_write_byte(datas[i][0], datas[i][1]))
        {
            break;
        }
    }

    if(i == size)
    {
        printf("ad82589.......write success\n");
    }
    else
    {
        printf("ad82589.......write fail\n");
    }

    unsigned char buf[32] = {0};
    size = 32;
    if(ad82589_read(buf, size))
    {
        printf("ad82589.......read success\n");
        for(i=0;i<size;++i)
        {
            printf("buf[0x%02x]=0x%02x\n",i,buf[i]);
        }
    }
    else
    {
        printf("ad82589.......read fail\n");
    }
}

/****************************************************************************
 * Name: ad82589_read
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
static bool ad82589_read(unsigned char * buf, int length)
{
    bool ret = false;
    
    if(NULL != buf && length > 0)
    {
        //发送开始信号
        iic_start(CLK_PIN, DATA_PIN);
        //发送带写信号的设备ID
        if(ad82589_send_device_addr(false))
        {//发送带写信号的设备ID成功
            //发送寄存器地址
            if(ad82589_send_sub_addr(0))
            {//发送寄存器地址成功
                //发送开始信号
                iic_start(CLK_PIN, DATA_PIN);
                //发送带读信号的设备ID
                if(ad82589_send_device_addr(true))
                {//发送带读信号的设备ID成功
                    int i;
                    for(i=0;i<length;++i)
                    {
                        buf[i] = iic_read_byte(CLK_PIN, DATA_PIN);
                        if(i!=length-1)
                        {
                            iic_send_ack(CLK_PIN, DATA_PIN);
                        }
                        else
                        {
                            iic_send_nack(CLK_PIN, DATA_PIN);
                        }
                    }
                    ret = true;
                }
            }
        }
        //停止之前的操作
        iic_stop(CLK_PIN, DATA_PIN);
    }

    return ret;
}

/****************************************************************************
 * Name: ad82589_send_device_addr
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
static bool ad82589_send_device_addr(bool w_r)
{
    iic_send_byte(DEVICE_ADDR|(w_r?1:0), CLK_PIN, DATA_PIN);
    return iic_recv_ack(CLK_PIN, DATA_PIN);
}

/****************************************************************************
 * Name: ad82589_send_sub_addr
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
static bool ad82589_send_sub_addr(unsigned char addr)
{
    iic_send_byte(addr, CLK_PIN, DATA_PIN);
    return iic_recv_ack(CLK_PIN, DATA_PIN);
}

/****************************************************************************
 * Name: ad82589_write_byte
 *
 * Description:
 *    通过IIC将特定值写到AD82589的特定寄存器
 *
 * Parameters:
 *    addr 寄存器地址
 *    byte 要写入的值
 *
 * Returned Value:
 *    true 成功，false失败
 *
 * Assumptions:
 *
 ****************************************************************************/
bool ad82589_write_byte(unsigned char addr, unsigned char byte)
{
    bool ret = false;
    //发送开始信号
    iic_start(CLK_PIN, DATA_PIN);
    //发送带写信号的设备ID
    if(ad82589_send_device_addr(false))
    {//发送带写信号的设备ID成功
        //发送寄存器地址
        if(ad82589_send_sub_addr(addr))
        {//发送寄存器地址成功
            //发送数据
            iic_send_byte(byte, CLK_PIN, DATA_PIN);
            ret = iic_recv_ack(CLK_PIN, DATA_PIN);
        }
    }
    //停止之前的操作
    iic_stop(CLK_PIN, DATA_PIN);

    return ret;
}