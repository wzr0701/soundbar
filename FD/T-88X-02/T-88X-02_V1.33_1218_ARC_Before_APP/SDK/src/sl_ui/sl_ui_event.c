#include <nuttx/config.h>
#include <nuttx/input.h>
#include <nuttx/module.h>
#include <pthread.h>
#include <stdio.h>
#include <silan_adc.h>
#include "sl_ui_cmd.h"
#include "sl_ui_display.h"



#define CMD_UI_PLAY              0
#define CMD_UI_PWR               1
#define CMD_UI_MODE              2
#define CMD_UI_INC               3
#define CMD_UI_DEC               4


#define CONFIG_AUTO_UPDATE

unsigned char soft_updata_flag=0;
extern bool usb_online;
extern bool sd_online;

extern bool test_mode_flag;
extern bool next_folder_flag;
extern bool prev_folder_flag;

extern bool enter_tre_set;
extern bool enter_bass_set;

extern bool change_mode_flag;


/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
static ui_cmd_t ui_handle_click(int event_cmd);
static ui_cmd_t ui_handle_ir_press(int event_cmd);
static ui_cmd_t ui_handle_ir_longpress(int event_cmd);

extern int ui_source_select;

extern int enter_count;
extern int playkey_count;
extern int prevkey_count;

extern int enter_count1;
extern int playkey_count1;
extern int nextkey_count;

extern int tre_bass_cnt;

char bt_pair_cnt = 0;

int bt_ir_long_cnt = 0;

int change_mode_cnt = 0;

bool fm_scan_flag =  false;
bool fm_scan_start =  false;
bool ir_reset_flag =  false;
bool ir_playpause_flag =  false;
bool fm_manual_save_flag =  false;



static void handle_ui_events_inner(struct input_event *event);


#define	UI_EVENT_BACKUP_MAX	4
extern struct button_info	*g_ui_button;
struct ui_input_event {
	bool should_resend;
	struct input_event event;
};
static struct ui_input_event ui_event_bk[UI_EVENT_BACKUP_MAX];



static int ui_eventbk_init(void)
{
	int i;
	for (i=0; i<UI_EVENT_BACKUP_MAX; i++) {
		ui_event_bk[i].should_resend = false;
	}
	return 0;
}

static bool ui_event_restore(struct input_event *event)
{
	int i;
	for (i=0; i<UI_EVENT_BACKUP_MAX; i++) {
		if(ui_event_bk[i].should_resend == true) {
			if ((event->type == ui_event_bk[i].event.type) && (event->code == ui_event_bk[i].event.code) && (event->value == ui_event_bk[i].event.value))
				break;
			else
				continue;
		}
	}
	if (i < UI_EVENT_BACKUP_MAX) {
		printf("ui event already backup%d %d %d\n", event->type, event->code, event->value);
		return false;
	}
	for (i=0; i<UI_EVENT_BACKUP_MAX; i++) {
		if(ui_event_bk[i].should_resend == false) {
			memcpy(&(ui_event_bk[i].event), event, sizeof(struct input_event));
			ui_event_bk[i].should_resend = true;
			break;
		}
	}
	if (i == UI_EVENT_BACKUP_MAX) {
		printf("ui event backup has full %d %d %d\n", event->type, event->code, event->value);
		return false;
	}
	else
		return true;
}

static void ui_event_check_done(void)
{
	int  i;
	struct input_event *event;
	for (i=0; i<UI_EVENT_BACKUP_MAX; i++) {
		if(ui_event_bk[i].should_resend == true) {
			ui_event_bk[i].should_resend = false;
			event = &(ui_event_bk[i].event);
			handle_ui_events_inner(event);
		}
	}
}


#ifdef CONFIG_AUTO_UPDATE
static pthread_addr_t update_local(pthread_addr_t arg)
{
	printf("%s %d\n", __func__, __LINE__);
	dis_ui_updata_program(0);
	soft_updata_flag=1;
	silan_update_safe();
	soft_updata_flag=0;
	dis_ui_updata_program(1);
	usleep(100000);
}
#endif

