/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "iic.h"
#include <silan_gpio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define HALF_CYCLE  (30)
#define CYCLE (2*HALF_CYCLE)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/*外部*/

/*内部*/
static void iic_cycle(unsigned int CLK_PIN);
static void iic_us(int us, unsigned int pin, int value);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: iic_cycle
 *
 * Description:
 *    IIC时钟CLK一个周期
 *
 * Parameters:
 *    CLK_PIN clock引脚序号
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void iic_cycle(unsigned int CLK_PIN)
{
    iic_us(HALF_CYCLE, CLK_PIN,GPIO_VALUE_LOW);
    iic_us(CYCLE, CLK_PIN,GPIO_VALUE_HIGH);
    iic_us(HALF_CYCLE, CLK_PIN,GPIO_VALUE_LOW);
}

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
void iic_init(unsigned int CLK_PIN, unsigned int DATA_PIN)
{
    zhuque_bsp_gpio_set_mode(CLK_PIN, GPIO_OUT, PULLING_HIGH);
    zhuque_bsp_gpio_set_mode(DATA_PIN, GPIO_OUT, PULLING_HIGH);
    iic_stop(CLK_PIN, DATA_PIN);
}

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
unsigned char iic_read_byte(unsigned int CLK_PIN, unsigned int DATA_PIN)
{
    unsigned char byte = 0;
    int i;
    int bit = 0x80;
    unsigned int value = 0;
    zhuque_bsp_gpio_set_mode(DATA_PIN, GPIO_IN, PULLING_HIGH);
    for(i=7;i>=0;--i,bit>>=1)
    {
        iic_us(HALF_CYCLE, CLK_PIN,GPIO_VALUE_LOW);
        iic_us(HALF_CYCLE, CLK_PIN,GPIO_VALUE_HIGH);
        zhuque_bsp_gpio_get_value(DATA_PIN,&value);
        if(GPIO_VALUE_HIGH == value)
        {
            byte |= bit;
        }
        iic_us(HALF_CYCLE, CLK_PIN,GPIO_VALUE_HIGH);
        iic_us(HALF_CYCLE, CLK_PIN,GPIO_VALUE_LOW);
    }
    zhuque_bsp_gpio_set_mode(DATA_PIN, GPIO_OUT, PULLING_HIGH);
    zhuque_bsp_gpio_set_value(DATA_PIN, GPIO_VALUE_LOW);
    return byte;
}

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
bool iic_recv_ack(unsigned int CLK_PIN, unsigned int DATA_PIN)
{
    unsigned int value = 0;
    zhuque_bsp_gpio_set_mode(DATA_PIN, GPIO_IN, PULLING_HIGH);
    iic_us(HALF_CYCLE, CLK_PIN,GPIO_VALUE_LOW);
    iic_us(HALF_CYCLE, CLK_PIN,GPIO_VALUE_HIGH);
    zhuque_bsp_gpio_get_value(DATA_PIN,&value);
    iic_us(HALF_CYCLE, CLK_PIN,GPIO_VALUE_HIGH);
    iic_us(HALF_CYCLE, CLK_PIN,GPIO_VALUE_LOW);
    zhuque_bsp_gpio_set_mode(DATA_PIN, GPIO_OUT, PULLING_HIGH);
    zhuque_bsp_gpio_set_value(DATA_PIN, GPIO_VALUE_LOW);
    return (GPIO_VALUE_LOW == value);
}

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
void iic_send_ack(unsigned int CLK_PIN, unsigned int DATA_PIN)
{
    zhuque_bsp_gpio_set_value(DATA_PIN, GPIO_VALUE_LOW);
    iic_cycle(CLK_PIN);
}

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
void iic_send_byte(unsigned char byte, unsigned int CLK_PIN, unsigned int DATA_PIN)
{
    int i;
    int bit = 0x80;
    for(i=7;i>=0;--i,bit>>=1)
    {
        if(bit & byte)
        {
            zhuque_bsp_gpio_set_value(DATA_PIN, GPIO_VALUE_HIGH);
        }
        else
        {
            zhuque_bsp_gpio_set_value(DATA_PIN, GPIO_VALUE_LOW);
        }
        iic_cycle(CLK_PIN);
    }
}

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
void iic_send_nack(unsigned int CLK_PIN, unsigned int DATA_PIN)
{
    zhuque_bsp_gpio_set_value(DATA_PIN, GPIO_VALUE_HIGH);
    iic_cycle(CLK_PIN);
}

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
void iic_start(unsigned int CLK_PIN, unsigned int DATA_PIN)
{
    zhuque_bsp_gpio_set_value(DATA_PIN, GPIO_VALUE_HIGH);
    zhuque_bsp_gpio_set_value(CLK_PIN, GPIO_VALUE_HIGH);
    iic_us(HALF_CYCLE, CLK_PIN, GPIO_VALUE_HIGH);
    iic_us(HALF_CYCLE, DATA_PIN, GPIO_VALUE_LOW);
    iic_us(HALF_CYCLE, CLK_PIN, GPIO_VALUE_LOW);
}

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
void iic_stop(unsigned int CLK_PIN, unsigned int DATA_PIN)
{
    zhuque_bsp_gpio_set_value(CLK_PIN, GPIO_VALUE_LOW);
    zhuque_bsp_gpio_set_value(DATA_PIN, GPIO_VALUE_LOW);
    iic_us(HALF_CYCLE, DATA_PIN, GPIO_VALUE_LOW);
    iic_us(HALF_CYCLE, CLK_PIN, GPIO_VALUE_HIGH);
    iic_us(HALF_CYCLE, DATA_PIN, GPIO_VALUE_HIGH);
}

/****************************************************************************
 * Name: iic_us
 *
 * Description:
 *    IIC延时
 *
 * Parameters:
 *    us 延时时间
 *    pin 延时时的输出引脚
 *    value 延时时引脚的输出值
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void iic_us(int us, unsigned int pin, int value)
{
    for(;us>=0;--us)
    {
        zhuque_bsp_gpio_set_value(pin, value);
    }
}