#ifndef SL_UI_CMD_H__
#define SL_UI_CMD_H__
/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "nxplayer.h"
#include "player_cmd.h"
#include "sl_ui_old.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define NET_STATUS_ESSETUP          0
#define NET_STATUS_STA_SUCCESS      1
#define NET_STATUS_STA_FAIL         2
#define NET_STATUS_STA_CONNECTTING  3
#define NET_STATUS_STA_DISCONNECTED 4
#define NET_STATUS_MAX              5

#define VOL_STEP (100.0f/60.0)//84.0

#define BYPASS_MODE 0
#define BYPASS_VOL_IN_MAX   -2
#define BYPASS_VOL_IN_MIN   -25
#define BYPASS_VOL_IN_STEP ((-2.0f - -25)/31)

#define INNERADC_MODE 0

#define MIX_6_CH_MIC 0

#define	ECHO_TYPE		0
#define	ECHO_GAIN		1
#define	ECHO_DELAY		2
#define	ECHO_STYPE		3
#define	ECHO_SHIFT		4
#define	ECHO_NUMBER		5
#define	ECHO_SETTING		6

#define	ECHO_SET                7


/****************************************************************************
 * Public Types
 ****************************************************************************/
/*开关机状态*/
enum
{
    POWER_START = -1,
    POWER_ON,
    POWER_OFF,
    POWER_END,
} POWER;
/*UI命令*/
enum ui_cmd_e {
	UI_CMD_NULL = 200,

	UI_CMD_USB_IN,
	UI_CMD_USB_OUT,
	UI_CMD_USB_UNLOAD,
	UI_CMD_SD_IN,
	UI_CMD_SD_OUT,
	UI_CMD_SD_UNLOAD,

	UI_NET_EASYSETUP,
	UI_NET_STA_SUCCESS,
	UI_NET_STA_FAIL,
	UI_NET_STA_CONNECTTING,
	UI_NET_STA_DISCONNECTED,

	UI_CMD_PLAYER_FINISH,
	UI_CMD_PLAYER_TONE_FINISH,

	UI_CMD_BT_PHONE_IN,
	UI_CMD_BT_VOICE_CONNECT,
	UI_CMD_BT_VOICE_DISCONNECT,

	UI_CMD_SHOW_TITLE,
	UI_CMD_SHOW_STATUS,
	UI_CMD_BUFFERING,

	/*************above cant change****************/
	/*************user add start*******************/
#ifdef CONFIG_CEC
	UI_CEC_INACTIVE_SOURCE,//234
	UI_CEC_ACTIVE_SOURCE,//235
#endif

	UI_CMD_SD_LOAD,	//220
	UI_CMD_USB_LOAD,   ///221
	UI_CMD_FILES_IS_LOAD,//222

	UI_CMD_NUM_0,       ///223
	UI_CMD_NUM_1,       //224
	UI_CMD_NUM_2,       ///225
	UI_CMD_NUM_3,     //226
	UI_CMD_NUM_4,     //227
	UI_CMD_NUM_5,    //228
	UI_CMD_NUM_6,    //229
	UI_CMD_NUM_7,   //230
	UI_CMD_NUM_8,   //231
	UI_CMD_NUM_9,   //232

	////////////////////////////////////////////////

	UI_CMD_HALF_SECOND,//233

	UI_CMD_BT_UP,//236
	UI_CMD_BT_DOWN,//237
	UI_CMD_BT_NEXT,//238
	UI_CMD_BT_PREV,//239
	UI_CMD_BT_OK,//240
	UI_CMD_BT_MODE,//241
	UI_CMD_BT_FUNC,//242
	UI_CMD_BT_EQ,//243

#if CONFIG_CEC
	UI_CEC_VOLUME_KEY_UP,//244
	UI_CEC_VOLUME_KEY_DOWN,//245
	UI_CEC_VOLUME_KEY_RELEASE,//246
	UI_CEC_MUTE_KEY,
#endif

	UI_CMD_BT_POWER,
	UI_CMD_NET_JOIN,

	UI_CMD_SECOND,

	UI_CMD_STOP,

	UI_CMD_LOW_BAT,
	UI_CMD_POWER,

	UI_CMD_VOLUME_MUTE,

	UI_CMD_VOLUME_DEC,
	UI_CMD_VOLUME_INC,

	UI_CMD_VOLUME_SET,

	UI_CMD_NEXT,
	UI_CMD_PREV,

	UI_CMD_FOLDER_NEXT,
	UI_CMD_FOLDER_PREV,

