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

	UI_CMD_USB_IN,//201
	UI_CMD_USB_OUT,//202
	UI_CMD_USB_UNLOAD,//203
	UI_CMD_SD_IN,//204
	UI_CMD_SD_OUT,//205
	UI_CMD_SD_UNLOAD,//206

	UI_NET_EASYSETUP,//207
	UI_NET_STA_SUCCESS,//208
	UI_NET_STA_FAIL,//209
	UI_NET_STA_CONNECTTING,//210
	UI_NET_STA_DISCONNECTED,//211

	UI_CMD_PLAYER_FINISH,//212
	UI_CMD_PLAYER_TONE_FINISH,//213

	UI_CMD_BT_PHONE_IN,//214
	UI_CMD_BT_VOICE_CONNECT,//215
	UI_CMD_BT_VOICE_DISCONNECT,//216

	UI_CMD_SHOW_TITLE,//217
	UI_CMD_SHOW_STATUS,//218
	UI_CMD_BUFFERING,//219

	/*************above cant change****************/
	/*************user add start*******************/

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

#ifdef CONFIG_CEC
	UI_CEC_INACTIVE_SOURCE,//234
	UI_CEC_ACTIVE_SOURCE,//235
#endif

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
	UI_CEC_MUTE_KEY,//247
#endif

	UI_CMD_BT_POWER,//248
	UI_CMD_NET_JOIN,//249

	UI_CMD_SECOND,//250

	UI_CMD_STOP,//251

	UI_CMD_LOW_BAT,//252
	UI_CMD_POWER,//253
	
	UI_CMD_VOLUME_MUTE,//254

	UI_CMD_VOLUME_DEC,//255
	UI_CMD_VOLUME_INC,//256

	UI_CMD_VOLUME_SET,//257
	UI_CMD_TREBLE_SET,//258
	UI_CMD_BASS_SET,//259
	UI_CMD_ECHO_SET,//260
	UI_CMD_MICVOL_SET,//261

	UI_CMD_NEXT,//262
	UI_CMD_PREV,//263

	UI_CMD_FOLDER_NEXT,//264
	UI_CMD_FOLDER_PREV,//265

	UI_CMD_ECHO_ADD,//266
	UI_CMD_ECHO_SUB,//267
	UI_CMD_PLAY_PAUSE,//268
	UI_CMD_MODE,//269
	UI_CMD_GO_TO_BT,//270
	UI_CMD_GO_TO_USB,//271
	UI_CMD_GO_TO_SD,//272
	UI_CMD_GO_TO_AUX,//273
	UI_CMD_GO_TO_SPDIF,//274
	UI_CMD_GO_TO_HDMI,//275
	UI_CMD_GO_TO_COA,//276
	UI_CMD_GO_TO_FM,//277
	UI_CMD_GO_TO_TEST,//278
	UI_CMD_GET_MODE,//279
	UI_CMD_GET_VOLUME,//280
	UI_CMD_LED_TEST,//287

	UI_CMD_VOLUME_DEC_DOWN,//282
	UI_CMD_VOLUME_DEC_UP,//283
	UI_CMD_VOLUME_INC_DOWN,//284
	UI_CMD_VOLUME_INC_UP,//285
	UI_CMD_BT_PAIR,//286

	UI_CMD_ENTER,//287

	UI_CMD_EQ_TRE_ADD,//288
	UI_CMD_EQ_TRE_SUB,//289

	UI_CMD_EQ_BASS_ADD,//290
	UI_CMD_EQ_BASS_SUB,//291


	UI_CMD_FM_SCAN,//292
	UI_CMD_FM_HALF_SCAN_ADD,//293
	UI_CMD_FM_HALF_SCAN_SUB,//294
	UI_CMD_FM_CHAN_ADD,//295
	UI_CMD_FM_CHAN_SUB,//296
	UI_CMD_FM_TUNE_ADD,//297
	UI_CMD_FM_TUNE_SUB,//298
	UI_CMD_FM_MANUAL_SAVE,//299

	UI_CMD_AUX_CONNECT,//300
	UI_CMD_AUX_DISCONNECT,//301
	UI_CMD_BLUETOOTH_CONNECT,//302
	UI_CMD_BLUETOOTH_DISCONNECT,//303

	UI_CMD_HDMI_CONNECT,//304
	UI_CMD_HDMI_DISCONNECT,//305

	UI_CMD_NUMBER,//306
	UI_CMD_PLAY_TONE,//307
	UI_CMD_PREV_ACTION,//308
	UI_CMD_NEXT_ACTION,//309

	UI_CMD_SYS_RESET,//310
	UI_CMD_SYS_MEM,//311

	UI_CMD_MIC_ON,//312

	UI_CMD_APP_MIC_ON,//313
	UI_CMD_APP_MIC_OFF,//314

	UI_CMD_MIC_VOL_ADD,//315
	UI_CMD_MIC_VOL_SUB,//316

	UI_CMD_MOVIE_ON,//317
	UI_CMD_APP_MOVIE_ON,//318
	UI_CMD_APP_MOVIE_OFF,//319

	UI_CMD_HDMION_SEND,//320
	UI_CMD_HDMION_DEINIT,//321

	UI_CMD_CHANGE_MODE_UNMUTE,//322

	UI_CMD_CHANGE_MODE_VOL_REC,//323

	UI_CMD_ENTER_TREBLE_SET,//324
	UI_CMD_ENTER_BASS_SET,//325

	UI_CMD_SET_SOURCE,//326

	UI_CMD_USB_PLAY_MUTE,//327

	UI_CMD_MIC_CONNECT,//328
	UI_CMD_MIC_DISCONNECT,//329

	UI_CMD_EQ_POWER_TEST,//330

	UI_CMD_APP_PLAY,//331
	UI_CMD_APP_PAUSE,//332

	UI_CMD_GET_USB_PLAY_STATUS,//333

	
	UI_CMD_FM_SCAN_START,//334
	UI_CMD_FM_SCAN_END,//335

	UI_CMD_USB_FOLDER_DIS,//336

	UI_CMD_OPEN_IIS,//337

    UI_CMD_SET_MICVOL,//338

	UI_CMD_PLAYER_PASS,//339
	
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
