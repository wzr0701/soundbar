/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/arch.h>
#include <nuttx/module.h>
#include <silan_addrspace.h>
#include <silan_resources.h>
#include <silan_timer.h>
#include <sys/types.h>
#include <zhuque_bsp_gpio.h>
#include "sc6138_led7.h"
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/*LED 引脚数量*/
#define LED_PIN_NUM 7
/*LED 扫描数量*/
#define LED_SCAN_NUM 7
/*可显示的类型*/
#define DISP_TYPE  NUMBER_TOTAL
/*数码管数量*/
#define DIGITAL_NUM 4



/*
 * LED驱动GPIO
 * 32   --  2_0
 * 33   --  2_1
 * 34   --  2_2
 * 35   --  2_3
 * 36   --  2_4
 * 37   --  2_5
 * 38   --  2_6
 */
#define LED_PIN1 32
#define LED_PIN2 33
#define LED_PIN3 34
#define LED_PIN4 35
#define LED_PIN5 36
#define LED_PIN6 37
#define LED_PIN7 38







/*刷屏使用的计时器*/
#define LED7_TIMER          4
/*刷屏的周期*/
#define LED7_SCAN_CYCLE  1000   //us
/****************************************************************************
 * Public Types
 ****************************************************************************/
/*
 * 7段LED屏显示信息结构体
 */
struct led7_dis_info
{
    char digital_num[DIGITAL_NUM]; //display number value
    char colon;
    char dot;
	char aux;
	char opt;
	char coa;
	char hdmi;
    char usb;
	char bt;
	char eq;
    char sd;
    char play_pause;
    char replay;
    char bat_level;


};

/****************************************************************************
 * Public Data
 ****************************************************************************/
/*LED显示缓存*/
char led7_dis_bitbuf[LED_SCAN_NUM];
/*LED数据缓存*/
struct led7_dis_info led7_disbuf;
/*LED引脚编号*/
static const int led7IONum[LED_PIN_NUM] = {
    LED_PIN1,
    LED_PIN2,
    LED_PIN3,
    LED_PIN4,
    LED_PIN5,
    LED_PIN6,
    LED_PIN7
};
/*显示段码*/


