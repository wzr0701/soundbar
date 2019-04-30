
#ifndef __SL_UI_DISPLAY_H__
#define __SL_UI_DISPLAY_H__


enum
{
    UI_DIS_START = -1,
    MAIN_VOL_DISPLAY,
    MIC_VOL_DISPLAY,
    ECHO_VOL_DISPLAY,
    TRE_VOL_DISPLAY,
    BASS_VOL_DISPLAY,
    UI_DIS_END,
} UI_DIS_MODE;



void display_ui_full(void);
void display_ui_clear(void);
void display_ui_icon(char icon,bool on_off);
void display_ui_main_sys(char wm_mode);
void display_ui_device(char wm_mode);

void display_ui_power(char on_off);
void display_ui_vol(int vol);
void display_ui_maxmin_vol(void);

void display_mic_vol(int vol);


void display_ui_usb(void);
void display_ui_sd(void);

void display_ui_bt(void);
void display_ui_fm(unsigned char f_ch);

void display_ui_aux(void);
void display_ui_rca(void);

void display_ui_op(void);

void display_ui_hdmi(void);

void display_str( char *dis_str);
 void display_set_source(int num);
 void ui_display_source(void);
void ui_goback_source(int delay);
void ui_update_music_time(void);
void display_ui_coaxial(void);

void display_ui_input(unsigned int number);

void dis_ui_updata_program(char on_off);
void display_ui_mic(bool on_off);
void display_ui_bass_vol(int mode,int vol);
void display_ui_version(int ver);
void display_ui_init(void);
void display_ui_ledtest(void);
void display_ui_mute(void);
void display_ui_enter_tre_bass(int mode);
void display_ui_usb_folder(int loc);
void display_ui_usb_number(int num);
void display_ui_movie(bool on_off);
void display_ui_fm_manual_save(void);
void display_ui_eq_power_test(void);

void display_eq_mode(int mode);
void display_ui_woofer(void);



#endif



