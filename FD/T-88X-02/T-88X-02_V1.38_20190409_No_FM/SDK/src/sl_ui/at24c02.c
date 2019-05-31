/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "at24c02.h"
#include "iic.h"
#include <stdio.h>
#include <silan_resources.h>
#include <silan_addrspace.h>
#include <fcntl.h>
#include <zhuque_bsp_gpio.h>

#include "sl_ui_handle.h"


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*at24c02芯片ID*/
#define AT_24C02_CHIP_ID 0xa0
/*周期计数值*/
#define CLK_CNT (0x10000000)
/*时钟引脚*/
#define AT24C02_CLK_PIN (32)
/*数据引脚*/
#define AT24C02_DATA_PIN (27)
/*EEPROM的大小*/
#define EEPROM_PAGE_SIZE (8)
/*EEPROM的页数*/
#define EEPROM_ADDR_MAX (256)
/*重发次数*/
#define RESEND_TIMES (5)


extern sem_t de_i2c_sem;

/****************************************************************************
 * Public Types
 ****************************************************************************/






/****************************************************************************
 * Name: AT24C02_init
 *
 * Description:
 *    初始化IIC的IO口
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void at24c02_init(void)
{
    iic_init(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
    //usleep(100);
}

/****************************************************************************
 * Name: at24c02_chip_id
 *
 * Description:
 *    IIC停止信号
 *
 * Parameters:
 *    is_read true是读，flase是写
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static bool at24c02_chip_id(bool is_read)
{

    iic_send_byte(AT_24C02_CHIP_ID|(is_read?0x01:0x00), AT24C02_CLK_PIN, AT24C02_DATA_PIN);
    bool ret = iic_recv_ack(AT24C02_CLK_PIN, AT24C02_DATA_PIN) ;

    return ret;
}

/****************************************************************************
 * Name: at24c02_read
 *
 * Description:
 *    从指定PAGE读取指定长度数据
 *
 * Parameters:
 *    page 从那一页码读
 *    buf 数据缓存
 *    len 读取的长度不超过16个byte
 *
 * Returned Value:
 *    true 读取成功，false读取失败
 *
 * Assumptions:
 *
 ****************************************************************************/