static const int ledDispSeg[][DISP_TYPE][LED_SCAN_NUM] = {
/*第0列*/
{
     {0x1a,0x01,0x01,0x01,0x00,0x00,0x00},//0 abcdef
     {0x08,0x00,0x01,0x00,0x00,0x00,0x00},//1 bc
     {0x14,0x01,0x01,0x01,0x00,0x00,0x00},//2 abdeg
     {0x1c,0x01,0x01,0x00,0x00,0x00,0x00},//3 abcdg
     {0x0e,0x00,0x01,0x00,0x00,0x00,0x00},//4 bcfg
     {0x1e,0x01,0x00,0x00,0x00,0x00,0x00},//5 afgcd
     {0x1e,0x01,0x00,0x01,0x00,0x00,0x00},//6 acdefg
     {0x08,0x01,0x01,0x00,0x00,0x00,0x00},//7 abc
     {0x1e,0x01,0x01,0x01,0x00,0x00,0x00},//8 abcdefg
	 {0x1e,0x01,0x01,0x00,0x00,0x00,0x00},//9 abcdfg
     {0x0e,0x01,0x01,0x01,0x00,0x00,0x00},//a  abcefg
     {0x1e,0x00,0x00,0x01,0x00,0x00,0x00},//b  cdefg
     {0x16,0x01,0x00,0x01,0x00,0x00,0x00},//e  adefg
     {0x0e,0x00,0x01,0x01,0x00,0x00,0x00},//h  bcefg
     {0x12,0x00,0x00,0x01,0x00,0x00,0x00},//l def
     {0x06,0x01,0x01,0x01,0x00,0x00,0x00},//p abefg
     {0x1a,0x00,0x01,0x01,0x00,0x00,0x00},//u bcdef
     {0x1a,0x00,0x01,0x01,0x00,0x00,0x00},//v u
     {0x04,0x00,0x00,0x00,0x00,0x00,0x00},//- g
     {0x18,0x00,0x01,0x01,0x01,0x00,0x00},//d bcdeg
     {0x06,0x01,0x00,0x01,0x00,0x00,0x00},//f aefg
     {0x0a,0x01,0x01,0x01,0x00,0x00,0x00},//n abcef
     {0x16,0x00,0x00,0x01,0x00,0x00,0x00},//t defg
     {0x14,0x00,0x00,0x00,0x00,0x00,0x00},//二段横线 dg     
	 {0x10,0x00,0x00,0x00,0x00,0x00,0x00},//一段横线 d	 
	 {0x12,0x01,0x00,0x01,0x00,0x00,0x00},//c adef
},
/*第1列*/
{
   {0x00,0x14,0x02,0x02,0x02,0x02,0x00},//0 abcdef
   {0x00,0x10,0x00,0x02,0x00,0x00,0x00},//1 bc
   {0x00,0x08,0x02,0x02,0x02,0x02,0x00},//2 abdeg
   {0x00,0x18,0x02,0x02,0x00,0x02,0x00},//3 abcdg
   {0x00,0x1c,0x00,0x02,0x00,0x00,0x00},//4 bcfg
   {0x00,0x1c,0x02,0x00,0x00,0x02,0x00},//5 acdfg
   {0x00,0x1c,0x02,0x00,0x02,0x02,0x00},//6 acdefg
   {0x00,0x10,0x02,0x02,0x00,0x00,0x00},//7 abc
   {0x00,0x1c,0x02,0x02,0x02,0x02,0x00},//8 abcdefg
   {0x00,0x1c,0x02,0x02,0x00,0x02,0x00},//9 abcdfg
   {0x00,0x1c,0x02,0x02,0x02,0x00,0x00},//a abcefg
   {0x00,0x1c,0x00,0x00,0x02,0x02,0x00},//b cdefg
   {0x00,0x0c,0x02,0x00,0x02,0x02,0x00},//e adefg
   {0x00,0x1c,0x00,0x02,0x02,0x00,0x00},//h bcefg
   {0x00,0x04,0x00,0x00,0x02,0x02,0x00},//l def
   {0x00,0x0c,0x02,0x02,0x02,0x00,0x00},//p abefg
   {0x00,0x14,0x00,0x02,0x02,0x02,0x00},//u bcdef
   {0x00,0x14,0x00,0x02,0x02,0x02,0x00},//v u
   {0x00,0x08,0x00,0x00,0x00,0x00,0x00},//- g
   {0x00,0x18,0x00,0x02,0x02,0x02,0x00},//d bcdeg
   {0x00,0x0c,0x02,0x00,0x02,0x00,0x00},//f aefg
   {0x00,0x18,0x02,0x02,0x02,0x00,0x00},//n abcef
   {0x00,0x0c,0x00,0x00,0x02,0x02,0x00},//t defg
   {0x00,0x08,0x00,0x00,0x00,0x02,0x00},//二段横线 dg
   {0x00,0x00,0x00,0x00,0x00,0x02,0x00},//一段横线 d
   {0x00,0x04,0x02,0x00,0x02,0x02,0x00},//c adef
},
/*第2列*/
{
  {0x20,0x00,0x28,0x10,0x0c,0x00,0x00},//0 abcdef
  {0x00,0x00,0x00,0x00,0x0c,0x00,0x00},//1 bc
  {0x20,0x00,0x30,0x10,0x04,0x00,0x00},//2 abdeg
  {0x20,0x00,0x10,0x10,0x0c,0x00,0x00},//3 abcdg
  {0x00,0x00,0x18,0x00,0x0c,0x00,0x00},//4 bcfg
  {0x20,0x00,0x18,0x10,0x08,0x00,0x00},//5 acdfg
  {0x20,0x00,0x38,0x10,0x08,0x00,0x00},//6 acdefg
  {0x00,0x00,0x00,0x10,0x0c,0x00,0x00},//7 abc
  {0x20,0x00,0x38,0x10,0x0c,0x00,0x00},//8 abcdefg
  {0x20,0x00,0x18,0x10,0x0c,0x00,0x00},//9 abcdfg
  {0x00,0x00,0x38,0x10,0x0c,0x00,0x00},//a abcefg
  {0x20,0x00,0x38,0x00,0x08,0x00,0x00},//b cdefg
  {0x20,0x00,0x38,0x10,0x00,0x00,0x00},//e adefg
  {0x00,0x00,0x38,0x00,0x0c,0x00,0x00},//h bcefg
  {0x20,0x00,0x28,0x00,0x00,0x00,0x00},//l def
  {0x00,0x00,0x38,0x10,0x04,0x00,0x00},//p abefg
  {0x20,0x00,0x28,0x00,0x0c,0x00,0x00},//u bcdef
  {0x20,0x00,0x28,0x00,0x0c,0x00,0x00},//v u
  {0x00,0x00,0x10,0x00,0x00,0x00,0x00},//- g
  {0x20,0x00,0x30,0x00,0x0c,0x00,0x00},//d bcdeg
  {0x00,0x00,0x38,0x10,0x00,0x00,0x00},//f aefg
  {0x00,0x00,0x28,0x10,0x0c,0x00,0x00},//n abcef
  {0x20,0x00,0x38,0x00,0x00,0x00,0x00},//t defg   
  {0x20,0x00,0x10,0x00,0x00,0x00,0x00},//二段横线 dg
  {0x20,0x00,0x00,0x00,0x00,0x00,0x00},//一段横线 d
  {0x20,0x00,0x28,0x10,0x00,0x00,0x00},//c adef
},
/*第3列*/
{
  {0x00,0x00,0x00,0x20,0x20,0x58,0x20},//0 abcdef
  {0x00,0x00,0x00,0x00,0x00,0x10,0x20},//1 bc
  {0x00,0x00,0x00,0x20,0x00,0x48,0x30},//2 abdeg
  {0x00,0x00,0x00,0x20,0x00,0x50,0x30},//3 abcdg
  {0x00,0x00,0x00,0x00,0x20,0x10,0x30},//4 bcfg
  {0x00,0x00,0x00,0x20,0x20,0x50,0x10},//5 acdfg
  {0x00,0x00,0x00,0x20,0x20,0x58,0x10},//6 acdefg
  {0x00,0x00,0x00,0x00,0x00,0x50,0x20},//7 abc
  {0x00,0x00,0x00,0x20,0x20,0x58,0x30},//8 abcdefg
  {0x00,0x00,0x00,0x20,0x20,0x50,0x30},//9 abcdfg
  {0x00,0x00,0x00,0x00,0x20,0x58,0x30},//a abcefg
  {0x00,0x00,0x00,0x20,0x20,0x18,0x10},//b cdefg
  {0x00,0x00,0x00,0x20,0x20,0x48,0x10},//e adefg
  {0x00,0x00,0x00,0x00,0x20,0x18,0x30},//h bcefg
  {0x00,0x00,0x00,0x20,0x20,0x08,0x00},//l def
  {0x00,0x00,0x00,0x00,0x20,0x48,0x30},//p abefg
  {0x00,0x00,0x00,0x20,0x20,0x18,0x20},//u bcdef
  {0x00,0x00,0x00,0x20,0x20,0x18,0x20},//v u
  {0x00,0x00,0x00,0x00,0x00,0x00,0x10},//- g
  {0x00,0x00,0x00,0x20,0x00,0x18,0x30},//d bcdeg
  {0x00,0x00,0x00,0x00,0x20,0x48,0x10},//f aefg
  {0x00,0x00,0x00,0x00,0x00,0x58,0x30},//n abcef
  {0x00,0x00,0x00,0x20,0x20,0x08,0x10},//t defg 
  {0x00,0x00,0x00,0x20,0x00,0x00,0x10},//二段横线 dg
  {0x00,0x00,0x00,0x20,0x00,0x00,0x00},//一段横线 d
  {0x00,0x00,0x00,0x20,0x20,0x48,0x00},//c adef
}
};








