#ifndef SL_UI_CMD_DEAL_H__
#define SL_UI_CMD_DEAL_H__











//EEPROM、IRTC-RAM 地址定义

#define MEM_WORK_MODE     0

#define MEM_FM_FREQUENCY        10   //2 byte   为兼容760-1080的频率，因1080-760=320已超出1个字节255的范围
#define MEM_FRE_TOTAL_NUM       11  //电台总数
#define MEM_CUR_FRE_NUM        12  //当前电台序号
#define MEM_SAVE_CHANNEL       13  //(1080-875+1)/8=26 byte  每个电台的频率值，每一位存一个电台

#define MEM_MIX_VOL                100




#define INPUT_MAX   9999



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


#endif




