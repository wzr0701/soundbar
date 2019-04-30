/*--------------------------------------------------------------------------*/
/**@file RDA5807P.C
   @brief FM模块RDA5807P驱动
   @details FM模块RDA5807P的驱动函数
   @author LZZ
   @date 2010-08-17
   @note 本驱动支持模块，支持32K共晶振,支持P05输出时钟,三种方式时钟的程序一样
         通过识别ID来区别各种版本
*/
/*----------------------------------------------------------------------------*/

#include "iic.h"
#include <typedef.h>
#include <silan_resources.h>
#include <silan_addrspace.h>
#include <stdio.h>
#include <string.h>
#include "RDA5807P.h"




#define DEVICE_RDA5807P_ADDR   0x20

#define FM_I2C_CLK_PIN 45

#define FM_I2C_DATA_PIN 42





#define USE_P05_AS_FM_CLOCK           1
#define USE_CRYSTAL_AS_FM_CLOCK       2  //共用晶振
#define USE_MODULE                    3  //使用模块,FM芯片自起振32.768K晶振的情况都属于这个模式

/*
  此项宏定义说明是使用何种FM驱动方式
  USE_P05_AS_FM_CLOCK，指使用P05作为时钟信号
  USE_CRYSTAL_AS_FM_CLOCK，使用外部32K作为驱动信号
  USE_MODULE，使用模块,FM芯片自起振晶振的情况都属于这个模式
*/
#define FM_CLOCK USE_CRYSTAL_AS_FM_CLOCK


//#define CS1000_MUTE      dat[0]|=0x80;     //mute
#define CS1000_RD_CTRL    0x21
#define CS1000_WR_CTRL   0x20

bool fm_rx_stereo;
u16 rda5807_chipID;
u8 dat[24];   //RDA5807与CL6017共用
u8 readdata[10]; //RDA5807与CL6017共用

void FM_Write_3(u8 size);
void FM_Write_2(u8 size);
void FM_Read_2(u8 size);

extern void Delay5Ms(unsigned char  t_ms);


/*此初始化支持 RDA5807PE  RDA5807SP 版本*/
const u8 rad5807_init_tbl[104] =           //    原来的5807SP
{
    0xC0, 0x01,
    0x00, 0x10,
    0x04, 0x00,
    0x86, 0xad, //05H:
    0x80, 0x00,
    0x5F, 0x1A, //07H
    0x5e, 0xc6,
    0x00, 0x00,
    0x40, 0x6e, //0AH:
    0x2d, 0x80,
    0x58, 0x03,
    0x58, 0x04,
    0x58, 0x04,
    0x58, 0x04,
    0x00, 0x47, //10H:
    0x90, 0x00,
    0xF5, 0x87,
    0x7F, 0x0B, //13H:
    0x04, 0xF1,
    0x5E, 0xc0, //15H: 0x42, 0xc0
    0x41, 0xe0,
    0x50, 0x6f,
    0x55, 0x92,
    0x00, 0x7d,
    0x10, 0xC0,//1AH
    0x07, 0x80,
    0x41, 0x1d,//1CH,
    0x40, 0x06,
    0x1f, 0x9B,
    0x4c, 0x2b,//1FH.
    0x81, 0x10, //20H:
    0x45, 0xa0,// 21H
    0x19, 0x3F, //22H
    0xaf, 0x40,
    0x06, 0x81,
    0x1b, 0x2a, //25H
    0x0D, 0x04,
    0x80, 0x9F, //0x80, 0x2F,
    0x17, 0x8A,
    0xD3, 0x49,
    0x11, 0x42,
    0xA0, 0xC4, //2BH
    0x3E, 0xBB,
    0x00, 0x00,
    0x58, 0x04,
    0x58, 0x04, //2FH
    0x58, 0x04,
    0x00, 0x74,
    0x3D, 0x00,
    0x03, 0x0C,
    0x2F, 0x68,
    0x38, 0x77, //35H
};