/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
void led7_clear_display(void);
void led7_update_bitbuf(void);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: led_7p7s_scan
 *
 * Description:
 *    7段LED屏显示扫描程序
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
static void led_7p7s_scan(void)
{

    static int led_fresh_cnt = 0;

    led7_clear_display();

    led_fresh_cnt++;
    if (led_fresh_cnt >= LED_SCAN_NUM)
    {
        led_fresh_cnt = 0;
    }

    char seg_code = led7_dis_bitbuf[led_fresh_cnt];

    if(seg_code & 0x7f)
    {   //需要显示
        int i = 0;
        int pin = led7IONum[led_fresh_cnt];

        //扫描口输出低
        zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PULLING_NONE);
        zhuque_bsp_gpio_set_value(pin, GPIO_VALUE_LOW);

        for(i=0; i < LED_PIN_NUM; ++i)
        {
            if (seg_code & (1 << i))
            {
                pin = led7IONum[i];
                zhuque_bsp_gpio_set_mode(pin, GPIO_OUT, PULLING_NONE);
                zhuque_bsp_gpio_set_value(pin, GPIO_VALUE_HIGH);
            }
        }
    }

}

/****************************************************************************
 * Name: led7_clear_display
 *
 * Description:
 *    清除LED屏显示
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void led7_clear_display(void)
{

    int i = 0;
    //初始化IO口模式和状态
    for(i=0;i<LED_PIN_NUM;++i)
    {
        zhuque_bsp_gpio_set_mode(led7IONum[i], GPIO_IN, PULLING_NONE);
        zhuque_bsp_gpio_set_value(led7IONum[i], GPIO_VALUE_LOW);
    }

}
/****************************************************************************
 * Name: led7_clear_bitbuf
 *
 * Description:
 *   æ¸…é™¤7æ®µLEDå±�çš„æ˜¾ç¤ºç¼“å†²
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void led7_clear_bitbuf(void)
{
    int i;
    for (i = 0; i < LED_SCAN_NUM; i++)
    {
        led7_dis_bitbuf[i] = 0;
    }
}

/****************************************************************************
 * Name: led7_clear_disbuf
 *
 * Description:
 *   清除数据缓存区
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void led7_clear_disbuf(void)
{
    led7_disbuf.digital_num[0] = 0;
    led7_disbuf.digital_num[1] = 0;
    led7_disbuf.digital_num[2] = 0;
    led7_disbuf.digital_num[3] = 0;

    led7_disbuf.colon = 0;
    led7_disbuf.dot = 0;
	led7_disbuf.bt = 0;
	led7_disbuf.aux = 0;
	led7_disbuf.opt = 0;
	led7_disbuf.coa = 0;
	led7_disbuf.hdmi = 0;
	led7_disbuf.usb = 0;
	led7_disbuf.eq = 0;
}

/****************************************************************************
 * Name: led7_init
 *
 * Description:
 *   7段LED屏驱动初始化
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void led7_init(void)
{
    //清除缓存数据
    led7_clear_disbuf();
    led7_clear_bitbuf();
    //更新显示
    led7_update_bitbuf();

    //停止计时
    silan_timer_stop(LED7_TIMER);

    led7_clear_display();
}

/****************************************************************************
 * Name: led7_off
 *
 * Description:
 *   关闭LED屏
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void led7_off(void)
{
    //停止LED刷新TIMER
    silan_timer_stop(4);

    //清除缓存数据
    led7_clear_disbuf();
    led7_clear_bitbuf();
    //清除显示
    led7_clear_display();
}

/****************************************************************************
 * Name: led7_open
 *
 * Description:
 *   打开LED屏
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void led7_open(void)
{

    //停止计时
    silan_timer_stop(LED7_TIMER);
    //启动LED屏扫描计时器
    silan_timer_initialize(LED7_TIMER, LED7_SCAN_CYCLE, led_7p7s_scan);
    //启动LED计时
    silan_timer_start(LED7_TIMER);

}

/****************************************************************************
 * Name: led7_set_colon
 *
 * Description:
 *   显示两点
 *
 * Parameters:
 *   disp  true显示两点，false隐藏两点
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void led7_set_colon(bool disp)
{
    led7_disbuf.colon = disp;
}

void led7_set_dot(bool disp)
{
    led7_disbuf.dot = disp;
}


void led7_set_aux(bool disp)
{
    led7_disbuf.aux = disp;
}

void led7_set_usb(bool disp)
{
    led7_disbuf.usb = disp;
}

void led7_set_bt(bool disp)
{
    led7_disbuf.bt = disp;
}

void led7_set_opt(bool disp)
{
    led7_disbuf.opt = disp;
}

void led7_set_coa(bool disp)
{
    led7_disbuf.coa = disp;
}

void led7_set_hdmi(bool disp)
{
    led7_disbuf.hdmi = disp;
}

void led7_set_eq(bool disp)
{
    led7_disbuf.eq = disp;
}


/*******************************************************************




*******************************************************************/
void led7_update_char(void)
{
	
/////////////////:///////////////////////
	if(led7_disbuf.colon)
	{
		led7_dis_bitbuf[3] |= 0x04;
	}      
	else
	{
		led7_dis_bitbuf[3] &= ~0x04;
	}
////////////////////////////////////////     

/////////////////.//////////////////////

	if(led7_disbuf.dot)
	{
		led7_dis_bitbuf[4] |= 0x40;
	}
	else
	{
		led7_dis_bitbuf[4] &= ~0x40;
	}
////////////////////////////////////////////  

/////////////////aux//////////////////////
	if(led7_disbuf.aux )
	{
		led7_dis_bitbuf[5] |= 0x01;
	}
	else
	{
		led7_dis_bitbuf[5] &=~0x01;
	}
///////////////////////////////////////////

/////////////////usb//////////////////////
	if(led7_disbuf.usb )
	{
		led7_dis_bitbuf[5] |= 0x04;
	}
	else
	{
		led7_dis_bitbuf[5] &=~0x04;
	}
///////////////////////////////////////////

/////////////////bt//////////////////////
	if(led7_disbuf.bt )
	{
		led7_dis_bitbuf[1] |= 0x20;
	}
	else
	{
		led7_dis_bitbuf[1] &=~0x20;
	}
///////////////////////////////////////////

/////////////////opt//////////////////////
	if(led7_disbuf.opt )
	{
		led7_dis_bitbuf[4] |= 0x01;
	}
	else
	{
		led7_dis_bitbuf[4] &=~0x01;
	}
///////////////////////////////////////////

/////////////////coa//////////////////////
	if(led7_disbuf.coa )
	{
		led7_dis_bitbuf[2] |= 0x40;
	}
	else
	{
		led7_dis_bitbuf[2] &=~0x40;
	}
///////////////////////////////////////////

/////////////////hdmi//////////////////////
	if(led7_disbuf.hdmi )
	{
		led7_dis_bitbuf[3] |= 0x40;
	}
	else
	{
		led7_dis_bitbuf[3] &=~0x40;
	}
///////////////////////////////////////////

/////////////////eq//////////////////////
	if(led7_disbuf.eq )
	{
		led7_dis_bitbuf[6] |= 0x04;
	}
	else
	{
		led7_dis_bitbuf[6] &=~0x04;
	}
///////////////////////////////////////////
}