	UI_CMD_ECHO_ADD,
	UI_CMD_ECHO_SUB,
	UI_CMD_PLAY_PAUSE,
	UI_CMD_MODE,
	UI_CMD_GO_TO_BT,
	UI_CMD_GO_TO_USB,
	UI_CMD_GO_TO_SD,
	UI_CMD_GO_TO_AUX,
	UI_CMD_GO_TO_SPDIF,
	UI_CMD_GO_TO_HDMI,
	UI_CMD_GO_TO_COA,
	UI_CMD_GO_TO_FM,
	UI_CMD_GO_TO_TEST,
	UI_CMD_GET_MODE,
	UI_CMD_GET_VOLUME,
	UI_CMD_LED_TEST,

	UI_CMD_VOLUME_DEC_DOWN,
	UI_CMD_VOLUME_DEC_UP,
	UI_CMD_VOLUME_INC_DOWN,
	UI_CMD_VOLUME_INC_UP,
	UI_CMD_BT_PAIR,

	UI_CMD_ENTER,

	UI_CMD_EQ_TRE_ADD,
	UI_CMD_EQ_TRE_SUB,

	UI_CMD_EQ_BASS_ADD,
	UI_CMD_EQ_BASS_SUB,


	UI_CMD_FM_SCAN,
	UI_CMD_FM_HALF_SCAN,
	UI_CMD_FM_CHAN_ADD,
	UI_CMD_FM_CHAN_SUB,
	UI_CMD_FM_TUNE_ADD,
	UI_CMD_FM_TUNE_SUB,
	UI_CMD_FM_MANUAL_SAVE,

	UI_CMD_AUX_CONNECT,
	UI_CMD_AUX_DISCONNECT,
	UI_CMD_BLUETOOTH_CONNECT,
	UI_CMD_BLUETOOTH_DISCONNECT,

	UI_CMD_HDMI_CONNECT,
	UI_CMD_HDMI_DISCONNECT,

	UI_CMD_NUMBER,
	UI_CMD_PLAY_TONE,
	UI_CMD_PREV_ACTION,
	UI_CMD_NEXT_ACTION,

	UI_CMD_SYS_RESET,
	UI_CMD_SYS_MEM,

	UI_CMD_MIC_ON,

	UI_CMD_MIC_VOL_ADD,
	UI_CMD_MIC_VOL_SUB,

	UI_CMD_MOVIE_ON,

	UI_CMD_HDMION_SEND,
	UI_CMD_HDMION_DEINIT,

	UI_CMD_CHANGE_MODE_UNMUTE,

	UI_CMD_CHANGE_MODE_VOL_REC,

	UI_CMD_ENTER_TREBLE_SET,
	UI_CMD_ENTER_BASS_SET,

	UI_CMD_SET_SOURCE,

	UI_CMD_USB_PLAY_MUTE,

	UI_CMD_MIC_CONNECT,
	UI_CMD_MIC_DISCONNECT,

	UI_CMD_EQ_POWER_TEST,

	UI_CMD_MAX,
};
/*静音状态*/
enum
{
    UI_MUTE_START = -1,
    UNMUTE,
    MUTE,
    UI_MUTE_END,
} UI_MUTE;
/*音源选择*/
enum
{
	SOURCE_SELECT_INC = -2,
	SOURCE_SELECT_START = -1,
	SOURCE_SELECT_BT,
	SOURCE_SELECT_SPDIFIN,
	SOURCE_SELECT_USB,
	SOURCE_SELECT_LINEIN,
	SOURCE_SELECT_HDMI,
	SOURCE_SELECT_COA,
	SOURCE_SELECT_FM,
	SOURCE_SELECT_TEST,
	SOURCE_SELECT_END,
	SOURCE_SELECT_SD,
	SOURCE_SELECT_WIFI,
} UI_SELECT_SOURCE;

/*UI消息队列信息*/
struct ui_s
{
    mqd_t mq;
    char mqname[16];
};

/**/
typedef int (*CMD_FUNC)(void);
struct ui_cmd_func {
	int cmd;
	CMD_FUNC func;
};

/****************************************************************************
 * Public Data
 ****************************************************************************/
extern int ui_power;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
struct ui_cmd_func *sl_ui_parse_cmd(int cmd);
bool is_processing_done(void);
int forced_action(void);
int processing_action(void);
int set_sl_ui_cmd(int cmd, void *param, int arg);

//adapt to old version
ui_info_t sc8836_get_all_play_info(void);
extern void player_get_info(ui_info_t *info);
extern bool player_get_tag(TagInfo *tag_in, char* tag_info);

#endif
