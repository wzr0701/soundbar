/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "iic.h"
#include "rda5807.h"
#include <typedef.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*RDA5807芯片ID*/
#define CHIP_ID  0x20 //0x20
/*周期计数值*/
#define CLK_CNT (0x10000000)
/*时钟引脚*/
#define CLK_PIN (32+13)
/*数据引脚*/
#define DATA_PIN (32+10)
/*重发次数*/
#define RESEND_TIMES (8)
/*32.768晶振*/
//#define _SHARE_CRYSTAL_32KHz_
/*命令在列表中的位置*/
#define		FM_RESET		0
#define		FM_PON			2
#define		FM_INIT			4
#define		FM_FREQ			76
#define		FM_MUTE			84

/****************************************************************************
 * Public Types
 ****************************************************************************/
 
/****************************************************************************
* Public Functions
****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/
//RDA5807FP参数
const unsigned char tuner_cmd_table[] = 
{
		//------------------------------
		0x00,0x02, //FM_RESET 0
		0xc0,0x01, //FM_PON 2
		//------------------------------
#if defined(_SHARE_CRYSTAL_24MHz_)
		0xC4, 0x51, //02H:
#elif defined(_SHARE_CRYSTAL_12MHz_)
		0xC4, 0x11, //02H:
#elif defined(_SHARE_CRYSTAL_32KHz_)
		0xC4, 0x01, //02H:
#else
		0xd0, 0x01,
#endif
	
#if defined(_FM_STEP_50K_)
		0x00,0x12,
#else
		0x00,0x10,
#endif
	
		0x04, 0x00,
		0xC3, 0xaf, //05h //高8位的低4位(bit11-bit8): 搜台snr阈值
		0x60, 0x00,
		0x22, 0x2e, //07h //低8位的高6位(bit7-bit2): 搜台rssi阈值
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,  //0x0ah
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,
		0x00, 0x00,  //0x10h
		0x00, 0x19,
		0x2a, 0x11,
		0xB0, 0x42,  
		0x2A, 0x11,  //BIT5-BIT4 设置整体输出
		0xb8, 0x31,  //0x15h 
		0xc0, 0x00,
		0x2a, 0x91,
		0x94, 0x00,
		0x00, 0xa8,
		0xc4, 0x00,  //0x1ah
		0xF7, 0xcF,   
		0x2A, 0xDC,  //0x1ch
		0x80, 0x6F, 
		0x46, 0x08,
		0x00, 0x86, //10000110								
		0x06, 0x61, //0x20H
		0x00, 0x00, //某些方案上TUNE以后出来声音慢，可以将这个寄存器修改成0x00,0x03
		0x10, 0x9E,
		0x23, 0xC8,
		0x04, 0x06,
		0x0E, 0x1C, //0x25H
	
		//------------------------------
#if defined(_SHARE_CRYSTAL_24MHz_)
		0xC4, 0x51, //02H:
#elif defined(_SHARE_CRYSTAL_12MHz_)
		0xC4, 0x11, //02H:
#elif defined(_SHARE_CRYSTAL_32KHz_)
		0xC4, 0x01, //02H:
#else
		0xC0, 0x01,
#endif
	
#if defined(_FM_STEP_50K_)
		0x00,0x12,
#else
		0x00,0x10,
#endif
	
		0x04, 0x00,
		0xC3, 0xaf, //05h
		//------------------------------
		0x80,0x01, //FM_MUTE 84

};

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: rda5807_stop
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
static bool rda5807_chip_id(bool is_read)
{
    iic_send_byte(CHIP_ID|(is_read?0x01:0x00), CLK_PIN, DATA_PIN);
    bool ret = iic_recv_ack(CLK_PIN, DATA_PIN) ;
    return ret;
}

/****************************************************************************
 * Name: rda5807_init
 *
 * Description:
 *    向FM收音IC发送初始化信号
 *
 * Parameters:
 *
 * Returned Value:
 *    true发送成功，false发送失败
 *
 * Assumptions:
 *
 ****************************************************************************/
