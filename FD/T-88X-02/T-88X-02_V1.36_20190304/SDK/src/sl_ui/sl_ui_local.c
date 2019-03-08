/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <dirent.h>
#include <filelist.h>
#include <nxplayer.h>
#include <player_cmd.h>
#include "sl_ui_def.h"
#include "sl_ui_cmd.h"
#include "sl_ui_struct.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "sl_ui_handle.h"


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/
//static char *audiotype2str[AUDIO_MAX] = {"unknown", "mp3", "alac", "aac", "wma", "flac", "ape", "ac3", "dts", "wav", "ogg", "spdac3", "spddts", "pcm"};
//static char *media2str[MEDIA_MAX] = {"unknown", "sd", "usb", "bluetooth", "airplay", "dlna", "radio"};

int usb_play_cnt = 100;

extern  bool usb_prev_flag;
extern int folder_index_cnt;


int folder_index_tab[255][2];
int folder_total_num = 0;
/*文件加载线程id*/
static pthread_t file_load_pid = -1;
//extern unsigned char power_on_usb_sd_auto_play;
/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/*外部*/
extern void send_cmd_2_ui(ui_cmd_t *ui_cmd);
/*内部*/
static int file_load_thread(pthread_addr_t arg);
static int get_local_info(char *local_name, int *total_num, int *folder_num);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: get_file_total
 *
 * Description:
 *    获取播放列表的总文件数
 *
 * Parameters:
 *
 * Returned Value:
 * 	  总的文件数
 *
 * Assumptions:
 *
 ****************************************************************************/
int get_file_total(void)
{
	return play_list_get_file_total();
}

/****************************************************************************
 * Name: file_load_thread
 *
 * Description:
 *    文件加载线程入口
 *
 * Parameters:
 *    arg 要加载的文件的路径
 *
 * Returned Value:
 * 	  0-成为，非0-失败
 *
 * Assumptions:
 *
 ****************************************************************************/
int file_load_thread(pthread_addr_t arg)
{
	int ret;
	int total_num, folder_num;
	char *search_name = (char *) arg;

	//获取搜索路径的总文件数和总文件夹数
	if (get_local_info(search_name, &total_num, &folder_num) < 0)
	{	//搜索失败
		printf("%s %d not find music:%s\n", __func__, __LINE__, search_name);

		ret = -SL_UI_ERROR_REGISTER;
	}
	else
	{	//搜索成功
		#ifdef SL_UI_DBG
		printf("%s %d, total_num:%d, folder_num:%d\n", __func__, __LINE__, total_num, folder_num);
		#endif

		//向UI层发送消息
		ui_cmd_t cmd;
		cmd.cmd = UI_CMD_FILES_IS_LOAD;
		cmd.arg.url = search_name;
		cmd.arg2 = total_num;
		cmd.mode = folder_num;
		send_cmd_2_ui(&cmd);
		ret = SL_UI_ERROR_NULL;
	}

	file_load_pid = -1;
	return ret;
}

/****************************************************************************
 * Name: get_local_info
 *
 * Description:
 *    搜索指定路径的总文件数和文件夹数
 *
 * Parameters:
 *    local_name 搜索的路径
 *    total_num	返回的总文件夹数目
 *    folder_num 返回有文件的文件夹数目
 *
 * Returned Value:
 * 	  0-成功，-6-没有文件夹，-5-没有文件
 *
 * Assumptions:
 *
 ****************************************************************************/
static int get_local_info(char *local_name, int *total_num, int *folder_num)
{
	int i, j;
	dir_elmt_t *dir_elm;
	int file_num;
	int total_file_index = 0;
	//初始化播放列表
	playlist_init();
	//枚举所有音频文件
	file_music_enumate(local_name, 0);
	//创建文件列表
	fs_list_create_liner_list_for_fat();
	//获取文件夹总数
	*total_num = play_list_get_dir_total();
	if (*total_num <= 0)
	{	//没有文件夹
		printf("no local folders %s!\n", local_name);
		return -SL_UI_ERROR_NO_FOLDER;
	}
	//获取文件总数
	for (i=0, j=0; i<(*total_num); ++i)
	{
		dir_elm = play_list_get_direlm_byindex(i);
		file_num = dir_elm->file_num;
		if (0 != file_num)
		{
			folder_index_tab[j][0] = total_file_index;
			total_file_index += file_num;
			folder_index_tab[j][1] = total_file_index - 1;
			//printf("%s-------- folder_index_tab[j][1]: %d \n",__func__,folder_index_tab[j][1]);
			/*printf("%s-------- path:%s--no.: %d--parent:%d--child:%d---index:%d--num:%d--dir:%d--folder_index_tab[%d][0]:%d--folder_index_tab[%d][1]:%d \n",
			__func__,dir_elm->path_name, i, dir_elm->parent_index, dir_elm->child_index, dir_elm->file_index,
			dir_elm->file_num, dir_elm->dir_num,j, folder_index_tab[j][0],j, folder_index_tab[j][1]);*/
			j++;
		}
	}
	*folder_num = j;
	folder_total_num = j;
	//printf("%s-------- folder_total_num: %d \n",__func__,folder_total_num);

	if (0 == (*folder_num))
	{	//没有任何音频文件
		printf("%s %d this part has null music!\n", __func__, __LINE__);
		return -SL_UI_ERROR_NO_FILE;
	}

	return SL_UI_ERROR_NULL;
}