static void handle_ui_events_inner(struct input_event *event)
{
	ui_cmd_t cmd;
#ifdef CONFIG_AUTO_UPDATE
	char name[32];
#endif
	cmd.cmd = UI_CMD_NULL;
	printf("%s %d 0x%x %d %d\n", __func__, __LINE__, event->type, event->code, event->value);
#if 0
	if (EV_CCHIP_KEY == event->type) {
		switch (event->code) {
			case CODE_KEY_CLICK:
                             // val = event->value & 0xfff;
				 // printf("press:%d\n", val);

                             // val = event->value & 0xf000;
				//  printf("press-flag:%x\n", val);
				  cmd.cmd =event->value & 0xffff;
				break;

		}
	}
#else
	if (EV_KEY == event->type)
    {
        switch (event->code)
        {
        case 0x8004:
        case 0xFFFF:
        case CODE_KEY_UP:
            if (event->value & 0xf00)
            {
                int value = event->value & 0x1f;
                if(value == CMD_UI_INC)
				{
					cmd.cmd = UI_CMD_VOLUME_INC_UP;
				}
                else if(value == CMD_UI_DEC)
				{
					cmd.cmd = UI_CMD_VOLUME_DEC_UP;
				}
			}
            break;
        case CODE_KEY_DOWN:
            if (event->value & 0xf00)
            {
                int value = event->value & 0x1f;
				if(value == CMD_UI_PLAY)
				{
					cmd.cmd = UI_CMD_BT_PAIR;
				}
                else if(value == CMD_UI_INC)
				{
					cmd.cmd = UI_CMD_VOLUME_INC_DOWN;
				}
                else if(value == CMD_UI_DEC)
				{
					cmd.cmd = UI_CMD_VOLUME_DEC_DOWN;
				}
            }
            break;
        case CODE_KEY_CLICK:
            cmd = ui_handle_click(event->value);
            break;
        }
    }
#endif
	else if (EV_BT == event->type) {
		switch (event->code) {
			case CODE_BT_PHONE_IN:
				cmd.cmd = UI_CMD_BT_PHONE_IN;
				break;

			case CODE_BT_VOICE_CONNECT:
				cmd.cmd = UI_CMD_BT_VOICE_CONNECT;
				break;

			case CODE_BT_VOICE_DISCONNECT:
				cmd.cmd = UI_CMD_BT_VOICE_DISCONNECT;
				break;
		}
	}
	else if (EV_PLAYER == event->type) {
		switch (event->code) {
			case CODE_PLAYER_PLAY_FINISH:
				cmd.cmd =UI_CMD_PLAYER_FINISH;
				break;

			case CODE_PLAYER_TONE_FINISH:
				cmd.cmd = UI_CMD_PLAYER_TONE_FINISH;
				break;

			case CODE_PLAYER_REPORT_VOLUME:
				cmd.cmd = NP_CMD_VOLUME_SET;
				cmd.arg2 = event->value;
				break;

			case CODE_PLAYER_BUFFER_LOW:
				cmd.cmd = UI_CMD_BUFFERING;
				break;

			case CODE_PLAYER_RECORD_ON:
				cmd.cmd = NP_CMD_RECORD_ON;
				break;

			case CODE_PLAYER_RECORD_OFF:
				cmd.cmd = NP_CMD_RECORD_OFF;
				break;

			case CODE_PLAYER_PLAY_STOP:
				cmd.cmd = NP_CMD_STOP;
				break;
		}
	}
	else if (EV_FSYS == event->type) {
		switch (event->code) {
			case CODE_FSYS_UPDATE_FILE:
				if(event->value == VALUE_SUCCESS) {
#ifdef CONFIG_AUTO_UPDATE
					pthread_t tid;
					pthread_create(&tid, NULL, update_local, NULL);
					pthread_detach(tid);
#endif
				}
				break;
		}
	}
	else if (EV_UI == event->type) {
		switch (event->code)  {
			case CODE_UI_SD_LOAD:
				if (event->value == VALUE_FAIL)
				{	//SD挂接失败
				}
				else if(event->value == VALUE_SUCCESS)
				{	//SD挂接成功
					cmd.cmd = UI_CMD_SD_LOAD;

				}
				break;

			case CODE_UI_SD_IN:
				sd_online=1;
#ifdef CONFIG_AUTO_UPDATE
				silan_check_update_file_save("/media/sd/sd00", "update.binary");
#endif
				cmd.cmd = UI_CMD_SD_IN;
				break;

			case CODE_UI_SD_UNLOAD:
				sd_online=0;
				cmd.cmd = UI_CMD_SD_UNLOAD;
				break;

			case CODE_UI_USB_LOAD:
				if (event->value == VALUE_FAIL)
				{	//USB挂接失败
				}
				else if(event->value == VALUE_SUCCESS)
				{	//USB挂接成功
					cmd.cmd = UI_CMD_USB_LOAD;
				}
				break;

			case CODE_UI_USB_IN:
				usb_online=1;
#ifdef CONFIG_AUTO_UPDATE
				memset(name, 0, 32);
				sprintf(name, "/media/usb/usb0%d", event->value);
				//silan_check_update_file_save(name, "update.binary");
				silan_check_update_file_save(name, "update.binary", false);
#endif
				//cmd.cmd = UI_CMD_USB_IN;
				cmd.arg2 = event->value;
				break;

			case CODE_UI_USB_UNLOAD:
				usb_online=0;
				if (event->value == VALUE_FAIL)
				{
					cmd.cmd = UI_CMD_USB_UNLOAD;
				}
				break;

			default:
				break;
		}
	}
#if 0
	else if (EV_IR == event->type) {
		switch (event->code) {

		case CODE_IR_POWER:
			cmd.cmd = UI_CMD_POWER;
			break;

		case CODE_IR_MUTE:
			cmd.cmd = UI_CMD_VOLUME_MUTE;
			break;

		case  CODE_IR_VOLUE_UP	:
			cmd.cmd = UI_CMD_VOLUME_INC;
			break;

		case  CODE_IR_VOLUE_DOWN:
			cmd.cmd = UI_CMD_VOLUME_DEC;
			break;

		case  CODE_IR_NEXT:
			cmd.cmd = UI_CMD_NEXT;
			break;


		case  CODE_IR_PREV:
			cmd.cmd = UI_CMD_PREV;
			break;

		case  CODE_IR_ECHO_ADD:
			cmd.cmd = UI_CMD_ECHO_ADD;
			break;

		case CODE_IR_ECHO_SUB:
			cmd.cmd = UI_CMD_ECHO_SUB;
			break;

		case CODE_IR_PLAY_PAUSE:
			cmd.cmd = UI_CMD_PLAY_PAUSE;
			break;

		case  CODE_IR_SOURCE:
			cmd.cmd = UI_CMD_MODE;
			break;

		case CODE_IR_GO_BT:
			cmd.cmd = UI_CMD_GO_TO_BT;
			break;


		///////////////////////////////////
		case CODE_IR_NUM_0:
			cmd.cmd = UI_CMD_NUM_0;
			break;

		case  CODE_IR_NUM_1:
			cmd.cmd = UI_CMD_NUM_1;
			break;

		case CODE_IR_NUM_2	:
			cmd.cmd = UI_CMD_NUM_2;
			break;


		case CODE_IR_NUM_3:
			cmd.cmd = UI_CMD_NUM_3;
			break;

		case CODE_IR_NUM_4:
			cmd.cmd = UI_CMD_NUM_4;
			break;

		case CODE_IR_NUM_5:
			cmd.cmd = UI_CMD_NUM_5;
			break;

		case CODE_IR_NUM_6:
			cmd.cmd = UI_CMD_NUM_6;
			break;

		case  CODE_IR_NUM_7:
			cmd.cmd = UI_CMD_NUM_7;
			break;

		case CODE_IR_NUM_8:
			cmd.cmd = UI_CMD_NUM_8;
			break;

		case  CODE_IR_NUM_9:
			cmd.cmd = UI_CMD_NUM_9;
			break;


		case CODE_IR_ENTER:
			cmd.cmd = UI_CMD_ENTER;
			break;

		////////////////////////////////////////////

		case  CODE_IR_FM_SCAN:
			cmd.cmd = UI_CMD_FM_SCAN;
			break;

		case  CODE_IR_FM_CHAN_ADD:
			cmd.cmd = UI_CMD_FM_CHAN_ADD;
			break;

		case  CODE_IR_FM_CHAN_SUB:
			cmd.cmd = UI_CMD_FM_CHAN_SUB;
			break;

		case  CODE_IR_FM_TUNE_ADD:
			cmd.cmd = UI_CMD_FM_TUNE_ADD;
			break;

		case  CODE_IR_FM_TUNE_SUB:
			cmd.cmd = UI_CMD_FM_TUNE_SUB;
			break;

		case  CODE_IR_TRE_ADD :
			cmd.cmd = UI_CMD_EQ_TRE_ADD;
			break;

		case CODE_IR_TRE_SUB :
			cmd.cmd = UI_CMD_EQ_TRE_SUB;
			break;

		case CODE_IR_BASS_ADD:
			cmd.cmd = UI_CMD_EQ_BASS_ADD;
			break;

		case  CODE_IR_BASS_SUB :
			cmd.cmd = UI_CMD_EQ_BASS_SUB;
			break;

		case CODE_IR_RESET	:
			cmd.cmd = UI_CMD_SYS_RESET;
			break;

		case CODE_IR_MEM  :
			cmd.cmd = UI_CMD_SYS_MEM;
			break;
		}

	}
#else

	else if (EV_IR == event->type) {
		switch (event->code) {
			case CODE_IR_PRESS:
				cmd = ui_handle_ir_press(event->value);
				break;
			case CODE_IR_LONG_PRESS:
				cmd = ui_handle_ir_longpress(event->value);
				break;
		}

	}
#endif
	#ifdef CONFIG_CEC
	else if (EV_CEC == event->type) {
		switch(event->code) {
			case CODE_CEC_ACTIVE_SOURCE:
		        cmd.cmd = UI_CEC_ACTIVE_SOURCE;
				break;
			case CODE_CEC_INACTIVE_SOURCE:
				cmd.cmd = UI_CEC_INACTIVE_SOURCE;
				break;
			case CODE_CEC_VOLUME_KEY_UP:
				cmd.cmd = UI_CEC_VOLUME_KEY_UP;
				break;
			case CODE_CEC_VOLUME_KEY_DOWN:
				cmd.cmd = UI_CEC_VOLUME_KEY_DOWN;
				break;
			case CODE_CEC_VOLUME_KEY_RELEASE:
				cmd.cmd = UI_CEC_VOLUME_KEY_RELEASE;
				break;
			case CODE_CEC_MUTE_KEY:
			    cmd.cmd = UI_CEC_MUTE_KEY;
				break;
		}
	}
#endif

	if (cmd.cmd != UI_CMD_NULL)
		send_cmd_2_ui(&cmd);

}


