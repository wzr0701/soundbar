/*
 * input_key.h 
 *
 * MaintainedBy:    XuHang <xuhang@silan.com.cn>
 *                  QinLing <qinling@silan.com.cn>
 */


#ifndef __CONFIG_SILAN_INCLUDE_INPUT_KEY_H
#define __CONFIG_SILAN_INCLUDE_INPUT_KEY_H

#include <nuttx/input.h>
#include "zhuque_bsp_gpio.h"
#include "silan_gpio.h"
#include "board.h"

#define SC6138_A4_BUTTON_NEW
//#define SC6138_DOUBLE_CLICK
//SC6138_CHANNEL_EN

typedef enum silan_input_key_code {
    SILAN_INPUTKEY_CODE_BUTTON_UP       = 0,    /* triggered when press button down on a long period */
    SILAN_INPUTKEY_CODE_BUTTON_DOWN,
    SILAN_INPUTKEY_CODE_BUTTON_CLICK,
    SILAN_INPUTKEY_CODE_BUTTON_SELECT,
    SILAN_INPUTKEY_CODE_BUTTON_CHANNEL,
    SILAN_INPUTKEY_CODE_BUTTON_DOUBLE_CLICK,
} input_key_code_e;

#ifndef CONFIG_UI
typedef enum silan_input_key_value {
    SILAN_INPUTKEY_VAL_BUTTON_RESET = GPIO1_18,
    SILAN_INPUTKEY_VAL_BUTTON_PLAY_CTRL = GPIO2_18,
    SILAN_INPUTKEY_VAL_BUTTON_VOICE = 0xff,
    SILAN_INPUTKEY_VAL_BUTTON_VOICE_CTRL = (0xff|0xf00),
    SILAN_INPUTKEY_VAL_BUTTON_VOICE_CTRL_PRE = (0xff|0xe00),
    SILAN_INPUTKEY_VAL_BUTTON_SOUNDVOL_INC = GPIO2_19,
    SILAN_INPUTKEY_VAL_BUTTON_SOUNDVOL_DEC = GPIO2_20,
#ifndef SC6138_A4_BUTTON_NEW
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_0 = GPIO2_0,
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_1 = GPIO2_1,
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_2 = GPIO2_2,
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_3 = GPIO2_3,
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_4 = GPIO2_4,
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_5 = GPIO2_5,
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_6 = GPIO2_6,
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_7 = GPIO2_14,
#else
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_MID = GPIO2_4,
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_NEXT = GPIO2_3,
    SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_PREV = GPIO2_5,
    SILAN_INPUTKEY_VAL_BUTTON_MODE = 0xff - 1,
    SILAN_INPUTKEY_VAL_BUTTON_FUNC = 0xff - 2,
    SILAN_INPUTKEY_VAL_BUTTON_EQ = 0xff - 3
#endif
} input_key_value_e;
#endif

#define BUTTON_PLAY(val)    (val == (SILAN_INPUTKEY_VAL_BUTTON_PLAY_CTRL))
#define BUTTON_RESET(val)   (val == (SILAN_INPUTKEY_VAL_BUTTON_RESET))
#define BUTTON_VOLUME(val)  (val == SILAN_INPUTKEY_VAL_BUTTON_SOUNDVOL_DEC)

#ifdef SC6138_A4_BUTTON_NEW
#define BUTTON_CHANNEL(val) (val == SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_MID || val == SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_PREV || val == SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_NEXT)
#else
#define BUTTON_CHANNEL(val) (val == SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_7 || (val >= SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_0 && val <= SILAN_INPUTKEY_VAL_BUTTON_CHANNEL_6))
#endif


#endif /* __CONFIG_SILAN_INCLUDE_INPUT_KEY_H */
