/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <apps/builtin.h>
#include <arch/chip/silan_timer.h>
#include <fcntl.h> /* For O_* constants */
#include <nuttx/audio/silan_audio_api.h>
#include <nuttx/config.h>
#include <nuttx/sys_conf.h>
#include <nxplayer.h>
#include <mqueue.h>
#include <player_cmd.h>
#include "sl_ui_cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> /* For mode constants */
#include "tone_data.h"
#include "sl_ui_io_led.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#ifdef CONFIG_WIRELESS
#ifdef CONFIG_BOARD_SHENZHEN
#define GPIO2_13_ 45
#define GPIO2_7_ 39
#define GPIO2_8_ 40
#define CONFIG_MHD_IRQ_PIN_ 43
#define CONFIG_MHD_RESET_PIN_ 40
#else

#define GPIO1_2 2
#define GPIO1_3 3
#define GPIO1_4 4
#endif
#endif

#if defined(CONFIG_CHIP_SC6138)
const char *g_product_version = "product_version_6138A-1-0";
#elif defined(CONFIG_BOARD_AP_8835C)
const char *g_product_version = "product_version_8835C-1-0";
#elif defined(CONFIG_BOARD_AP_5864B)
const char *g_product_version = "product_version_5864B-1-0";
#elif defined(CONFIG_BOARD_AP_5864B_2)
const char *g_product_version = "product_version_5864B-2-0";
#endif

#define PARTITION_LEN   64
static char flash_part[CONFIG_MTD_PARTITIONS][PARTITION_LEN] = {
#if CONFIG_CHIP_SC6138
#if CONFIG_FLASH_4M
	"boot,	   0x00000000, 0x00008000",
	   "btupdate1,0x00008000, 0x00001000",
	   "btupdate2,0x00009000, 0x00001000",
	   "btconf,   0x0000a000, 0x00008000",
	   "image2,   0x00012000, 0x00200000",
	   "sysconf1, 0x00212000, 0x00001000",
	   "sysconf2, 0x00213000, 0x00001000",
	   "sysdata,  0x00214000, 0x00001000",
	   "wifibin,  0x00215000, 0x00070000",
	   "fs, 	  0x00285000, 0x00001000",
	   "userspace,0x00286000, 0x00001000",
	   "tonespace,0x00287000, 0x00030000",
	   "table,	  0x002b7000, 0x00012000",
	   "chfnt,	  0x002c9000, 0x000f5000",
	   "backup,   0x003be000, 0x000320000"
#else
	"boot,     0x00000000, 0x00008000",
	"btupdate1,0x00008000, 0x00001000",
	"btupdate2,0x00009000, 0x00001000",
	"btconf,   0x0000a000, 0x00008000",
	"image2,   0x00012000, 0x00300000",
	"sysconf1, 0x00312000, 0x00001000",
	"sysconf2, 0x00313000, 0x00001000",
	"sysdata,  0x00314000, 0x00001000",
	"wifibin,  0x00315000, 0x00070000",
	"fs,       0x00385000, 0x00001000",
	"userspace,0x00386000, 0x00023000",
	"tonespace,0x003a9000, 0x00030000",		//128K
	"table,    0x003d9000, 0x00012000",		//72K
	"chfnt,    0x003eb000, 0x000f5000",		//979K
	"backup,   0x004e0000, 0x00320000"
#endif
#elif CONFIG_CHIP_AP1508
	"boot,     0x00020000, 0x00008000",
	"btupdate1,0x00028000, 0x00001000",
	"btupdate2,0x00029000, 0x00001000",
	"btconf,   0x0002a000, 0x00008000",
	"image2,   0x00032000, 0x00200000",
	"sysconf1, 0x00232000, 0x00001000",
	"sysconf2, 0x00233000, 0x00001000",
	"sysdata,  0x00234000, 0x00001000",
	"wifibin,  0x00235000, 0x00070000",
	"fs,       0x002a5000, 0x00001000",
	"userspace,0x002a6000, 0x00001000",
	"tonespace,0x002a7000, 0x00030000",
	"table,    0x002d7000, 0x00012000",
	"chfnt,    0x002e9000, 0x000f5000",
	"backup,   0x003de000, 0x00422000"
#endif
};
struct tone_data datas[] = {
    { 0, 16016 },
    { 16016, 16016 },
    { 32032, 17644 },
    { 49676, 18668 },
    { 68344, 16044 },
    { 84388, 18668 },
    { 103056, 3260 },
    { 106316, 9916 },
    { 116232, 10476 },
    { 126708, 12076 },
    { 138784, 26840 },
    { 165624, 22416 },
    { 188040, 22428 },
    { 210468, 23100 },
    { 233568, 20860 },
    { 254428, 26588 },
    { 281016, 23116 },
    { 304132, 21944 },
    { 326076, 22044 },
    { 348120, 11994 },
    { 360114, 16044 },
    { 376158, 16044 },
    { 392202, 9644 },
    { 401846, 9644 },
    { 411490, 27116 },
    { 438606, 48044 },
    { 486650, 9580 },
};

