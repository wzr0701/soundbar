#ifndef __LED7_H__
#define __LED7_H__
/****************************************************************************
 * Public Functions
 ****************************************************************************/
 #if 0
#define NUM_0  0
#define NUM_1  1
#define NUM_2  2
#define NUM_3  3
#define NUM_4  4
#define NUM_5  5
#define NUM_6  6
#define NUM_7  7
#define NUM_8  8
#define NUM_9  9
#define NUM_A  10
#define NUM_B  11
#define NUM_C  12
#define NUM_D  13
#define NUM_E  14
#define NUM_F  15
#define NUM_G  16
#define NUM_H  17
#define NUM_I  18
#define NUM_J  19
#define NUM_K  20
#define NUM_L  21
#define NUM_M  22   //can't display
#define NUM_N  23
#define NUM_O  24
#define NUM_P  25
#define NUM_Q  26
#define NUM_R  27
#define NUM_S  28
#define NUM_t  29
#define NUM_T  29   //the same as 't'
#define NUM_U  30
#define NUM_V  31   //the same as U
#define NUM_W  32   //can't display
#define NUM_X  33   //can't display
#define NUM_y  34
#define NUM_Z  35
#define NUM__  36
#define NUM_OFF  37   //'-',žÃÐòÁÐ×îŽóÖµ£¬³¬³öŽËÖµ°ŽŽËŽŠÀí
#define NUMBER_TOTAL 38

#endif

#define NUM_0  0
#define NUM_1  1
#define NUM_2  2
#define NUM_3  3
#define NUM_4  4
#define NUM_5  5
#define NUM_6  6
#define NUM_7  7
#define NUM_8  8
#define NUM_9  9
#define NUM_A  10
#define NUM_B  11
//#define NUM_C  12
//#define NUM_D  13
#define NUM_E  12
//define NUM_F  15
//#define NUM_G  16
#define NUM_H  13
//#define NUM_I  NUM_1
//#define NUM_J  19
//#define NUM_K  20
#define NUM_L  14
//#define NUM_M  22   //can't display
//#define NUM_N  23
//#define NUM_O  24
#define NUM_P  15
//#define NUM_Q  26
//#define NUM_R  27
//#define NUM_S  28
//#define NUM_t  29
//#define NUM_T  29   //the same as 't'
#define NUM_U  16
#define NUM_V  17   //the same as U
#define NUM__  18   //can't display
#define NUM_D  19   //can't display
#define NUM_F  20
#define NUM_N  21
#define NUM_T  22
#define NUM_OFF  (-1)   //'-',žÃÐòÁÐ×îŽóÖµ£¬³¬³öŽËÖµ°ŽŽËŽŠÀí
#define NUMBER_TOTAL 23













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




void led7_set_usb(bool disp);
void led7_set_sd(bool disp);
void led7_set_fm(bool disp);
void led7_set_play_pause(bool disp);
void led7_set_replay(bool disp);
void led7_set_bat_level(char level);







#endif