int at24c02_read(unsigned char addr, unsigned char* buf, int len)
{
    bool ret = false;
//	int semval;
//	sem_getvalue(&iic_state_sem, &semval);
//	printf("%s %d semval :%d \n", __func__, __LINE__, semval);
//	sem_wait(&iic_state_sem);

    if (NULL != buf && len > 0 && addr < EEPROM_ADDR_MAX && (addr + len) < EEPROM_ADDR_MAX)
    {//入参有效
        //发送开始信号
        iic_start(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
        if (at24c02_chip_id(false))
        {//发送写芯片ID成功
            //发送页码
            iic_send_byte(addr, AT24C02_CLK_PIN, AT24C02_DATA_PIN);
            if(iic_recv_ack(AT24C02_CLK_PIN, AT24C02_DATA_PIN))
            {//接收ACK成功
                //发送开始信号
                iic_start(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
                if (at24c02_chip_id(true))
                {//发送读芯片ID成功
                    int i;
                    for(i=0;i<len;++i)
                    {
                        //接收数据
                        buf[i] = iic_read_byte(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
                        if (i == len - 1)
                        {
                            //发送NACK信号
                            iic_send_nack(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
                        }
                        else
                        {
                            //发送ACK信号
                            iic_send_ack(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
                        }
                    }
                    ret = true;
                }
            }
        }
        iic_stop(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
    }

//	sem_post(&iic_state_sem);

    return ret;
}

/****************************************************************************
 * Name: at24c02_write
 *
 * Description:
 *    向EEPROM的指页写入指定长度数据
 *
 * Parameters:
 *    page 页码
 *    buf 数据缓存
 *    len 写入的长度
 *
 * Returned Value:
 *    true 写入成功，false 写入失败
 *
 * Assumptions:
 *
 ****************************************************************************/
bool at24c02_write(unsigned char addr, unsigned char* buf, int len)
{
    bool ret = false;
//	int semval;
//	sem_getvalue(&iic_state_sem, &semval);
//	printf("%s %d semval :%d \n", __func__, __LINE__, semval);
//	sem_wait(&iic_state_sem);

    if (addr < EEPROM_ADDR_MAX && addr + len < EEPROM_ADDR_MAX &&
        NULL != buf && len > 0 && len <= EEPROM_PAGE_SIZE)
    {
        #if 1
        //发送开始信号
        iic_start(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
        if (at24c02_chip_id(false))
        {//发送写芯片ID成功
            //发送页码
            iic_send_byte(addr, AT24C02_CLK_PIN, AT24C02_DATA_PIN);
            if(iic_recv_ack(AT24C02_CLK_PIN, AT24C02_DATA_PIN))
            {//接收ACK成功
                int i;
                for(i=0;i<len;++i)
                {
                    iic_send_byte(buf[i], AT24C02_CLK_PIN, AT24C02_DATA_PIN);
                    if(!iic_recv_ack(AT24C02_CLK_PIN, AT24C02_DATA_PIN))
                    {
                        break;
                    }
                }

                if (i == len)
                {//发送成功
                    ret = true;
                }
            }
        }
        //停止之前的操作
        iic_stop(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
        #else
        int i, j;
        unsigned char val = 0;
        for(i=0;i<len;++i)
        {
            while(!at24c02_read(addr+i, &val, 1))
            {

            }

            if(!at24c02_write_one_byte(addr+i, buf[i]))
            {
                break;
            }
        }

        if(i >= len)
        {
            ret = true;
        }
        #endif
    }

//	sem_post(&iic_state_sem);

    return ret;
}

/****************************************************************************
 * Name: at24c02_write
 *
 * Description:
 *    向EEPROM的指页写入指定长度数据
 *
 * Parameters:
 *    page 页码
 *    buf 数据缓存
 *    len 写入的长度
 *
 * Returned Value:
 *    true 写入成功，false 写入失败
 *
 * Assumptions:
 *
 ****************************************************************************/
 bool at24c02_write_one_byte(unsigned char addr, unsigned char data)
{
    bool ret = false;
    //发送开始信号
//	int semval;
//	sem_getvalue(&iic_state_sem, &semval);
//	printf("%s %d semval :%d \n", __func__, __LINE__, semval);
	//sem_wait(&de_i2c_sem);

    at24c02_init();

     iic_start(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
    if (at24c02_chip_id(false))
    {//发送写芯片ID成功
        //发送页码
        iic_send_byte(addr, AT24C02_CLK_PIN, AT24C02_DATA_PIN);
        if(iic_recv_ack(AT24C02_CLK_PIN, AT24C02_DATA_PIN))
        {//接收ACK成功
            iic_send_byte(data, AT24C02_CLK_PIN, AT24C02_DATA_PIN);
            if(iic_recv_ack(AT24C02_CLK_PIN, AT24C02_DATA_PIN))
            {
                ret = true;
            }
        }
    }
   else
   	{
       printf("at24c02_write_one_byte--------addr----err\r\n");//	at24c02_write_one_byte
   	}
    //停止之前的操作
    iic_stop(AT24C02_CLK_PIN, AT24C02_DATA_PIN);

	//sem_post(&de_i2c_sem);
}


/*************************************************



**************************************************/
unsigned char  at24c02_read_one_byte(unsigned char addr)
{

 unsigned char tmp;
//	int semval;
//	sem_getvalue(&iic_state_sem, &semval);
//	printf("%s %d semval :%d \n", __func__, __LINE__, semval);
	//sem_wait(&de_i2c_sem);
	  at24c02_init();
        iic_start(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
        if (at24c02_chip_id(false))
        {//发送写芯片ID成功
            iic_send_byte(addr, AT24C02_CLK_PIN, AT24C02_DATA_PIN);
            if(iic_recv_ack(AT24C02_CLK_PIN, AT24C02_DATA_PIN))
            {//接收ACK成功

                iic_start(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
                if (at24c02_chip_id(true))
                {
                          tmp= iic_read_byte(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
                           iic_send_nack(AT24C02_CLK_PIN, AT24C02_DATA_PIN);
                   }

                }
            }

        iic_stop(AT24C02_CLK_PIN, AT24C02_DATA_PIN);


	//sem_post(&de_i2c_sem);

    return tmp;
}