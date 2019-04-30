#ifndef SL_UI_CMD_DEAL_H__
#define SL_UI_CMD_DEAL_H__


#define TOUCH_KEY_ENABLE    1

#define EV_CCHIP_KEY  0x72
#define CCHIP_KEY_SHORT_UP    0X1000
#define CCHIP_KEY_LONG    0X2000
#define CCHIP_KEY_HOLD    0X4000
#define CCHIP_KEY_LONG_UP    0X8000

#define CCHIP_KEY_LONG_TIME 200//////200*10==2S
#define CCHIP_KEY_HOLD_START_TIME 300//////300*10==3S
#define CCHIP_KEY_HOLD_KEEP_TIME 80//////80*10==0.8S


//EEPROM、IRTC-RAM 地址定义

#define MEM_WORK_MODE     0

#define MEM_FM_FREQUENCY        10   //2 byte   为兼容760-1080的频率，因1080-760=320已超出1个字节255的范围
#define MEM_FRE_TOTAL_NUM       11  //电台总数
#define MEM_CUR_FRE_NUM        12  //当前电台序号
#define MEM_SAVE_CHANNEL       13  //(1080-875+1)/8=26 byte  每个电台的频率值，每一位存一个电台

#define MEM_MIX_VOL                100

#define MEM_TREBLE_LEVEL      200
#define MEM_BASS_LEVEL      300

#define MEM_USB_NUM  500

#define BASS_MODE 0
#define TREBLE_MODE 2



#define INPUT_MAX   9999


#define EQ_NORMAL 1
#define EQ_NIGHT          2
#define EQ_3D             3
#define EQ_CLEAR          4

void select_eq_mode(void);

void hdmi_send_unmute(void);


unsigned char ui_handle_cmd_com(ui_cmd_t *cmd);

void source_mode_bt(void);
void source_mode_usb(void);

void source_mode_sd(void);
void source_mode_fm(void);

void source_mode_aux(void);
void source_mode_rca(void);

void source_mode_spdifin(void);

void source_mode_hdmi(void);
void source_mode_coaxial(void);
void source_mode_test(void);

void source_mode_start(void);

void  save_player_info(void);
void  read_player_info(void);

void  save_mix_vol(void);
void  read_mix_vol(void);

void  save_usb_num(int file_index);
int  read_usb_num(void);



#endif