/****************************************************************************
 * Name: handle_local
 *
 * Description:
 *    本地播放处理，启动线程加载文件列表
 *
 * Parameters:
 *    local_media 搜索的本地媒体路径
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void handle_local(const char* local_media)
{
	DIR *local_dir = NULL;

	//测试USB设备文件夹是否能够打开
	local_dir = opendir(local_media);
	if (NULL == local_dir)
	{	//USB设备文件夹打开失败
		printf("%s:open dir failed \n", __func__, __LINE__);
	    //  power_on_usb_sd_auto_play=1;
	}
	else
	{	//USB设备文件夹打开成功
		//关闭文件夹
		closedir(local_dir);

		if (file_load_pid > 0)
		{	//usb文件枚举加载线程存在
			//停止usb文件枚举加载线程
			pthread_cancel(file_load_pid);
			file_load_pid = -1;
			usleep(10);
		}

		if(file_load_pid == -1)
		{
			if(pthread_create(&file_load_pid, NULL, (pthread_startroutine_t) file_load_thread,
					(pthread_addr_t)local_media) < 0)
			{	//创建USB文件加载线程失败
				printf("%s:create thread to get usb file info fail\n", __func__);
			}
			else
			{
				if (pthread_detach(file_load_pid) < 0)
				{
					printf("%s:detach thread fail\n", __func__);
				}
				else
				{
					printf("%s:detach thread success\n", __func__);
				}
			}
		}
	}
}

/****************************************************************************
 * Name: handle_local_music_play
 *
 * Description:
 *    启动播放
 *
 * Parameters:
 *    file_index 要播放的文件序号
 *    playtime 播放位置
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void handle_local_music_play(int file_index, int playtime)
{
	//获取播放项
	play_list_item_t item;
	int file_rel_pos;
	//printf("%s:file_index === %d\n", __func__,file_index);
	if(play_list_get_file_byindex(&item, file_index) == 0)
	{
		//printf("%s:folder_index_tab[%d][0] === %d----------folder_index_tab[%d][1] === %d\n", __func__,folder_index_cnt,folder_index_tab[folder_index_cnt][0],folder_index_cnt,folder_index_tab[folder_index_cnt][1]);
		if(file_index < folder_index_tab[0][1])
		{
			file_rel_pos = file_index;
		}
		else
		{
			file_rel_pos = file_index - folder_index_tab[folder_index_cnt][0];
		}
		
		display_ui_usb_number(file_rel_pos);
		player_process_cmd(NP_CMD_VOLUME_SET, NULL, 0, NULL, NULL);
		usb_play_cnt = 0;
		usb_prev_flag = false;
		//发送命令进行播放
		player_process_cmd(NP_CMD_PLAY, item.path, 0, NULL, NULL);
		usleep(100);

		if(playtime > 0)
		{	//seek到上次播放的位置
			player_process_cmd(NP_CMD_SEEK, item.path, playtime * 1000, NULL, NULL);
		}
	}
}

/****************************************************************************
 * Name: reset_playlist
 *
 * Description:
 *    初始化播放列表
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void reset_playlist(void)
{
	//初始化播放列表
	playlist_init();
}

/****************************************************************************
 * Name: time2str
 *
 * Description:
 *    音乐时间转字符串
 *
 * Parameters:
 *    curtime 当前时间
 *    totaltime 总时间
 *    str 接收转换后的字符串
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void time2str(int curtime, int totaltime, char *str)
{
	int cur_min, cur_sec, tot_min, tot_sec;
	cur_min = curtime/60;
	cur_sec = curtime%60;
	tot_min = totaltime/60;
	tot_sec = totaltime%60;
	sprintf(str, "%02d:%02d/%02d:%02d", cur_min, cur_sec, tot_min, tot_sec);
}

