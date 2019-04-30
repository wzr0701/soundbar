#ifndef __IIC_H__
#define __IIC_H__
/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
 
/****************************************************************************
 * Public Types
 ****************************************************************************/
 
/****************************************************************************
 * Public Data
 ****************************************************************************/
 
/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
 
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: iic_init
 *
 * Description:
 *    IIC初始化
 *
 * Parameters:
 *    CLK_PIN clock引脚序号
 *    DATA_PIN data引脚序号
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void iic_init(unsigned int CLK_PIN, unsigned int DATA_PIN);

/****************************************************************************
 * Name: iic_read_byte
 *
 * Description:
 *    IIC开始发送一个byte数据
 *
 * Parameters:
 *    CLK_PIN clock引脚序号
 *    DATA_PIN data引脚序号
 *
 * Returned Value:
 *    读取到的值
 *
 * Assumptions:
 *
 ****************************************************************************/
unsigned char iic_read_byte(unsigned int CLK_PIN, unsigned int DATA_PIN);

/****************************************************************************
 * Name: iic_recv_ack
 *
 * Description:
 *    IIC接收ACK
 *
 * Parameters:
 *    CLK_PIN clock引脚序号
 *    DATA_PIN data引脚序号
 *
 * Returned Value:
 *    true 接收到ack，false 没有接收到ack
 *
 * Assumptions:
 *
 ****************************************************************************/
bool iic_recv_ack(unsigned int CLK_PIN, unsigned int DATA_PIN);

/****************************************************************************
 * Name: iic_send_ack
 *
 * Description:
 *    IIC开始发送一个ack
 *
 * Parameters:
 *    CLK_PIN clock引脚序号
 *    DATA_PIN data引脚序号
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void iic_send_ack(unsigned int CLK_PIN, unsigned int DATA_PIN);

/****************************************************************************
 * Name: iic_send_byte
 *
 * Description:
 *    IIC开始发送一个byte数据
 *
 * Parameters:
 *    byte 要发送的数据
 *    CLK_PIN clock引脚序号
 *    DATA_PIN data引脚序号
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void iic_send_byte(unsigned char byte, unsigned int CLK_PIN, unsigned int DATA_PIN);

/****************************************************************************
 * Name: iic_send_nack
 *
 * Description:
 *    IIC开始发送一个nack
 *
 * Parameters:
 *    CLK_PIN clock引脚序号
 *    DATA_PIN data引脚序号
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void iic_send_nack(unsigned int CLK_PIN, unsigned int DATA_PIN);

/****************************************************************************
 * Name: iic_start
 *
 * Description:
 *    IIC开始信号
 *
 * Parameters:
 *    CLK_PIN clock引脚序号
 *    DATA_PIN data引脚序号
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void iic_start(unsigned int CLK_PIN, unsigned int DATA_PIN);

/****************************************************************************
 * Name: iic_stop
 *
 * Description:
 *    IIC停止信号
 *
 * Parameters:
 *    CLK_PIN clock引脚序号
 *    DATA_PIN data引脚序号
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void iic_stop(unsigned int CLK_PIN, unsigned int DATA_PIN);
#endif