/*此初始化支持 RDA5807HS  RDA5807HP 版本*/
const u8 rad5807H_init_tbl[110] =      //更新后的5807，型号5807HP
{
    0xc0,0x01,
    0x00,0x10,
    0x04,0x00,
    0x88,0xB0, //05h://86    如打开20H寄存器，可在此调搜台个数,与SP版一样，0X80搜台最多
    0x40,0x00,
    0x7E,0xc6,					//以下寄存器配置不可省去
    0x00,0x00,
    0x00,0x00,
    0x00,0x00,  //0x0ah
    0x00,0x00,
    0x00,0x00,
    0x00,0x00,
    0x00,0x00,
    0x00,0x00,
    0x00,0x06,  //0x10h
    0x00,0x19,// //0x00,0x09,//0830
    0x2a,0x11,
    0x00,0x2e,
    0x2a,0x30,
    0xb8,0x3c,  //0x15h
    0x90,0x00,
    0x2a,0x91,
    0x84,0x12,
    0x00,0xa8,
    0xc4,0x00,  //0x1ah
    0xe0,0x00,
    0x30,0x1d,//0x24,0x9d,1ch
    0x81,0x6a,
    0x46,0x08,
    0x00,0x86,  //0x1fh
    0x06,0x61,// 0x16,0x61, 20h  //0X16为打开第05H寄存器，可在05H调搜台数
    0x00,0x00,
    0x10,0x9e,
    0x25,0x4A,//0x24,0Xc9台多  //   0x23,0x46 //0x25,0x4a //0x25,0xCB  //0x26,0x4c台相对少,但清楚台更多，根据客户需求选择  //23h  搜台门限值调整
    0x04,0x08,//0830
    0x0c,0x16,//0x06,0x08,  //0x06,0x08,//0830  //0x25h
    0xe1,0x05,
    0x3b,0x6c,
    0x2b,0xec,
    0x09,0x0f,
    0x34,0x14,  //0x2ah
    0x14,0x50,
    0x09,0x6d,
    0x2d,0x96,
    0x01,0xda,
    0x2a,0x7b,
    0x08,0x21,   //0x30h
    0x13,0xd5,
    0x48,0x91,
    0x00,0xbc,
    0x08,0x96,//0830  0x34h
    0x15,0x3c,
    0x0b,0x80,
    0x25,0xc7,
    0x00,0x00,
};

/*此初始化支持 RDA5807MP  RDA5807M 版本*/
const u8 rad5807M_init_tbl[70] =    //更新后的5807，型号5807MP
{
    0xC0, 0x05,
    0x00, 0x00,
    0x04, 0x00,
    0xC3, 0xad, //05h
    0x60, 0x00,
    0x42, 0x16,
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
    0x2A, 0x11,
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
    0x00, 0x86,
    0x06, 0x61, //0x20H
    0x00, 0x00,
    0x10, 0x9E,
    0x23, 0xC8,
    0x04, 0x06,
};



