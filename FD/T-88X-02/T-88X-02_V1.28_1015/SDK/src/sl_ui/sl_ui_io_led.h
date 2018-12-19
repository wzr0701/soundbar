#ifndef SL_UI_IO_LED_H__
#define SL_UI_IO_LED_H__



#define PA_MUTE_HIGH   0     //////1----¸ßmute£¬0------µÍmute

#define BT_MUTE_DETECT_PIN   8
#define BT_POWER_CRT_PIN    23

#define SYS_POWER_CON_PIN    26



#define SL_HDMI_CEC_DET_PIN  (25)
#define SL_HDMI_CEC_PIN           (12)


#define FM_AUX_CHANNEL_CRT_PIN  15


#define POWER_MIC_PIN  46

#define PA_MUTE_PIN       7
#define PA_CLIP_OTW_PIN        33
#define PA_FAULT_PIN              34
#define PA_RESET_PIN              35



//////////////////////////////////////////////////
 void padmux_init(void);
void io_early_set(void);
void pa_static_check(void);
void enter_testmode_check(void);
void change_mode_unmute(void);
void pa_mute_ctrl(bool mute);
//void bt_mute_detect(void);
void pa_io_ret_set(bool on_off);
void bt_power_crt(bool on_off);
void aux_fm_channel_choose(bool chan);
void touch_key_int(void);
void pcm1803_power_crt(bool on_off);
void ui_hdmion_send(void);
void sys_power_control(void);


#endif
