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

/*???t?��?????3��?id*/
static pthread_t file_load_pid = -1;

bool seek_mute_play = false;
/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/*��aa2?*/
extern void send_cmd_2_ui(ui_cmd_t *ui_cmd);
/*?��22?*/
static int file_load_thread(pthread_addr_t arg);
static int get_local_info(char *local_name, int *total_num, int *folder_num);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: get_file_total
 *
 * Description:
 *    ??����?2���?��?����D?����a|��??����1???t��oy
 * 
 * Parameters:
 *
 * Returned Value:
 * 	  ?����1|��????t��oy
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
 *    ???t?��?????3��?����??��2
 * 
 * Parameters:
 *    arg ����a?��???|��????t|��???��??
 *
 * Returned Value:
 * 	  0-3��|?a?��??��?0-��o?��??��1
 * 
 * Assumptions:
 *
 ****************************************************************************/
int file_load_thread(pthread_addr_t arg)
{
	int ret;
	int total_num, folder_num;
	char *search_name = (char *) arg;

	//??����????????��??|��??����1???t��oyo��a?����1???t?D��oy
	if (get_local_info(search_name, &total_num, &folder_num) < 0) 
	{	//?????��o?��??��1
		printf("%s %d not find music:%s\n", __func__, __LINE__, search_name); 

		ret = -SL_UI_ERROR_REGISTER;
	}
	else
	{	//?????3��|1|
		//#ifdef SL_UI_DBG
		printf("%s %d, total_num:%d, folder_num:%d\n", __func__, __LINE__, total_num, folder_num);
		//#endif

		//?����UI2??��?��?��a????��
		ui_cmd_t cmd;
		cmd.cmd = UI_CMD_FILES_IS_LOAD;
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
 *    ?????????��??��??|��??����1???t��oyo��a???t?D��oy
 * 
 * Parameters:
 *    local_name ?????|��???��??
 *    total_num	?��|��??|��??����1???t?D��oy??
 *    folder_num ?��|��??��?D???t|��????t?D��oy??
 *
 * Returned Value:
 * 	  0-3��|1|?��?-6-??��?D???t?D?��?-5-??��?D???t
 * 
 * Assumptions:
 *
 ****************************************************************************/
static int get_local_info(char *local_name, int *total_num, int *folder_num)
{
	int i, j;
	dir_elmt_t *dir_elm;
	int file_num; 
	//3?��o???��2���?��?����D?����a
	playlist_init();
	//???����?���䡧?D����??|��???t
	file_music_enumate(local_name, 0);
	//??????��???t����D?����a
	fs_list_create_liner_list_for_fat();
	//??����????t?D?����1��oy
	*total_num = play_list_get_dir_total();
	if (*total_num <= 0) 
	{	//??��?D???t?D
		printf("no local folders %s!\n", local_name);
		return -SL_UI_ERROR_NO_FOLDER;
	}
	//??����????t?����1��oy
	for (i=0, j=0; i<(*total_num); ++i) {
		dir_elm = play_list_get_direlm_byindex(i);
		file_num = dir_elm->file_num;
		if (0 != file_num)
		{
			j++;
		}
	}
	*folder_num = j;

	if (0 == (*folder_num)) 
	{	//??��?D����?o?����??|��???t
		printf("%s %d this part has null music!\n", __func__, __LINE__);
		return -SL_UI_ERROR_NO_FILE;
	}

	return SL_UI_ERROR_NULL;
}

/****************************************************************************
 * Name: handle_local
 *
 * Description:
 *    ?��?|��?2���?��???|���衧a?��?????��??3��??��??????t����D?����a
 * 
 * Parameters:
 *    local_media ?????|��??��?|��???��????��??
 *
 * Returned Value:
 * 
 * Assumptions:
 *
 ****************************************************************************/
void handle_local(const char* local_media)
{
	DIR *local_dir = NULL;

	//2a��o?USB��|����?��????t?D��o??��??��11???����?a
	local_dir = opendir(local_media);
	if (NULL == local_dir) 
	{	//USB��|����?��????t?D??����?a��o?��??��1
		printf("%s:open dir failed \n", __func__, __LINE__);
	}
	else
	{	//USB��|����?��????t?D??����?a3��|1|
		//1??��????t?D
		closedir(local_dir);

		if (file_load_pid > 0) 
		{	//usb???t???����?��?????3��?????��2
			//��a?��?1usb???t???����?��?????3��?
			pthread_cancel(file_load_pid);
			file_load_pid = -1;
		}

		if(pthread_create(&file_load_pid, NULL, (pthread_startroutine_t) file_load_thread, local_media) < 0)
		{	//??????��USB???t?��?????3��?��o?��??��1
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

/****************************************************************************
 * Name: handle_local_music_play
 *
 * Description:
 *    ????��2���?��?
 * 
 * Parameters:
 *    file_index ����a2���?��?|��????tD����o?
 *    playtime 2���?��?????
 *
 * Returned Value:
 * 
 * Assumptions:
 *
 ****************************************************************************/
void handle_local_music_play(int file_index, int playtime)
{
	//??����?2���?��???
	play_list_item_t item;
	if(play_list_get_file_byindex(&item, file_index) == 0)
	{	
		player_process_cmd(NP_CMD_PLAY, item.path, 0, NULL, NULL);
		usleep(100);
		if(playtime > 0)
		{	//seek|��?��|????2���?��?|��?????
			player_process_cmd(NP_CMD_SEEK, item.path, playtime * 1000, NULL, NULL);
			usleep(100);
		}
	}
}

/****************************************************************************
 * Name: reset_playlist
 *
 * Description:
 *    3?��o???��2���?��?����D?����a
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
	//3?��o???��2���?��?����D?����a
	playlist_init();
}

/****************************************************************************
 * Name: time2str
 *
 * Description:
 *    ����?����?��o?��???��a?��??��????
 * 
 * Parameters:
 *    curtime |��?��???��o?��??
 *    totaltime ?����1��o?��??
 *    str ?��?��o??��a??o��?|��??��??��????
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



