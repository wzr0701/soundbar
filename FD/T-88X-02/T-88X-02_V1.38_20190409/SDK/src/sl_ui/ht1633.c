#include <nuttx/arch.h>
#include <nuttx/module.h>
#include <silan_addrspace.h>
#include <silan_resources.h>
#include <silan_timer.h>
#include <sys/types.h>
#include <zhuque_bsp_gpio.h>
#include "iic.h"
#include "ht1633.h"

#include <nuttx/clock.h>
#include <nuttx/module.h>

#include <sys/time.h>
#include <time.h>




#define HT_I2C_ADD  0xe0
#define I2C_CLK_PIN 17
/*数据引脚*/
#define I2C_DATA_PIN 18


#define HT_LED_ON  0x81
#define HT_LED_OFF  0x80

#define HT_LED_POWER_ON 0x21
#define HT_LED_POWER_OFF 0x20
#define HT_LED_RAM_START 0x00
#define HT_LED_LEVER          0xE7

#define HT1633_TIME   4

char ht1633_dis_bitbuf[16];

//static WDOG_ID wdtimer_ht1633;
extern sem_t de_i2c_sem;

///////////////////////////////////////////////////

#define HT_SEG_A                0
#define HT_SEG_B                1
#define HT_SEG_C                2
#define HT_SEG_D                3
#define HT_SEG_E                4
#define HT_SEG_F                5
#define HT_SEG_M1             6
#define HT_SEG_M2             7
#define HT_SEG_M3             8
#define HT_SEG_M4             9
#define HT_SEG_M5             10
#define HT_SEG_M6             11
#define HT_SEG_M7             12
#define HT_SEG_M8             13
#define HT_SEG_OFF           (-1)
#define HT_SEG_TOTAL       14


////////////////////////////////////////////////////




