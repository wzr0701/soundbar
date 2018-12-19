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

/*???t?°ß?????3°ß?id*/
static pthread_t file_load_pid = -1;

bool seek_mute_play = false;
/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
/*°ßaa2?*/
extern void send_cmd_2_ui(ui_cmd_t *ui_cmd);
/*?°ß22?*/
static int file_load_thread(pthread_addr_t arg);
static int get_local_info(char *local_name, int *total_num, int *folder_num);

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: get_file_total
 *
 * Description:
 *    ??°ß°ß?2°Í°Ë?°Ë?°ß°ÈD?®§°ßa|®¨??®¢°ß1???t°ßoy
 * 
 * Parameters:
 *
 * Returned Value:
 * 	  ?®¢°ß1|®¨????t°ßoy
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
 *    ???t?°ß?????3°ß?°ß°ß??°ß2
 * 
 * Parameters:
 *    arg °ß°„a?°ß???|®¨????t|®¨???°Ë??
 *
 * Returned Value:
 * 	  0-3°ß|?a?®∫??°Ë?0-°ßo?®¨??°ß1
 * 
 * Assumptions:
 *
 ****************************************************************************/
int file_load_thread(pthread_addr_t arg)
{
	int ret;
	int total_num, folder_num;
	char *search_name = (char *) arg;

	//??°ß°ß????????°Ë??|®¨??®¢°ß1???t°ßoyo°ßa?®¢°ß1???t?D°ßoy
	if (get_local_info(search_name, &total_num, &folder_num) < 0) 
	{	//?????°ßo?®¨??°ß1
		printf("%s %d not find music:%s\n", __func__, __LINE__, search_name); 

		ret = -SL_UI_ERROR_REGISTER;
	}
	else
	{	//?????3°ß|1|
		//#ifdef SL_UI_DBG
		printf("%s %d, total_num:%d, folder_num:%d\n", __func__, __LINE__, total_num, folder_num);
		//#endif

		//?°ß°„UI2??°Ë?®¶?°ßa????®¶
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
 *    ?????????°Ï??°Ë??|®¨??®¢°ß1???t°ßoyo°ßa???t?D°ßoy
 * 
 * Parameters:
 *    local_name ?????|®¨???°Ë??
 *    total_num	?°Ë|®¨??|®¨??®¢°ß1???t?D°ßoy??
 *    folder_num ?°Ë|®¨??°ß?D???t|®¨????t?D°ßoy??
 *
 * Returned Value:
 * 	  0-3°ß|1|?®∫?-6-??°ß?D???t?D?®∫?-5-??°ß?D???t
 * 
 * Assumptions:
 *
 ****************************************************************************/
static int get_local_info(char *local_name, int *total_num, int *folder_num)
{
	int i, j;
	dir_elmt_t *dir_elm;
	int file_num; 
	//3?°ßo???£§2°Í°Ë?°Ë?°ß°ÈD?®§°ßa
	playlist_init();
	//???°ß°‰?°ß°‰°ß?D°ß°„??|®¨???t
	file_music_enumate(local_name, 0);
	//??????°Ï???t°ß°ÈD?®§°ßa
	fs_list_create_liner_list_for_fat();
	//??°ß°ß????t?D?®¢°ß1°ßoy
	*total_num = play_list_get_dir_total();
	if (*total_num <= 0) 
	{	//??°ß?D???t?D
		printf("no local folders %s!\n", local_name);
		return -SL_UI_ERROR_NO_FOLDER;
	}
	//??°ß°ß????t?®¢°ß1°ßoy
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
	{	//??°ß?D°ß°ß?o?°ß°„??|®¨???t
		printf("%s %d this part has null music!\n", __func__, __LINE__);
		return -SL_UI_ERROR_NO_FILE;
	}

	return SL_UI_ERROR_NULL;
}

/****************************************************************************
 * Name: handle_local
 *
 * Description:
 *    ?®§?|®¨?2°Í°Ë?°Ë???|°ß°Ë°ßa?®∫?????£§??3°ß??°ß??????t°ß°ÈD?®§°ßa
 * 
 * Parameters:
 *    local_media ?????|®¨??®§?|®¨???°ß????°Ë??
 *
 * Returned Value:
 * 
 * Assumptions:
 *
 ****************************************************************************/
void handle_local(const char* local_media)
{
	DIR *local_dir = NULL;

	//2a°ßo?USB°ß|°ß°ß?®§????t?D°ßo??°Ë??°ß11???°ß°„?a
	local_dir = opendir(local_media);
	if (NULL == local_dir) 
	{	//USB°ß|°ß°ß?®§????t?D??°ß°„?a°ßo?®¨??°ß1
		printf("%s:open dir failed \n", __func__, __LINE__);
	}
	else
	{	//USB°ß|°ß°ß?®§????t?D??°ß°„?a3°ß|1|
		//1??®§????t?D
		closedir(local_dir);

		if (file_load_pid > 0) 
		{	//usb???t???°ß°‰?°ß?????3°ß?????°ß2
			//°ßa?®∫?1usb???t???°ß°‰?°ß?????3°ß?
			pthread_cancel(file_load_pid);
			file_load_pid = -1;
		}

		if(pthread_create(&file_load_pid, NULL, (pthread_startroutine_t) file_load_thread, local_media) < 0)
		{	//??????°ÏUSB???t?°ß?????3°ß?°ßo?®¨??°ß1
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
 *    ????£§2°Í°Ë?°Ë?
 * 
 * Parameters:
 *    file_index °ß°„a2°Í°Ë?°Ë?|®¨????tD°ß°„o?
 *    playtime 2°Í°Ë?°Ë?????
 *
 * Returned Value:
 * 
 * Assumptions:
 *
 ****************************************************************************/
void handle_local_music_play(int file_index, int playtime)
{
	//??°ß°ß?2°Í°Ë?°Ë???
	play_list_item_t item;
	if(play_list_get_file_byindex(&item, file_index) == 0)
	{	
		player_process_cmd(NP_CMD_PLAY, item.path, 0, NULL, NULL);
		usleep(100);
		if(playtime > 0)
		{	//seek|®¨?°ß|????2°Í°Ë?°Ë?|®¨?????
			player_process_cmd(NP_CMD_SEEK, item.path, playtime * 1000, NULL, NULL);
			usleep(100);
		}
	}
}

/****************************************************************************
 * Name: reset_playlist
 *
 * Description:
 *    3?°ßo???£§2°Í°Ë?°Ë?°ß°ÈD?®§°ßa
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
	//3?°ßo???£§2°Í°Ë?°Ë?°ß°ÈD?®§°ßa
	playlist_init();
}

/****************************************************************************
 * Name: time2str
 *
 * Description:
 *    °ß°„?°ß°Ë?°ßo?®§???®¢a?®¢??°Ë????
 * 
 * Parameters:
 *    curtime |®¨?®§???°ßo?®§??
 *    totaltime ?®¢°ß1°ßo?®§??
 *    str ?°ß?°ßo??®¢a??o°ß?|®¨??®¢??°Ë????
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