static void handle_ui_events(struct input_event *event, void *cb_para)
{
	handle_ui_events_inner(event);
	ui_event_check_done();
}

static int init_sl_ui_event(void)
{
	printf("%s %d\n", __func__, __LINE__);
	ui_eventbk_init();
	ui_cmd_init();
	idjs_event_register(handle_ui_events, NULL);
	return 0;
}

app_entry(init_sl_ui_event)



/****************************************************************************
 * Name: ui_handle_click
 *
 * Description:
 *    按键单击事件到UI事件的转换函数
 *
 * Parameters:
 *
 * Returned Value:
 *    UI事件命令
 *
 * Assumptions:
 *
 ****************************************************************************/
static ui_cmd_t ui_handle_click(int event_cmd)
{
    ui_cmd_t cmd;
    cmd.cmd = UI_CMD_NULL;
    switch (event_cmd & 0x1f)
    {
		case CMD_UI_PLAY:
			if(test_mode_flag == false)
			{
				playkey_count++;
				enter_count = 0;
				playkey_count1++;
				enter_count1 = 0;
				cmd.cmd = UI_CMD_PLAY_PAUSE;
			}
			else
			{
				cmd.cmd = UI_CMD_LED_TEST;
			}
			break;
		case CMD_UI_PWR:
			if(test_mode_flag == false)
			{
				cmd.cmd = UI_CMD_POWER;
			}
	        else
	        {
				cmd.cmd = UI_CMD_LED_TEST;
			}
	        break;
	    case CMD_UI_MODE:
			if(test_mode_flag == false)
			{
				//change_mode_cnt++;
				if(change_mode_flag == true)
				{
					cmd.cmd = UI_CMD_MODE;
				}
			}
	        else
	        {
				cmd.cmd = UI_CMD_LED_TEST;
			}
	        break;
	    case CMD_UI_INC:
			if(test_mode_flag == false)
			{
				if(playkey_count1 == 5)
				{
					nextkey_count++;
					enter_count1 = 0;
				}
				cmd.cmd = UI_CMD_VOLUME_INC;
			}
	        else
	        {
				cmd.cmd = UI_CMD_VOLUME_INC;
			}
	        break;
	    case CMD_UI_DEC:
			if(test_mode_flag == false)
			{
				if(playkey_count == 3)
				{
					prevkey_count++;
					enter_count = 0;
				}
				cmd.cmd = UI_CMD_VOLUME_DEC;
			}
			else
			{
				cmd.cmd = UI_CMD_VOLUME_DEC;
			}
	        break;
    }
    return cmd;
}