static struct ui_s *ui_mq = NULL;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/*
	outMode		输出模式
	inMode		输入模式
	chnNum		输出通道数
	spdifNum	使用的SPDIF通道号
*/
extern void player_paramter_set_init(AUDIO_OUT_MODE outMode, AUDIO_IN_MODE inMode, int chnNum, int spdifNum);
extern void set_flash_partition_info(char info[][64]);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: check_is_playing_sd
 *
 * Description:
 *    检查是否正在播放SD卡内容
 *
 * Parameters:
 *
 * Returned Value:
 *    true  正在播放SD卡内容
 *    false 不是正在播放SD卡内容
 *
 * Assumptions:
 *
 ****************************************************************************/
bool check_is_playing_sd(void)
{
    ui_info_t info = sc8836_get_all_play_info();
	return (info.ui_media == MEDIA_SD);
}

/****************************************************************************
 * Name: check_is_playing_usb
 *
 * Description:
 *    检查是否正在播放U盘内容
 *
 * Parameters:
 *
 * Returned Value:
 *    true  正在播放U盘内容
 *    false 不是正在播放U盘内容
 *
 * Assumptions:
 *
 ****************************************************************************/
bool check_is_playing_usb(void)
{
    ui_info_t info = sc8836_get_all_play_info();
    return  (info.ui_media == MEDIA_USB);
}

/****************************************************************************
 * Name: send_cmd_2_ui
 *
 * Description:
 *    向UI消息队列发送消息
 *
 * Parameters:
 *    ui_cmd    要发送的消息
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void send_cmd_2_ui(ui_cmd_t *ui_cmd)
{
    if (NULL != ui_mq)
    {
        if (mq_send(ui_mq->mq, (FAR const char *)ui_cmd, sizeof(ui_cmd_t), CONFIG_UI_MSG_PRIO))
        {
            printf("%s mq_send error:%d\n", __func__, errno);
        }
    }
}

/****************************************************************************
 * Name: sl_appentry_module_switch
 *
 * Description:
 *
 *
 * Parameters:
 *    ui_cmd
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
bool sl_appentry_module_switch(enum appentry_module_e app_module)
{
	switch(app_module) {
		case APPENTRY_MODULE_CEC:
			return true;
		default:
			return false;
	}
}

/****************************************************************************
 * Name: sys_early_init
 *
 * Description:
 *    开机系统初始化程序
 *
 * Parameters:
 *
 * Returned Value:
 *    总是0
 *
 * Assumptions:
 *
 ****************************************************************************/
int sys_early_init(void)
{
    set_flash_partition_info(flash_part);

    return 0;
}

/****************************************************************************
 * Name: ui_cmd_init
 *
 * Description:
 *    ui命令消息队列初始化
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ui_cmd_init(void)
{
    struct mq_attr attr;
    ui_mq = (struct ui_s *)malloc(sizeof(struct ui_s));
    if (NULL == ui_mq)
    {
        printf("mq malloc failed\n");
    }
    else
    {
        memset(ui_mq, 0, sizeof(struct ui_s));
        attr.mq_maxmsg = 64;
        attr.mq_msgsize = sizeof(ui_cmd_t);
        attr.mq_curmsgs = 0;
        attr.mq_flags = 0;
        sprintf(ui_mq->mqname, "mq_common");
        //snprintf(Ui->mqname, sizeof(Ui->mqname), "/tmp/%0lx", (unsigned long)((uintptr_t)Ui));
        ui_mq->mq = mq_open(ui_mq->mqname, O_RDWR | O_CREAT | O_NONBLOCK, 0644, &attr);
        if (ui_mq->mq < 0)
        {
            printf("mq_open error!\n");
        }
    }
}

/****************************************************************************
 * Name: user_early_init
 *
 * Description:
 *    开机用户初始化程序
 *
 * Parameters:
 *
 * Returned Value:
 *    总是0
 *
 * Assumptions:
 *
 ****************************************************************************/
int user_early_init(void)
{
    printf("%s %d\n", __func__, __LINE__);
/////////////////////////////////////////////////////////////////////////
    io_early_set();
/////////////////////////////////////////////////////////////////////////

#if (BYPASS_MODE || INNERADC_MODE)
    player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_INNER_IN, 2, 4);
#else
    #if (MIX_6_CH_MIC)
    player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_EXTRA_MASTER_IN, 6, 4);
    #else
	//IIS主模式输出，IIS模式输入，2声道输出，默认SPDIF 0号通道
    player_paramter_set_init(AUDIO_EXTRA_CODEC_MASTER_OUT, AUDIO_INNER_IN, 2, 0);
    //player_paramter_set_init(AUDIO_INNER_CODEC_OUT, AUDIO_EXTRA_SLAVE_IN, 2, 0);
    #endif
#endif
    return 0;
}

/****************************************************************************
 * Name: user_late_init
 *
 * Description:
 *    开机用户初始化程序
 *
 * Parameters:
 *
 * Returned Value:
 *    总是0
 *
 * Assumptions:
 *
 ****************************************************************************/
int user_late_init(void)
{
    return 0;
}

/****************************************************************************
 * Name: user_timer_isr
 *
 * Description:
 *    计时器默认中断服务处理程序
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void user_timer_isr(void)
{

}
