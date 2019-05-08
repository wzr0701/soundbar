#ifndef SL_UI_CMD_DEAL_H__
#define SL_UI_CMD_DEAL_H__




//EEPROM��IRTC-RAM ��ַ����

#define MEM_WORK_MODE     0

#define MEM_FM_FREQUENCY        10   //2 byte   Ϊ����760-1080��Ƶ�ʣ���1080-760=320�ѳ���1���ֽ�255�ķ�Χ
#define MEM_FRE_TOTAL_NUM       11  //��̨����
#define MEM_CUR_FRE_NUM        12  //��ǰ��̨���
#define MEM_SAVE_CHANNEL       13  //(1080-875+1)/8=26 byte  ÿ����̨��Ƶ��ֵ��ÿһλ��һ����̨

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