#define   DIS_SEG_ADD_TOTAL  8
////////////////////////////////////////////////////
static char num_seg[NUM_TOTAL][DIS_SEG_ADD_TOTAL]=
{
{HT_SEG_A,HT_SEG_B,HT_SEG_C,HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_OFF,HT_SEG_OFF},///0
{HT_SEG_B,HT_SEG_C,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},     ////////////1
{HT_SEG_A,HT_SEG_B,HT_SEG_E,HT_SEG_D,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2      	////2
{HT_SEG_A,HT_SEG_B,HT_SEG_C,HT_SEG_D,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2    	////3
{HT_SEG_B,HT_SEG_C,HT_SEG_F,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////4
{HT_SEG_A,HT_SEG_F,HT_SEG_C,HT_SEG_D,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2    	////5
{HT_SEG_A,HT_SEG_C,HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF},//2    	////6
{HT_SEG_A,HT_SEG_B,HT_SEG_C,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////7
{HT_SEG_A,HT_SEG_B,HT_SEG_C,HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_M5,HT_SEG_M6},//2    	////8
{HT_SEG_A,HT_SEG_B,HT_SEG_C,HT_SEG_F,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2    	////9

{HT_SEG_A,HT_SEG_B,HT_SEG_C,HT_SEG_E,HT_SEG_F,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF},//2    	////A

{HT_SEG_C,HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2    	////b

{HT_SEG_A,HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////c

{HT_SEG_B,HT_SEG_C,HT_SEG_D,HT_SEG_E,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2    	////d

{HT_SEG_A,HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2    	////e

{HT_SEG_A,HT_SEG_E,HT_SEG_F,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////f

{HT_SEG_A,HT_SEG_C,HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2    	////g

{HT_SEG_B,HT_SEG_C,HT_SEG_E,HT_SEG_F,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2    	////h

{HT_SEG_M7,HT_SEG_M8,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////I

{HT_SEG_B,HT_SEG_C,HT_SEG_D,HT_SEG_E,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////J

{HT_SEG_M2,HT_SEG_M4,HT_SEG_M7,HT_SEG_M8,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////K

{HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////L
{HT_SEG_B,HT_SEG_C,HT_SEG_E,HT_SEG_F,HT_SEG_M1,HT_SEG_M2,HT_SEG_OFF,HT_SEG_OFF},//2    	////M
{HT_SEG_B,HT_SEG_C,HT_SEG_E,HT_SEG_F,HT_SEG_M1,HT_SEG_M4,HT_SEG_OFF,HT_SEG_OFF},//2    	////N
{HT_SEG_A,HT_SEG_B,HT_SEG_C,HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_OFF,HT_SEG_OFF},//2    	////O
{HT_SEG_A,HT_SEG_B,HT_SEG_E,HT_SEG_F,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2    	////P
{HT_SEG_A,HT_SEG_B,HT_SEG_C,HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_M4,HT_SEG_OFF},//2    	////Q
{HT_SEG_A,HT_SEG_E,HT_SEG_F,HT_SEG_M2,HT_SEG_M3,HT_SEG_M4,HT_SEG_OFF,HT_SEG_OFF},//2    	////R
{HT_SEG_A,HT_SEG_F,HT_SEG_C,HT_SEG_D,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF},//2    	////S
{HT_SEG_A,HT_SEG_M7,HT_SEG_M8,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////T
{HT_SEG_B,HT_SEG_C,HT_SEG_D,HT_SEG_E,HT_SEG_F,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////U
{HT_SEG_M1,HT_SEG_M2,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////V
{HT_SEG_B,HT_SEG_C,HT_SEG_E,HT_SEG_F,HT_SEG_M3,HT_SEG_M4,HT_SEG_OFF,HT_SEG_OFF},//2    	////W
{HT_SEG_M1,HT_SEG_M2,HT_SEG_M3,HT_SEG_M4,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////X
{HT_SEG_M1,HT_SEG_M2,HT_SEG_M8,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////Y
{HT_SEG_A,HT_SEG_D,HT_SEG_M3,HT_SEG_M4,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////Z

{HT_SEG_E,HT_SEG_D,HT_SEG_F,HT_SEG_M5,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////t

{HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2    	////-

{HT_SEG_A,HT_SEG_B,HT_SEG_C,HT_SEG_E,HT_SEG_F,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2 	////n

{HT_SEG_M5,HT_SEG_M6,HT_SEG_M7,HT_SEG_M8,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2 	////+

{HT_SEG_A,HT_SEG_D,HT_SEG_M5,HT_SEG_M6,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF,HT_SEG_OFF},//2	////All horizontal

{HT_SEG_B,HT_SEG_C,HT_SEG_E,HT_SEG_F,HT_SEG_M7,HT_SEG_M8,HT_SEG_OFF,HT_SEG_OFF},//2 ////All vertical


};
/////////////////////////////////////////////////////
static char seg_add[HT_SEG_TOTAL][5][2]=
{
{{10,0},{10,2},{10,4},{10,6},{11,0}},////HT_SEG_A
{{0,1},{0,3},{0,5},{0,7},{1,1}},////HT_SEG_b
{{2,1},{2,3},{2,5},{2,7},{3,1}},////HT_SEG_C
{{10,1},{10,3},{10,5},{10,7},{11,1}},////HT_SEG_D
{{2,0},{2,2},{2,4},{2,6},{3,0}},////HT_SEG_E
{{0,0},{0,2},{0,4},{0,6},{1,0}},////HT_SEG_F
{{4,0},{4,2},{4,4},{4,6},{5,0}},////HT_SEG_M1
{{4,1},{4,3},{4,5},{4,7},{5,1}},////HT_SEG_M2
{{8,0},{8,2},{8,4},{8,6},{9,0}},////HT_SEG_M3
{{8,1},{8,3},{8,5},{8,7},{9,1}},////HT_SEG_M4
{{6,0},{6,2},{6,4},{6,6},{7,0}},////HT_SEG_M5
{{6,1},{6,3},{6,5},{6,7},{7,1}},////HT_SEG_M6
{{12,0},{12,2},{12,4},{12,6},{13,0}},////HT_SEG_M7
{{12,1},{12,3},{12,5},{12,7},{13,1}},////HT_SEG_M8
};

////////////////////////////////////////////////////

static char icon[ICON_TOTAL][2]=
{
{14,0},/////usb
{14,4},/////BT
{14,2},/////AUX
{1,2},/////FM
{14,3},/////OP
{3,2},/////RE_ALL
{5,2},/////RE_ONE
{7,2},/////FM_ST
{11,2},/////MUTE
{9,2},/////UMUTE
{14,1},/////COLON1
{14,5},/////COLON2
{14,6},/////DOT

};

/********************************************



*********************************************/
void ht1633_set_icon(char ic_id,bool on_off)
{

	char buf_num, buf_bit;
	if((ic_id>ICON_TOTAL)||(ic_id<0))
	return;

	buf_num=icon[ic_id][0];
	buf_bit=icon[ic_id][1];

	if(on_off)
		ht1633_dis_bitbuf[buf_num]|=1<<buf_bit;
	else
		ht1633_dis_bitbuf[buf_num]&=~(1<<buf_bit);

	//printf("ht1633_icon_bitbuf[%d],con_bufbit=%d\r\n",buf_num,buf_bit);
}


void ht1633_set_num_leter(char num_index,char num_id,bool on_off)//////num_index 0------4,left-----right
{
	char i;
	char * num_seg_p;
	char seg_value;
	char buf_num, buf_bit;
	if((num_id>=NUM_TOTAL)||(num_id<0))
	return;

	num_seg_p=&num_seg[num_id][0];
	for(i=0;i<DIS_SEG_ADD_TOTAL;i++)
	{
		seg_value= *(num_seg_p+i);

		if((seg_value>=0)&&(seg_value<HT_SEG_TOTAL))
		{
			buf_num= seg_add[seg_value][num_index][0];
			buf_bit=seg_add[seg_value][num_index][1];

			if(on_off)
				ht1633_dis_bitbuf[buf_num]|=1<<buf_bit;
			else
				ht1633_dis_bitbuf[buf_num]&=~(1<<buf_bit);
		}
	}

}


/*********************************************


*******************************************/
void ht1633_clear_disbuf(void)
{
	int i;
	for(i=0;i<16;i++)
	{
		ht1633_dis_bitbuf[i]=0x00;
	}
}
/*********************************************


*******************************************/
void ht1633_full_disbuf(void)
{
	int i;
	for(i=0;i<16;i++)
	{
		ht1633_dis_bitbuf[i]=0xff;
	}
}

/****************************************************************************

 * Public Functions
 ****************************************************************************/


static bool ht1633_send_device_addr(bool w_r)
{
    iic_send_byte(HT_I2C_ADD|(w_r?1:0), I2C_CLK_PIN, I2C_DATA_PIN);
    return iic_recv_ack(I2C_CLK_PIN, I2C_DATA_PIN);
}



bool ht1633_write_byte(unsigned char byte)
{
	bool ret = false;
	//发送开始信号
	iic_start(I2C_CLK_PIN, I2C_DATA_PIN);
	//发送带写信号的设备ID
	if(ht1633_send_device_addr(false))
	{
		//发送带写信号的设备ID成功
		//发送寄存器地址
		iic_send_byte(byte, I2C_CLK_PIN, I2C_DATA_PIN);
		ret = iic_recv_ack(I2C_CLK_PIN, I2C_DATA_PIN);
	}
	//停止之前的操作
	iic_stop(I2C_CLK_PIN, I2C_DATA_PIN);
	return ret;
}

char ht1633_write_nbyte(char com,char bye,char *p)
{
	int i;
	bool ret = false;

	//发送开始信号
	iic_start(I2C_CLK_PIN, I2C_DATA_PIN);
	//发送带写信号的设备ID
	if(ht1633_send_device_addr(false))
	{
		//发送带写信号的设备ID成功
		//发送寄存器地址

		iic_send_byte(com, I2C_CLK_PIN, I2C_DATA_PIN);
		ret = iic_recv_ack(I2C_CLK_PIN, I2C_DATA_PIN);

		if(ret)
		{
			for (i = 0; i < bye; i++)
			{
				iic_send_byte(*(p+i), I2C_CLK_PIN, I2C_DATA_PIN);
				ret = iic_recv_ack(I2C_CLK_PIN, I2C_DATA_PIN);
				if(ret==false)
					break;
			}
		}
		else
		{
			printf("i2c_err\r\n");
		}
	}
	else
	{
		printf("ht1633_send_device_addr_err\r\n");
	}
	//停止之前的操作
	iic_stop(I2C_CLK_PIN, I2C_DATA_PIN);
	return ret;

}


void ht1633_init(void)
{
	printf("ht1633_init\r\n");
	ht1633_clear_disbuf();


	iic_init(I2C_CLK_PIN, I2C_DATA_PIN);
	ht1633_write_byte(HT_LED_POWER_ON);
	ht1633_write_byte(HT_LED_ON);
	ht1633_write_byte(HT_LED_LEVER);

	ht1633_write_nbyte(HT_LED_RAM_START,16,ht1633_dis_bitbuf);

	ht1633_write_byte(HT_LED_ON);

#if 0
    sc6138_timer_stop(4);
    //启动LED屏扫描计时器
    sc6138_timer_initialize(4, 20000, check_seg);
    //启动LED计时
    sc6138_timer_start(4);
#endif

}

extern unsigned char soft_updata_flag;
/**************************************************



***************************************************/
void ht1633_updata_display(void)
{
	if(soft_updata_flag)
		return;
	// sem_wait(&de_i2c_sem);
	ht1633_write_nbyte(HT_LED_RAM_START,16,ht1633_dis_bitbuf);
	ht1633_write_byte(HT_LED_ON);
	// sem_post(&de_i2c_sem);
 }





/*****************************************




*****************************************/



void check_seg(void)
{
	static char seg_cout=0;
	static char seg_num=0;
	static int time_cout;

	time_cout++;
	if(time_cout>80)
	{
		time_cout=0;

		ht1633_clear_disbuf();

#if 0
		ht1633_dis_bitbuf[seg_num]|=1<<seg_cout;

		printf("ht1633[%d],bufbit=%d\r\n",seg_num,seg_cout);
		seg_cout++;
		if(seg_cout>7)
		{
		seg_cout=0;
		seg_num++;
		if(seg_num>15)
		seg_num=0;
		}
#endif

		seg_cout++;
		if(seg_cout>37)
			seg_cout=0;

		if(seg_cout==37)
		{
			char i;
			for(i=0;i<ICON_TOTAL;i++)
				ht1633_set_icon(i,1);
		}
		else
		{
			seg_cout=1;

			ht1633_set_num_leter(0,seg_cout,1);
			ht1633_set_num_leter(1,seg_cout,1);
			ht1633_set_num_leter(2,seg_cout,1);
			ht1633_set_num_leter(3,seg_cout,1);
			ht1633_set_num_leter(4,seg_cout,1);
		}
		ht1633_updata_display();

	}

#if 0

	if (wdtimer_ht1633 != NULL)
	{
	wd_cancel(wdtimer_ht1633);
	}
	else
	{
	wdtimer_ht1633 = wd_create();
	}


	wd_start(wdtimer_ht1633, 3000, check_seg, 0);

#endif


}

