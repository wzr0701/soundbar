#ifndef __SL_UI_HANDLE_H__
#define __SL_UI_HANDLE_H__
/****************************************************************************
 * Included Files
 ****************************************************************************/

static const int bass_treble_gains[][2] = {
    {50, 600},  //300
    {60, 600},
    {70, 600},
    {80, 600},
    {90, 600},
};

#if 0
static const int mix_vol_table[61]={
	0, 1, 2, 3, 5, 7,
	8, 10, 11, 13,15,18,
	20, 22, 24, 26,28,30,
	31, 33, 35, 37,38,40,
	41, 43, 45, 47,48,50,
	51, 53, 55, 57,58,60,
	61, 63, 65, 67,68,70,
	71, 73, 75, 77,78,80,
	81, 83, 85, 87,88,90,
	91, 93, 94, 96,98,99,
	100
};
#else
static const int mix_vol_table[51]={
	0, 2, 4, 6, 8, 10,
	12, 14, 16, 18,20,
	22, 24, 26,28,30,
	32, 34, 36, 38,40,
	42, 44, 46,48,50,
	52, 54, 56,58,60,
	62, 64, 66,68,70,
	72, 74, 76,78,80,
	82, 84, 86,88,90,
	92, 94, 96,98,100
};

#define MIX_VOL_BT
#ifdef MIX_VOL_BT
static const int mix_vol_bt_table[51]={
	0, 1,3, 5, 7, 9, 10,
	11, 13, 15, 17,19,20,
	21, 23, 25,27,29,30,
	31, 33, 35, 37,39,40,
	41, 43, 45,47,49,50,
	51, 53, 55,57,59,60,
	61, 63, 65,67,69,70,
	71, 73, 75,77,79,80,
	81,82
};
#endif

#endif

static const int bass_treble_gains_size = (sizeof(bass_treble_gains)/sizeof(bass_treble_gains[0]));


#define Frist_MIX_VOL 30
#define Frist_MIX_LEV 15
#define MIX_LEV_CNT 50


#define BASS_TREBLE_GAIN_MAX 600
#define BASS_GAIN_MAX 1200
#define TREBLE_GAIN_MAX 600


//BASS/TREBLE��???��y
#define BASS_TREBLE_LEVEL_MAX                 15
#define BASS_TREBLE_LEVEL_MIN -15

//BASS/TREBLE��?2????��
#define BASS_TREBLE_STEP  1
//BASS/TREBLE��???��?2????��
#define BASS_TREBLE_GAIN_STEP  (BASS_TREBLE_GAIN_MAX/15)
#define BASS_GAIN_STEP  (BASS_GAIN_MAX/15)
#define TREBLE_GAIN_STEP  (TREBLE_GAIN_MAX/15)



/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/
void bt_cmd_dis_connect(void);