/****************************************************************************
 * Name: led7_set_display
 *
 * Description:
 *   设置第X个LED显示的内容
 *
 * Parameters:
 *   index  LED的序号0~3
 *   disp   要显示的内容
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void led7_set_display(int index, char disp)
{
    if(index >= 0 && index < DIGITAL_NUM)
    {
        led7_disbuf.digital_num[index] = disp;
    }
}

/****************************************************************************
 * Name: led7_update_bitbuf
 *
 * Description:
 *   更新显示缓存数据
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
//从左到右0-1
//bit    6    5    4    3    2    1    0
//buf 0--F0   E0   D0   C0   B0   A0   0
//buf 1--F1   E1   D1   C1   B1   0    A1
//buf 2--F2   E2   D2   C2   0    B2   A2
//buf 3--F3   E3   D3   0    C3   B3   A3
//buf 4--0    0    0    G3   G2   G1   G0
void led7_update_bitbuf(void)
{
    int i, j, type;

    led7_clear_bitbuf();

    for (j = 0; j < DIGITAL_NUM; ++j)
    {
        type = led7_disbuf.digital_num[j];
        if(0 <= type && type < DISP_TYPE)
        {
            for(i = 0; i < LED_SCAN_NUM; ++i)
            {
                led7_dis_bitbuf[i] |= ledDispSeg[j][type][i];
            }
        }
    }

	led7_update_char();

}