static ui_cmd_t ui_handle_ir_press(int event_cmd)
{
    ui_cmd_t cmd;
    cmd.cmd = UI_CMD_NULL;
    switch (event_cmd)
    {
		case CODE_IR_POWER:
			cmd.cmd = UI_CMD_POWER;
			break;

		case CODE_IR_MUTE:
			ir_reset_flag = true;
			cmd.cmd = UI_CMD_VOLUME_MUTE;
			break;

		case  CODE_IR_VOLUE_UP	:
			cmd.cmd = UI_CMD_VOLUME_INC;
			break;

		case  CODE_IR_VOLUE_DOWN:
			cmd.cmd = UI_CMD_VOLUME_DEC;
			break;

		case  CODE_IR_NEXT:
			next_folder_flag = false;
			cmd.cmd = UI_CMD_NEXT;
			break;

		case  CODE_IR_PREV:
			prev_folder_flag = false;
			cmd.cmd = UI_CMD_PREV;
			break;

		case  CODE_IR_ECHO_ADD:
			cmd.cmd = UI_CMD_ECHO_ADD;
			break;

		case CODE_IR_ECHO_SUB:
			cmd.cmd = UI_CMD_ECHO_SUB;
			break;

		case CODE_IR_PLAY_PAUSE:
			ir_playpause_flag = false;
			if(ui_source_select != SOURCE_SELECT_FM)
				cmd.cmd = UI_CMD_PLAY_PAUSE;
			break;

		case  CODE_IR_SOURCE:
			//change_mode_cnt++;
			if(change_mode_flag == true)
			{
				cmd.cmd = UI_CMD_MODE;
			}
			break;

		case CODE_IR_TRE:
			cmd.cmd =    UI_CMD_ENTER_TREBLE_SET;
			break;

		case CODE_IR_BASS:
			cmd.cmd =    UI_CMD_ENTER_BASS_SET;
			break;

		case CODE_IR_MIC  :
			cmd.cmd = UI_CMD_MIC_ON;
			break;
/*
		case CODE_IR_GO_BT:
			if(ui_source_select != SOURCE_SELECT_BT)
				cmd.cmd = UI_CMD_GO_TO_BT;
			break;
*/

		///////////////////////////////////
		case CODE_IR_NUM_0:
			cmd.cmd = UI_CMD_NUM_0;
			break;

		case  CODE_IR_NUM_1:
			cmd.cmd = UI_CMD_NUM_1;
			break;

		case CODE_IR_NUM_2	:
			cmd.cmd = UI_CMD_NUM_2;
			break;


		case CODE_IR_NUM_3:
			cmd.cmd = UI_CMD_NUM_3;
			break;

		case CODE_IR_NUM_4:
			cmd.cmd = UI_CMD_NUM_4;
			break;

		case CODE_IR_NUM_5:
			cmd.cmd = UI_CMD_NUM_5;
			break;

		case CODE_IR_NUM_6:
			cmd.cmd = UI_CMD_NUM_6;
			break;

		case  CODE_IR_NUM_7:
			cmd.cmd = UI_CMD_NUM_7;
			break;

		case CODE_IR_NUM_8:
			cmd.cmd = UI_CMD_NUM_8;
			break;

		case  CODE_IR_NUM_9:
			cmd.cmd = UI_CMD_NUM_9;
			break;


		case CODE_IR_ENTER:
			cmd.cmd = UI_CMD_ENTER;
			break;

		////////////////////////////////////////////

		case  CODE_IR_FM_SCAN:
			cmd.cmd = UI_CMD_FM_SCAN;
			break;

		case CODE_IR_MIC_ADD:
			cmd.cmd = UI_CMD_MIC_VOL_ADD;
			break;

		case CODE_IR_MIC_SUB:
			cmd.cmd = UI_CMD_MIC_VOL_SUB;
			break;
/*
		case  CODE_IR_FM_CHAN_ADD:
			cmd.cmd = UI_CMD_FM_CHAN_ADD;
			break;

		case  CODE_IR_FM_CHAN_SUB:
			cmd.cmd = UI_CMD_FM_CHAN_SUB;
			break;
*/
		case  CODE_IR_FM_TUNE_ADD:
			fm_scan_flag = true;
			if(fm_scan_start == true)
			{
				fm_scan_start =      false;
				cmd.cmd = UI_CMD_FM_HALF_SCAN;
			}
			else
			{
				cmd.cmd = UI_CMD_FM_TUNE_ADD;
			}
			break;

		case  CODE_IR_FM_TUNE_SUB:
			fm_scan_flag = true;
			if(fm_scan_start == true)
			{
				fm_scan_start =      false;
				cmd.cmd = UI_CMD_FM_HALF_SCAN;
			}
			else
			{
				cmd.cmd = UI_CMD_FM_TUNE_SUB;
			}
			break;

		case CODE_IR_MOVIE:
			cmd.cmd = UI_CMD_MOVIE_ON;
			break;
/*
		case  CODE_IR_TRE_ADD :
			cmd.cmd = UI_CMD_EQ_TRE_ADD;
			break;

		case CODE_IR_TRE_SUB :
			cmd.cmd = UI_CMD_EQ_TRE_SUB;
			break;

		case CODE_IR_BASS_ADD:
			cmd.cmd = UI_CMD_EQ_BASS_ADD;
			break;

		case  CODE_IR_BASS_SUB :
			cmd.cmd = UI_CMD_EQ_BASS_SUB;
			break;

		case CODE_IR_RESET	:
			cmd.cmd = UI_CMD_SYS_RESET;
			break;

		case CODE_IR_MEM  :
			cmd.cmd = UI_CMD_SYS_MEM;
			break;
*/
    }
    return cmd;
}