/****************************************************************************
 * Name: aux_set_connect_state
 *
 * Description:
 *    设置aux连接状态
 *
 * Parameters:
 *    state true连接，false未连接
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void aux_set_connect_state(bool state);

/****************************************************************************
 * Name: bt_set_connect_state
 *
 * Description:
 *    设置蓝牙连接状态
 *
 * Parameters:
 *    state true连接，false未连接
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void bt_set_connect_state(bool state);

/****************************************************************************
 * Name: bt_chk_and_disp
 *
 * Description:
 *    蓝牙连接状态检查并显示
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
//void bt_chk_and_disp(void);

/****************************************************************************
 * Name: bt_init_sem
 *
 * Description:
 *    蓝牙同步信号量初始化
 *
 * Parameters:
 *    en 1-使能，0-禁用
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void bt_init_sem(void);

/****************************************************************************
 * Name: radio_freq_valid
 *
 * Description:
 *    校验收音机当前是否有效频道
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void radio_freq_valid(void);

/****************************************************************************
 * Name: ui_goback_source
 *
 * Description:
 *    音源延时显示函数
 *
 * Parameters:
 *    delay 延时的时间
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_goback_source(int delay);

/****************************************************************************
 * Name: ui_handle_action_up
 *
 * Description:
 *    next/prev 长按抬起处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_action_up(void);

/****************************************************************************
 * Name: ui_handle_down
 *
 * Description:
 *    红外音量减事件处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_down(void);

/****************************************************************************
 * Name: ui_handle_eq
 *
 * Description:
 *    eq处理
 *
 * Parameters:
 *    num eq的类型
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_eq(int num);

/****************************************************************************
 * Name: ui_handle_file_load
 *
 * Description:
 *    处理文件加载
 *
 * Parameters:
 *    total_num 总文件数
 *    folder_num 总文件夹数
 *    url 媒体路径
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_file_load(int total_num, int folder_num, char * url);

/****************************************************************************
 * Name: ui_handle_get_mode
 *
 * Description:
 *    获取模式消息处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_get_mode(void);

/****************************************************************************
 * Name: ui_handle_get_volume
 *
 * Description:
 *    获取音量消息处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_get_volume(void);

/****************************************************************************
 * Name: ui_handle_load_sd
 *
 * Description:
 *    SD文件列表加载事件处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_load_sd(void);

/****************************************************************************
 * Name: ui_handle_load_usb
 *
 * Description:
 *    USB文件列表加载事件处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_load_usb(void);

/****************************************************************************
 * Name: ui_handle_mode
 *
 * Description:
 *    模式切换处理
 *
 * Parameters:
 *    source 要切换的模式
 *    notify 是否需要通知3266
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_mode(int source, bool notify);

/****************************************************************************
 * Name: ui_handle_mute
 *
 * Description:
 *    静音处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_mute(void);

/****************************************************************************
 * Name: ui_handle_next
 *
 * Description:
 *    下一曲处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_next(void);

/****************************************************************************
 * Name: ui_handle_next_action
 *
 * Description:
 *    快进处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_next_action(void);

/****************************************************************************
 * Name: ui_handle_next_down
 *
 * Description:
 *     长按next按键快进处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_next_down(void);

/****************************************************************************
 * Name: ui_handle_pause_play
 *
 * Description:
 *    暂停播放处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_pause_play(void);

/****************************************************************************
 * Name: ui_handle_power
 *
 * Description:
 *    开关机处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_power(int power_on_off);

/****************************************************************************
 * Name: ui_handle_prev
 *
 * Description:
 *    上一曲处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_prev(void);

/****************************************************************************
 * Name: ui_handle_prev_action
 *
 * Description:
 *    快进处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_prev_action(void);

/****************************************************************************
 * Name: ui_handle_next_down
 *
 * Description:
 *     长按next按键快进处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_prev_down(void);

/****************************************************************************
 * Name: ui_handle_sd_unload
 *
 * Description:
 *    SD卡卸载处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_sd_unload(void);

/****************************************************************************
 * Name: ui_handle_tone
 *
 * Description:
 *    播放指定的tone
 *
 * Parameters:
 *    tone_num tone的序号
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_tone(int tone_num);

/****************************************************************************
 * Name: ui_handle_tone_finish
 *
 * Description:
 *    播放播放结束处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_tone_finish(void);

/****************************************************************************
 * Name: ui_handle_up
 *
 * Description:
 *    红外音量加事件处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_up(void);

/****************************************************************************
 * Name: ui_handle_usb_out
 *
 * Description:
 *    USB拔出事件处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_usb_out(void);

/****************************************************************************
 * Name: ui_handle_vol_dec_long_press
 *
 * Description:
 *    音量减物理按键长按处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_dec_long_press(void);

/****************************************************************************
 * Name: ui_handle_vol_inc_long_press
 *
 * Description:
 *    音量增加物理按键长按处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_inc_long_press(void);

/****************************************************************************
 * Name: ui_handle_vol_long_press_up
 *
 * Description:
 *    音量增减物理按键长按弹起处理
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_long_press_up(void);

/****************************************************************************
 * Name: ui_handle_vol_set
 *
 * Description:
 *    音量设置处理
 *
 * Parameters:
 *    vol 设置的音量（0-100）
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_handle_vol_set(int vol);

/****************************************************************************
 * Name: ui_update_music_time
 *
 * Description:
 *    更新时间显示
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
//oid ui_update_music_time(void);
void save_usb_play_time(void);





void ui_handle_play_num(int num);


#ifdef CONFIG_CEC

int sc8836_action_hdmion_send(void);

int sc8836_ui_handle_cec_inactive_source( void);
int sc8836_ui_handle_cec_active_source( void);

int sc8836_ui_handle_cec_volume_key_up(struct ui_cmd_s *cmd);
int sc8836_ui_handle_cec_volume_key_down(struct ui_cmd_s *cmd);
int sc8836_ui_handle_cec_volume_key_release(struct ui_cmd_s *cmd);
int sc8836_ui_handle_cec_mute_key(struct ui_cmd_s *cmd);


int sc8836_action_hdmi_soundbar_adj_tv_vol(void);
int sc8836_action_hdmi_soundbar_mute_tv(void);
int sc8836_action_hdmi_soundbar_unmute_tv(void);

int sc8836_action_hdmi_on(void);
int sc8836_action_hdmi_off(void);
int sc8836_action_hdmi_standby(void);
int sc8836_action_hdmi_poweron(void);

#endif

void ui_handle_eq_music(void);
void ui_handle_eq_movie(void);
void ui_handle_eq_dialog(void);
void ui_handle_unload_eq(void);


void bt_read_state(void);
void bt_cmd_reset_iis(void);
void bt_cmd_play_pause(void);
void bt_cmd_next_song(void);
void bt_cmd_prev_song(void);
void bt_cmd_get_version(void);
void bt_handle_get_volume(void);
void bt_handle_get_mode(void);
void ui_bt_rewind(void);
void ui_bt_forward(void);

void set_adc_channel_vol(int ch, int vol);
void sl_ui_set_reqrate(void);
void sl_ui_system_reset(void);

void ui_handle_folder_next(void);
void ui_handle_folder_prev(void);


#endif
