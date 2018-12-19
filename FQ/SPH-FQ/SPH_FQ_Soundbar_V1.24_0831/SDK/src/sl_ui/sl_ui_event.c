/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/module.h>
#include "input_key.h"
#include "sl_ui_cmd.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CMD_UI_NEXT              0
#define CMD_UI_PREV              1
#define CMD_UI_MODE              2
#define CMD_UI_INC               3
#define CMD_UI_DEC               4
#define CMD_UI_PLAY	             5
#define CMD_UI_PWR               7


 #if 0
#define CMD_UI_PREV  1
#define CMD_UI_NEXT  3
#define CMD_UI_PLAY	2
#define CMD_UI_MODE          0
#define CMD_UI_PWR   7
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/
extern void idjs_event_register(void *func, void *p);
extern void send_cmd_2_ui(ui_cmd_t *ui_cmd);

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
static ui_cmd_t ui_handle_click(int event_cmd);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: handle_ui_events
 *
 * Description:
 *    将事件命令转换成UI命令
 * 
 * Parameters:
 *    event  事件信息
 *    cb_para  回调参数
 *
 * Returned Value:
 * 
 * Assumptions:
 *
 ****************************************************************************/
static void handle_ui_events(struct input_event *event, void *cb_para)
{
    ui_cmd_t cmd;
    cmd.cmd = UI_CMD_NULL;
    printf("%s %X %X %X\n", __func__, event->type, event->code, event->value);
    if (EV_KEY == event->type)
    {
        switch (event->code)
        {
        case 0x8004:
        case 0xFFFF:
        case CODE_KEY_UP:
            if (event->value & 0xf00)
            {
                int value = event->value & 0x0f;
                if(value == CMD_UI_INC)
				{
					cmd.cmd = UI_CMD_VOLUME_INC_UP;
				}
                else if(value == CMD_UI_DEC)
				{
					cmd.cmd = UI_CMD_VOLUME_DEC_UP;
				}
                #if 0
                if(value == CMD_UI_PREV)
                {
					cmd.cmd = UI_CMD_VOLUME_DEC_UP;
                }
                else if(value == CMD_UI_NEXT)
                {
                	cmd.cmd = UI_CMD_VOLUME_INC_UP;
                }
				else if(value == CMD_UI_PLAY)
				{
				}
                #endif
			}
            break;
        case CODE_KEY_DOWN:
            if (event->value & 0xf00)
            {
                int value = event->value & 0x0f;
                if(value == CMD_UI_PREV)
                {
                    cmd.cmd = UI_CMD_BT_PREV;
                }
                else if(value == CMD_UI_NEXT)
                {
                    cmd.cmd = UI_CMD_BT_NEXT;
                }
                if(value == CMD_UI_MODE)
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
                #if 0
                if(value == CMD_UI_PREV)
                {
                    cmd.cmd = UI_CMD_VOLUME_DEC_DOWN;
                }
                else if(value == CMD_UI_NEXT)
                {
                    cmd.cmd = UI_CMD_VOLUME_INC_DOWN;
                }
				else if(value == CMD_UI_PLAY)
				{
					cmd.cmd = UI_CMD_BT_PAIR;
				}				
				else if(value == CMD_UI_PWR)
				{
					cmd.cmd = UI_CMD_BT_POWER;
				}
                #endif
            }
            break;
        case CODE_KEY_CLICK:
            cmd = ui_handle_click(event->value);
            break;
        }
    }
    else if (EV_BT == event->type)
    {
        switch (event->code)
        {
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
    else if (EV_PLAYER == event->type)
    {
        switch (event->code)
        {
        case CODE_PLAYER_PLAY_FINISH:
            cmd.cmd = UI_CMD_PLAYER_FINISH;
            break;
        case CODE_PLAYER_TONE_FINISH:
            cmd.cmd = UI_CMD_PLAYER_TONE_FINISH;
            break;
        case CODE_PLAYER_REPORT_VOLUME:
            break;
        case CODE_PLAYER_BUFFER_LOW:
            cmd.cmd = UI_CMD_BUFFERING;
            break;
        case CODE_PLAYER_RECORD_ON:
            break;
        case CODE_PLAYER_RECORD_OFF:
            break;
        case CODE_PLAYER_PLAY_STOP:
            cmd.cmd = UI_CMD_STOP;
            break;
        }
    }
    else if (EV_IR == event->type)
    {
        switch (event->code)
        {
		#if 0
        case CODE_POWER:
			cmd.cmd = UI_CMD_BT_POWER;
			break;
		case CODE_MUTE:
			cmd.cmd = UI_CMD_BT_MUTE;
			break;	
		case CODE_VOLUME_INC:
			cmd.cmd = UI_CMD_BT_UP;
			break;
		case CODE_VOLUME_DEC:
			cmd.cmd = UI_CMD_BT_DOWN;
			break;	
		case CODE_CHANNEL_PREV:
			cmd.cmd = UI_CMD_BT_PREV;
			break;
		case CODE_CHANNEL_NEXT:
			cmd.cmd = UI_CMD_BT_NEXT;
			break;
		case CODE_PLAY_PAUSE:
			cmd.cmd = UI_CMD_BT_OK;
			break;
		case CODE_BT_MODE:
			cmd.cmd = UI_CMD_BT_MODE;
			break;
		#endif
		/*
		case CODE_AUX_MODE:
			cmd.cmd = UI_CMD_IR_AUX_MODE;
			break;
		case CODE_RCA_MODE:
			cmd.cmd = UI_CMD_IR_RCA_MODE;
			break;
		case CODE_OPT_MODE:
			cmd.cmd = UI_CMD_IR_OPT_MODE;
			break;
		case CODE_EQ_MUSIC:
			cmd.cmd = UI_CMD_EQ_MUSIC;
			break;
		case CODE_EQ_MOVIE:
			cmd.cmd = UI_CMD_EQ_MOVIE;
			break;
		case CODE_EQ_NEWS:
			cmd.cmd = UI_CMD_IR_HDMI_MODE;
			break;
			*/
        }
    }
    else if (EV_UI == event->type)
    {
		switch (event->code)
		{
			case CODE_UI_SD_LOAD:
				if (event->value == VALUE_FAIL)
				{
				}
				else if (event->value == VALUE_SUCCESS)
				{
					cmd.cmd = UI_CMD_SD_LOAD;
				}
				break;
			case CODE_UI_SD_IN:
#ifdef CONFIG_AUTO_UPDATE
				silan_check_update_file_save("/media/sd/sd00", "update.binary");
#endif
				cmd.cmd = UI_CMD_SD_IN;
				break;
			case CODE_UI_SD_UNLOAD:
				cmd.cmd = UI_CMD_SD_UNLOAD;
				break;
				
			case CODE_UI_USB_LOAD:
				if (event->value == VALUE_FAIL)
				{	
				}
				else if (event->value == VALUE_SUCCESS)
				{	
					cmd.cmd = UI_CMD_USB_LOAD;
				}
				break;
			case CODE_UI_USB_IN:
#ifdef CONFIG_AUTO_UPDATE
				memset(name, 0, 32);
				sprintf(name, "/media/usb/usb0%d", event->value);
				silan_check_update_file_save(name, "update.binary");
#endif
				cmd.cmd = UI_CMD_USB_IN;
				cmd.arg2 = event->value;
				break;	
			case CODE_UI_USB_UNLOAD:
				if (event->value == VALUE_FAIL)
					cmd.cmd = UI_CMD_USB_UNLOAD;
				break;
			default:
				return;
		}

    }
#ifdef CONFIG_CEC
	else if (EV_CEC == event->type) 
	{
		switch(event->code) {
			case CODE_CEC_ACTIVE_SOURCE:
				cmd.cmd = UI_CEC_ACTIVE_SOURCE;
				break;
			case CODE_CEC_INACTIVE_SOURCE:
				cmd.cmd = UI_CEC_INACTIVE_SOURCE;
				break;
		}
	}
#endif
    send_cmd_2_ui(&cmd);
}

/****************************************************************************
 * Name: init_silan_event
 *
 * Description:
 *    注册事件接收函数
 * 
 * Parameters:
 *
 * Returned Value:
 *    总是0
 * 
 * Assumptions:
 *
 ****************************************************************************/
static int init_silan_event(void)
{
    printf("%s %d\n", __func__, __LINE__);
    idjs_event_register(handle_ui_events, NULL);	
	ui_cmd_init();
    return 0;
}
//设置开机执行注册事件函数
app_entry(init_silan_event)

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
		case CMD_UI_NEXT:
	        cmd.cmd = UI_CMD_BT_NEXT;
			break;
		case CMD_UI_PREV:
	        cmd.cmd = UI_CMD_BT_PREV;
	        break;
	    case CMD_UI_MODE:
	        cmd.cmd = UI_CMD_BT_MODE;
	        break;
	    case CMD_UI_INC:
	        cmd.cmd = UI_CMD_BT_UP;
	        break;
	    case CMD_UI_DEC:
	        cmd.cmd = UI_CMD_BT_DOWN;
	        break;
        case CMD_UI_PLAY:
	        cmd.cmd = UI_CMD_BT_OK;
	        break;
       case CMD_KEY_POWER:
	        cmd.cmd = UI_CMD_BT_MUTE;
			break;
       case CMD_IR_MUTE:
	        cmd.cmd = UI_CMD_BT_MUTE;
			break;
       case CMD_IR_EQ1:
	        cmd.cmd = UI_CMD_EQ_MUSIC;
			break;
       case CMD_IR_EQ2:
	        cmd.cmd = UI_CMD_EQ_DIALOG;
			break;
       case CMD_IR_EQ3:
	        cmd.cmd = UI_CMD_EQ_MOVIE;
			break;
       case CMD_IR_LINE:
	        cmd.cmd = UI_CMD_IR_RCA_MODE;
			break;
       case CMD_IR_HDMI:
	        cmd.cmd = UI_CMD_IR_HDMI_MODE;
			break;
       case CMD_IR_AUX:
	        cmd.cmd = UI_CMD_IR_AUX_MODE;
			break;
       case CMD_IR_VOL_UP:
	        cmd.cmd = UI_CMD_BT_UP;
			break;
       case CMD_IR_VOL_DOWN:
	        cmd.cmd = UI_CMD_BT_DOWN;
			break;
       case CMD_IR_BT:
	        cmd.cmd = UI_CMD_IR_BT_MODE;
			break;
	   case CMD_IR_PLAY:
	        cmd.cmd = UI_CMD_BT_MUTE;
			break;
			
    }
    #if 0
    switch (event_cmd & 0x0f)
    {
		case CMD_UI_PWR:
	        cmd.cmd = UI_CMD_BT_POWER;
			break;
		case CMD_UI_MODE:
	        cmd.cmd = UI_CMD_BT_MODE;
	        break;
	    case CMD_UI_PREV:
	        cmd.cmd = UI_CMD_BT_DOWN;
	        break;
	    case CMD_UI_PLAY:
	        cmd.cmd = UI_CMD_BT_OK;
	        break;
	    case CMD_UI_NEXT:
	        cmd.cmd = UI_CMD_BT_UP;
	        break;
    }
    #endif
    return cmd;
}