/*----------------------------------------------------------------------------*/
/**@brief FM模块RDA5807P初始化
   @param 无
   @return 无
   @note
*/
/*----------------------------------------------------------------------------*/
void rda5807p_init(void)
{
    u8 i;
    iic_init(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    if (0x5804 == rda5807_chipID)
    {
        for(i=0;i<=11;i++)
            dat[i] = rad5807_init_tbl[i];

        //power on
        FM_Write_2(2);
        Delay5Ms(120);
        //init
        FM_Write_3(104);
        Delay5Ms(1);
    }
    else if(0x5801 == rda5807_chipID)
    {
        for(i=0;i<=11;i++)
          dat[i] = rad5807H_init_tbl[i];
        //power on

        FM_Write_2(2);
        Delay5Ms(120);
        //init
        FM_Write_3(110);
        Delay5Ms(1);
    }
    else if (0x5808 == rda5807_chipID)
    {
        for(i=0;i<=11;i++)
          dat[i] = rad5807M_init_tbl[i];
        //power on

        FM_Write_2(2);
        Delay5Ms(120);
        //init

        FM_Write_3(70);

        Delay5Ms(1);
    }
}

void CS1000_Powerdown(void)
{
    dat[1] &= ~(1<<0);
    FM_Write_2(2);
    Delay5Ms(100);
}

void CS1000_FMSeek_TuneMethod(u16 TunerFrequency,u8 diect)
{
    u16 ch;

    ch = 0;


    ch = (TunerFrequency - 8700)/10;
   // dat[0] |=0x40;
    dat[2] = ch>>2;
    dat[3] = ((ch&0x0003)<<6)|0x10;

    FM_Write_2(4);
    Delay5Ms(10);
}








void FM_Read_2(u8 size)
{
    u16 i;

    iic_start(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    iic_send_byte(0X21, FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    iic_recv_ack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);

    for (i=0;i<size-1;i++)
    {
        readdata[i] =  iic_read_byte(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
        iic_send_ack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    }

    readdata[i] =  iic_read_byte(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
   iic_send_nack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);

    iic_stop(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
}

void FM_Write_2(u8 size)
{
    u8 i;

    iic_start(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    iic_send_byte(0X20, FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    iic_recv_ack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);

    for (i=0;i<size;i++)
    {
        iic_send_byte(dat[i],FM_I2C_CLK_PIN,FM_I2C_DATA_PIN);
        iic_recv_ack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    }

    iic_stop(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
}


void FM_Write_3(u8 size)
{
    u8 i;

    iic_start(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    iic_send_byte(0X20, FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    iic_recv_ack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);

    for (i=0;i<size;i++)
    {
        if(0x5804 == rda5807_chipID)
        {
            iic_send_byte(rad5807_init_tbl[i], FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
        }
        else if(0x5801 == rda5807_chipID)
        {
            iic_send_byte(rad5807H_init_tbl[i], FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
        }
        else if(0x5808 == rda5807_chipID)
        {
            iic_send_byte(rad5807M_init_tbl[i], FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
        }

        iic_recv_ack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    }


    iic_stop(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
}
/*----------------------------------------------------------------------------*/
/**@brief FM模块RDA5807P设置频率
   @param freq 要设置的频率
   @return 无
   @note 频率范围为875~1080
*/
/*----------------------------------------------------------------------------*/
void rda5807p_set_freq(u16 freq)
{
    u8 try_cnt = 2;

    while (try_cnt--)
    {
        CS1000_FMSeek_TuneMethod(freq*10,0);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief FM模块RDA5807P关闭
   @param 无
   @return 无
   @note
*/
/*----------------------------------------------------------------------------*/
void rda5807p_off(void)
{
    CS1000_Powerdown();
}

/*----------------------------------------------------------------------------*/
/**@brief FM模块RDA5807P设置音量
   @param vol 音量值
   @return 无
   @note 音量范围0~15
*/
/*----------------------------------------------------------------------------*/
void rda5807p_set_vol(u8 vol)
{
    if(vol > 15)
        vol = 15;

    dat[7] &=~0x0F;
    dat[7] |= vol;

    dat[3] &= ~(1<<4);  //03H.4=0,disable tune
    if(vol == 0)
    {
        dat[0] &= ~(1<<6);//MUTE ENABLE
    }
    else
    {
        dat[0] |= (1<<6)|(1<<7);
        dat[1] |= (1<<0);
    }
    FM_Write_2(8);
}

/*----------------------------------------------------------------------------*/
/**@brief FM模块RDA5807P自动搜台
   @param freq 要搜索的频率
   @return 指定的频率下有台返回1，否则返回0
   @note
*/
/*----------------------------------------------------------------------------*/
u8 rda5807p_seek(u16 freq)
{
   rda5807p_set_freq(freq);

    Delay5Ms(10);

    //do
    //{
        FM_Read_2(4);
        Delay5Ms(4);
       // fm_rx_rdy = (readdata[3]>>7) & 0x01;
   // }
    //while (!((readdata[3]>>7) & 0x01));


//    if (flag)
//    {
//        rda5807p_set_freq(cur_freq);
//        return cur_freq;
//    }
   // fm_rx_true = readdata[2] & 0x01;
    if (readdata[2] & 0x01)//FM_TRUE
        return true;
    else
        return false;
}

void rda5807p_mute(void)
{
    dat[0] &= ~(1<<6);//MUTE ENABLE

    FM_Write_2(2);
    Delay5Ms(10);
}

void rda5807p_unmute(void)
{
    dat[0] |= (1<<6)|(1<<7);
    dat[1] |= (1<<0);

    FM_Write_2(2);
    Delay5Ms(10);
}
/*----------------------------------------------------------------------------*/
/**@brief FM模块RDA5807P检测
   @param 无
   @return 检测到RDA5807P模块返回1，否则返回0
   @note
*/
/*----------------------------------------------------------------------------*/
u8 rda5807p_online(void)
{
    iic_init(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
   	dat[0] = 0x00;
    dat[1] = 0x02;
    FM_Write_2(2);
    //read ID
    FM_Read_2(10);
    rda5807_chipID = readdata[8];
    rda5807_chipID = (rda5807_chipID << 8) | readdata[9];

	printf("%s:rda5807_chipID === %x.\r\n",__func__,rda5807_chipID);

    if((rda5807_chipID == 0x5801) || (rda5807_chipID == 0x5804) || (rda5807_chipID == 0x5808))
      return 1;
    else
      return 0;
}