bool rda5807_init(void)
{
    bool ret = false;
    int i;
	
    for(i=0;i<RESEND_TIMES;++i)
    {
        if(rda5807_write(&(tuner_cmd_table[FM_INIT]), 72))
        {
            ret = true;
            break;
        }
    }
    return ret;
}

unsigned short rda5807_read_chipid(void)
{
	unsigned char chipid[10] = {0};
	unsigned short temp = 0;
	int i;
	rda5807_init_iic();
	if(rda5807_read(chipid,10))
	{
		temp = chipid[4];
		temp = ((temp<<8) | chipid[5]);
		printf(" RDA5807 ChipID: %x.\n",temp);
	}
	
	if(temp == 0x5803)
    	return 1;
	else
		return 0;
}


/****************************************************************************
 * Name: rda5807_init_iic
 *
 * Description:
 *    与FM收音IC通信的IO口初始化
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void rda5807_init_iic(void)
{
    iic_init(CLK_PIN, DATA_PIN);
}

/****************************************************************************
 * Name: rda5807_power_on
 *
 * Description:
 *    向FM收音IC发送开机信号
 *
 * Parameters:
 *
 * Returned Value:
 *    true发送成功，false发送失败
 *
 * Assumptions:
 *
 ****************************************************************************/
bool rda5807_power_on(void)
{
    bool ret = false;
    int i;
    for(i=0;i<RESEND_TIMES;++i)
    {
        if(rda5807_write(&(tuner_cmd_table[FM_PON]), 2))
        {
            ret = true;
            break;
        }
    }
    return ret;
}

/****************************************************************************
 * Name: rda5807_reset
 *
 * Description:
 *    向FM收音IC发送复位信号
 *
 * Parameters:
 *
 * Returned Value:
 *    true发送成功，false发送失败
 *
 * Assumptions:
 *
 ****************************************************************************/
bool rda5807_reset(void)
{
    bool ret = false;
    int i;
    for(i=0;i<RESEND_TIMES;++i)
    {
        if(rda5807_write(&(tuner_cmd_table[FM_RESET]), 2))
        {
            ret = true;
            break;
        }
    }
    return ret;

//	unsigned short temp = 0;
//	rda5807_write_reg(0x02, 0x0002);
//	usleep(50000);
//	rda5807_write_reg(0x02, 0xd081);
}

