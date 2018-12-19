#ifndef SL_UI_IO_LED_H__
#define SL_UI_IO_LED_H__

enum
{
    BT_4052,
    RCA_4052,
    AUX_4052,
    NONE_4052
} switch_4052;


#define PA_MUTE_HIGH   0//////1----¸ßmute£¬0------µÍmute

#define BT_MUTE_DETECT_PIN   8
#define BT_POWER_CRT_PIN    23

//#define SYS_POWER_CON_PIN    26

#define SL_AUXIN_DET_PIN        26

#define SL_HDMI_CEC_DET_PIN                17
#define SL_HDMI_CEC_PIN             16


#define FM_AUX_CHANNEL_CRT_PIN  15


#define POWER_MIC_PIN  46

//#define MIC_DET_PIN 37
#define PA_MUTE_PIN         12
//#define PA_CLIP_OTW_PIN        33
//#define PA_FAULT_PIN              34
//#define PA_RESET_PIN              35

#define SW1_4052 7
#define SW2_4052 8

#define PWR_LED_PIN 34
#define BT_LED_PIN 35
#define OPT_LED_PIN 36
#define HDMI_LED_PIN 37
#define LINEIN_LED_PIN 38


#define RGB_R_PIN 33
#define RGB_G_PIN 32
#define RGB_B_PIN 27

#define MIC_LED_PIN 45

#define RGB_RED 1
#define RGB_GREEN 2
#define RGB_BLUE 3
#define RGB_WHITE 4
#define RGB_PURPLE 5 //ç´«è‰²
#define RGB_ORANGE 6 //æ©™è‰²
#define RGB_YELLOW          7



//////////////////////////////////////////////////
 void padmux_init(void);
void io_early_set(void);
void pa_static_check(void);
void show_modeled_off(void);
void show_modeled_on(int mode);
void show_RGB_off(void);
void show_RGB_on(int mode);
void switch_4052_function(int function);
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
