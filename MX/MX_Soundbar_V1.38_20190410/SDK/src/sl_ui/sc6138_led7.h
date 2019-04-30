#ifndef __LED7_H__
#define __LED7_H__
/****************************************************************************
 * Public Functions
 ****************************************************************************/

#define   ICON_USB        0
#define   ICON_BT        1
#define   ICON_AUX     2
#define   ICON_FM       3
#define   ICON_OPTI     4
#define   ICON_COA 5
#define   ICON_HDMI 6
#define   ICON_EQ 7
#define   ICON_REPEAT_ALL   8
#define   ICON_REPEAT_ONE  9
#define   ICON_FM_ST            10
#define   ICON_VOL_MUTE          11
#define   ICON_VOL_UMUTE 12
#define   ICON_CLON_2          13
#define   ICON_DOT          14
#define   ICON_TOTAL        15


#define NUM_0		0
#define NUM_1		1
#define NUM_2		2
#define NUM_3		3
#define NUM_4		4
#define NUM_5		5
#define NUM_6		6
#define NUM_7		7
#define NUM_8		8
#define NUM_9		9
#define NUM_A		10
#define NUM_B		11
#define NUM_E		12
#define NUM_H		13
#define NUM_L		14
#define NUM_P		15
#define NUM_U		16
#define NUM_V		17
#define NUM__		18
#define NUM_D		19
#define NUM_F		20
#define NUM_N		21
#define NUM_T		22
#define NUM_LINE_DB	23  //二道划线
#define NUM_LINE	24  //下划线
#define NUM_C		25  //C
#define NUM_OFF		(-1)
#define NUMBER_TOTAL  26


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
void led7_clear_display(void);

/****************************************************************************
 * Name: led7_init
 *
 * Description:
 *   init the 7 segment LED display
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void led7_init(void);


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
void led7_off(void);

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
void led7_open(void);

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
void led7_set_colon(bool disp);

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
void led7_set_display(int index, char disp);

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
void led7_update_bitbuf(void);


void led7_set_aux(bool disp);
void led7_set_usb(bool disp);
void led7_set_bt(bool disp);
void led7_set_opt(bool disp);
void led7_set_coa(bool disp);
void led7_set_hdmi(bool disp);
void led7_set_eq(bool disp);





#endif