static ui_cmd_t ui_handle_ir_longpress(int event_cmd)
{
	ui_cmd_t cmd;
    cmd.cmd = UI_CMD_NULL;
    switch (event_cmd)
    {
		/*case CODE_IR_GO_BT:
			bt_ir_long_cnt ++;
			if(bt_ir_long_cnt == 3)
			{
				bt_ir_long_cnt = 0;
				if(ui_source_select == SOURCE_SELECT_BT)
				{
					cmd.cmd = UI_CMD_BT_PAIR;
				}
			}

			//printf("%s : %d\n", __func__, __LINE__);
		break;*/

		case CODE_IR_VOLUE_UP:
			cmd.cmd = UI_CMD_VOLUME_INC;
			//printf("%s : %d\n", __func__, __LINE__);
		break;

		case CODE_IR_VOLUE_DOWN:
				cmd.cmd = UI_CMD_VOLUME_DEC;
			//printf("%s : %d\n", __func__, __LINE__);
		break;

		case CODE_IR_NEXT:
			if(next_folder_flag == false)
			{
				next_folder_flag = true;
				if(ui_source_select == SOURCE_SELECT_USB)
					cmd.cmd = UI_CMD_FOLDER_NEXT;
					//printf("%s : %d\n", __func__, __LINE__);
			}
			break;

		case CODE_IR_PREV:
			if(prev_folder_flag == false)
			{
				prev_folder_flag =      true;
				if(ui_source_select == SOURCE_SELECT_USB)
					cmd.cmd = UI_CMD_FOLDER_PREV;
				//printf("%s : %d\n", __func__, __LINE__);
			}
			break;

		case CODE_IR_MUTE:
			if(ir_reset_flag == false)
			{
				ir_reset_flag = true;
				cmd.cmd = UI_CMD_SYS_RESET;
			}
			break;

		case CODE_IR_PLAY_PAUSE:
			if(ir_playpause_flag == false)
			{
				if(ui_source_select == SOURCE_SELECT_BT)
				{
					bt_pair_cnt++;
					if(bt_pair_cnt == 3)
					{
						ir_playpause_flag = true;
						bt_pair_cnt = 0;
						cmd.cmd = UI_CMD_BT_PAIR;
					}
				}
				else if(ui_source_select == SOURCE_SELECT_FM)
				{
					ir_playpause_flag = true;
					cmd.cmd = UI_CMD_PLAY_PAUSE;
				}
			}
			break;

		case  CODE_IR_FM_TUNE_ADD:
			if(fm_scan_flag == true)
			{
				fm_scan_flag = false;
				fm_scan_start = true;
				cmd.cmd = UI_CMD_FM_HALF_SCAN;
			}
			break;

		case  CODE_IR_FM_TUNE_SUB:
			if(fm_scan_flag == true)
			{
				fm_scan_flag = false;
				fm_scan_start = true;
				cmd.cmd = UI_CMD_FM_HALF_SCAN;
			}
			break;

		case CODE_IR_MIC_ADD:
			cmd.cmd = UI_CMD_MIC_VOL_ADD;
			break;

		case CODE_IR_MIC_SUB:
			cmd.cmd = UI_CMD_MIC_VOL_SUB;
			break;

		case  CODE_IR_ECHO_ADD:
			cmd.cmd = UI_CMD_ECHO_ADD;
			break;

		case CODE_IR_ECHO_SUB:
			cmd.cmd = UI_CMD_ECHO_SUB;
			break;

		case  CODE_IR_FM_SCAN:
			if(fm_manual_save_flag == false)
			{
				fm_manual_save_flag = true;
				cmd.cmd = UI_CMD_FM_MANUAL_SAVE;
			}
			break;
/*
		case  CODE_IR_TRE_ADD :
			cmd.cmd = UI_CMD_EQ_TRE_ADD;
			break;

		case CODE_IR_TRE_SUB :
			cmd.cmd = UI_CMD_EQ_TRE_SUB;
			break;

		case CODE_IR_BASS_ADD:
			cmd.cmd = UI_CMD_EQ_BASS_ADD;
			break;

		case  CODE_IR_BASS_SUB :
			cmd.cmd = UI_CMD_EQ_BASS_SUB;
			break;
*/
	}
	return cmd;
}


