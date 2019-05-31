#ifndef __HT1633__H__
#define __HT1633__H__


#define   ICON_USB        0
#define   ICON_BT        1
#define   ICON_AUX     2
#define   ICON_FM       3
#define   ICON_OPTI     4
#define   ICON_REPEAT_ALL   5
#define   ICON_REPEAT_ONE  6
#define   ICON_FM_ST            7
#define   ICON_VOL_MUTE          8
#define   ICON_VOL_UMUTE          9
#define   ICON_CLON_1          10
#define   ICON_CLON_2          11
#define   ICON_DOT          12
#define   ICON_TOTAL        13




/////////////////////////////////////////////////////////////////////
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
#define NUM_Y  34
#define NUM_Z  35
#define NUM_t   36
#define NUM__  37
#define NUM_n 38
#define NUM_ADD 39
#define NUM_HOR 40
#define NUM_VER 41



#define NUM_OFF  (-1)

#define NUM_TOTAL 42



void ht1633_full_disbuf(void);
void ht1633_clear_disbuf(void);
void ht1633_init(void);
void check_seg(void);
void ht1633_set_num_leter(char num_index,char num_id,bool on_off);
void ht1633_set_icon(char ic_id,bool on_off);

void ht1633_updata_display(void);


#endif
