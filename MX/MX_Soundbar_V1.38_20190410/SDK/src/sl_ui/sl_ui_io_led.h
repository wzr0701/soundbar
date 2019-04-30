#ifndef SL_UI_IO_LED_H__
#define SL_UI_IO_LED_H__



#define PA_MUTE_HIGH   0     //////1----¸ßmute£¬0------µÍmute

#define BT_MUTE_DETECT_PIN   8

#define SYS_POWER_CON_PIN    26



#define SL_HDMI_CEC_DET_PIN  (25)
#define SL_HDMI_CEC_PIN           (12)

#define FM_AUX_CHANNEL_CRT_PIN  15

#define PA_MUTE_PIN        12

#define SW1_4052_PIN 7
#define SW2_4052_PIN 8


enum
{
    RCA_4052,
    AUX_4052,
    FM_4052,
    NONE_4052
} switch_4052;




//////////////////////////////////////////////////
 void padmux_init(void);
void io_early_set(void);
void pa_static_check(void);
void enter_othermode_check(void);
void change_mode_unmute(void);
void pa_mute_ctrl(bool mute);
//void bt_mute_detect(void);
void ui_hdmion_send(void);
void sys_power_control(void);


#endif