/****************************************************************************
 * Name: rda5807_read
 *
 * Description:
 *    从FM收音IC读取指定长度数据
 *
 * Parameters:
 *    buf 缓存的区
 *    length 数据长度
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
bool rda5807_read(unsigned char* buf, int length)
{
    int i;
    bool ret = false;
    if (NULL != buf && length > 0)
    {
        //发送开始信号
        iic_start(CLK_PIN, DATA_PIN);
        if (rda5807_chip_id(true))
        {
            ret = true;
            for(i=0;i<length;++i)
            {
                buf[i] = iic_read_byte(CLK_PIN, DATA_PIN);
                iic_send_ack(CLK_PIN, DATA_PIN);
            }
        }
        //停止之前的操作
        iic_stop(CLK_PIN, DATA_PIN);
    }
    return ret;
}

/****************************************************************************
 * Name: rda5807_read
 *
 * Description:
 *    从FM收音IC读取指定长度数据
 *
 * Parameters:
 *    buf 缓存的区
 *    length 数据长度
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
unsigned short rda5807_read_reg(unsigned char reg)
{
    int i;
	unsigned short temp = 0;
	unsigned char buf[2];
    bool ret = false;
	
    //发送开始信号
    iic_start(CLK_PIN, DATA_PIN);
    if (rda5807_chip_id(false))
    {
		iic_send_byte(reg, CLK_PIN, DATA_PIN);
		if(iic_recv_ack(CLK_PIN, DATA_PIN))
		{
			iic_start(CLK_PIN, DATA_PIN);
			if (rda5807_chip_id(true))
			{
                buf[0] = iic_read_byte(CLK_PIN, DATA_PIN);
                iic_send_ack(CLK_PIN, DATA_PIN);
                buf[1] = iic_read_byte(CLK_PIN, DATA_PIN);
				iic_send_nack(CLK_PIN, DATA_PIN);
				temp = ((buf[0] << 8) | buf[1]); 
			}
		}
    }
    //停止之前的操作
    iic_stop(CLK_PIN, DATA_PIN);
	
    return temp;
}

/****************************************************************************
 * Name: rda5807_write_reg
 *
 * Description:
 *    向RDA5807写入命令
 *
 * Parameters:
 *    cmds 命令列表
 *    length 长度
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
bool rda5807_write_reg(unsigned char reg, unsigned short data)
{
    bool ret = false;
    //发送开始信号
    iic_start(CLK_PIN, DATA_PIN);
    if (rda5807_chip_id(false))
    {
        iic_send_byte(reg, CLK_PIN, DATA_PIN);
        ret = iic_recv_ack(CLK_PIN, DATA_PIN);
        iic_send_byte((unsigned char)(data>>8), CLK_PIN, DATA_PIN);
        ret = iic_recv_ack(CLK_PIN, DATA_PIN);
        iic_send_byte((unsigned char)data, CLK_PIN, DATA_PIN);
        ret = iic_recv_ack(CLK_PIN, DATA_PIN);
    }
    //停止之前的操作
    iic_stop(CLK_PIN, DATA_PIN);
    return ret;
}

/****************************************************************************
 * Name: rda5807_set_freq
 *
 * Description:
 *    设置RDA5807的频率
 *
 * Parameters:
 *    freq 要设置的频率
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
bool rda5807_set_freq(unsigned short freq)
{
    bool ret = false;
    int i;
	int tmp = 0;
	#ifdef  _FM_STEP_50K_
    tmp = ((freq-8700)/5);
	#else
    tmp = ((freq-8700)/10);
	#endif
    unsigned char buf[4]={0};
    int length = sizeof(buf)/sizeof(buf[0]);
	//printf("fm set freq %d \n", freq);
    for(i=0;i<length;++i)
    {
        buf[i] = tuner_cmd_table[FM_FREQ+i];
		//printf("tuner cmd buf[%d] :%x \n", i, buf[i]);
    }
    buf[2] = tmp>>2;
	#ifdef	_FM_STEP_50K_
	buf[3] = (((tmp&0x0003)<<6)|0x12);	//set tune bit 0x10
	#else
	buf[3] = (((tmp&0x0003)<<6)|0x10);	//set tune bit 0x10
	#endif
    for(i=0;i<RESEND_TIMES;++i)
    {
        if(rda5807_write(buf, 4))
        {
			//printf("fm set freq %d \n", freq);
            ret = true;
            break;
        }
    }
    return 1;
}


/****************************************************************************
 * Name: rda5807_write
 *
 * Description:
 *    向RDA5807写入命令
 *
 * Parameters:
 *    cmds 命令列表
 *    length 长度
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
bool rda5807_write(const unsigned char* cmds, int length)
{
    bool ret = false;
    if(NULL != cmds && length > 0)
    {
        //发送开始信号
        iic_start(CLK_PIN, DATA_PIN);
        if (rda5807_chip_id(false))
        {
            int i;
            for(i=0;i<length;++i)
            {
                iic_send_byte(cmds[i], CLK_PIN, DATA_PIN);
                if(!iic_recv_ack(CLK_PIN, DATA_PIN))
                {
                    break;
                }
            }

            if (i == length)
            {//发送成功
                ret = true;
            }
        }
        //停止之前的操作
        iic_stop(CLK_PIN, DATA_PIN);
    }
    return ret;
}

void rda5807_mute(void)
{
    unsigned char buf[2]={0};
	int i;
    buf[0] = 0x90;
	buf[1] = 0x01;
    for(i=0;i<RESEND_TIMES;++i)
    {
        if(rda5807_write(buf, 2))
        {
            break;
        }
    }
}

void rda5807_unmute(void)
{
    unsigned char buf[2]={0};
	int i;
    buf[0] = 0xd0;
	buf[1] = 0x01;
    for(i=0;i<RESEND_TIMES;++i)
    {
        if(rda5807_write(buf, 2))
        {
            break;
        }
    }
}

