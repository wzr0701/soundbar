/*--------------------------------------------------------------------------*/
/**@file fm_rcv.c
   @brief FMÄ£¿éÇý¶¯½Ó¿Ú
   @details FMÄ£¿éÇý¶¯²Ù×÷º¯Êý
   @author LZZ
   @date 2010-08-17
   @note none
*/
/*----------------------------------------------------------------------------*/

#include <errno.h>
#include <fcntl.h>
#include <nuttx/audio/sladsp_ae.h>
#include <nuttx/power/pm.h>
#include <nxplayer.h>
#include "player_cmd.h"
#include <pthread.h>
#include <silan_gpio.h>
#include <silan_addrspace.h>
#include <silan_resources.h>
#include "sl_lcd.h"
#include "sl_ui_cmd.h"
#include "sl_ui_handle.h"
#include "sl_ui_local.h"
#include <string.h>
#include <sys/ioctl.h>
#include "tone_data.h"
//#include "QN8035.h"
//#include "RDA5807P.h"
#include "rda5807.h"
#include "at24c02.h"
#include "sl_ui_display.h"
#include "sl_ui_fm.h"
#include "sl_ui_cmd_deal.h"



#define FM_RDA5807_EN

//#define WriteToDev



#define FM_VOL_MAX           15

bool fm_scan_end_flag = false;

extern ui_cmd_t get_mq_msg(void);
extern unsigned char g_FMType_RX;
extern bool fm_test_flag;
extern bool fm_scan_start;
extern int ui_source_select;



u8 Frequency_Save[MAX_CH_NUM];
u8 Fre_Total_Num = 0;

u16 fmFrequency = 875;
u8 Cur_Fre_Num = 1;
u8 volume=15;

char fmFre_lev = 0;
u16 fmFre[3] = {900,960,1043};


/*----------------------------------------------------------------------------*/
/**@brief FMÄ£Ê½Ö÷º¯Êý
   @param ÎÞ
   @return ÍË³öFMÄ£Ê½ºó½«Òª½øÈëµÄÄ£Ê½
   @note
*/
/*----------------------------------------------------------------------------*/

void Delay5Ms(u8 t_ms)
{
	usleep(5000*t_ms);
}


/*----------------------------------------------------------------------------*/
/**@brief FMÄ£Ê½Ö÷º¯Êý
   @param ÎÞ
   @return ÍË³öFMÄ£Ê½ºó½«Òª½øÈëµÄÄ£Ê½
   @note
*/
/*----------------------------------------------------------------------------*/


unsigned char FM_Mode(void)
{
#if 1
	radio_on();	
#else
	fm_rx_init();
	Delay5Ms(15);   //ÎªÁË¼æÈÝ5807P£¬ÔÚ´Ë¼ÓÉÏÑÓÊ±
	fm_rx_set_freq(fmFrequency);
	fm_rx_set_vol(FM_VOL_MAX);
#endif
#if 0
	u8 i;

#ifndef WriteToDev
	fmFrequency = at24c02_read_one_byte(MEM_FM_FREQUENCY);
	fmFrequency += FM_MIN;
	if((fmFrequency > FM_MAX) || (fmFrequency < FM_MIN))
		fmFrequency = FM_MIN;

	//printf("fmFrequency ===%d\r\n",fmFrequency);

	Fre_Total_Num = at24c02_read_one_byte(MEM_FRE_TOTAL_NUM);
	if(Fre_Total_Num > MAX_CH_NUM)
		Fre_Total_Num = 0;

	//printf("Fre_Total_Num ===%d\r\n",Fre_Total_Num);

	Cur_Fre_Num = at24c02_read_one_byte(MEM_CUR_FRE_NUM);
	if(Cur_Fre_Num > Fre_Total_Num)
		Cur_Fre_Num = 0;

	//printf("Cur_Fre_Num ===%d\r\n",Cur_Fre_Num);

	for(i = 0;i < Fre_Total_Num;i++)
	{
		Frequency_Save[i] = at24c02_read_one_byte(MEM_SAVE_CHANNEL + i);
		if(Frequency_Save[i] > (FM_MAX- FM_MIN))
			Frequency_Save[i] = 0;
		//printf("Frequency_Save[%d]===%d\r\n",Frequency_Save[i]);
	}
#endif

	if(fm_test_flag == true)
	{
		fmFrequency = 900;
	}
	volume=15;

	fm_rx_init();
	Delay5Ms(15);   //ÎªÁË¼æÈÝ5807P£¬ÔÚ´Ë¼ÓÉÏÑÓÊ±
	fm_rx_set_freq(fmFrequency);
#ifdef FM_CTL_VOL
	fm_rx_set_vol(volume);
#else
	fm_rx_set_vol(FM_VOL_MAX);
#endif
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief FMÄ£Ê½ÏÂµÄ×Ô¶¯ËÑÌ¨
   @param ÎÞ
   @return ÎÞ
   @note
*/
/*----------------------------------------------------------------------------*/
 u8 FmScan(int mode,int dir)
{
	u8 temp;
	static u16 CurFrequency = 0;
	ui_cmd_t cmdq;
	temp = 1;

	//bt_cmd_fmscan_start();
	fm_scan_end_flag = false;
	

	if(mode == 1)
	{
		fmFrequency = 875;    
		Fre_Total_Num = 0;
	}
	
	CurFrequency = fmFrequency;    //±£´æÆðÊ¼Æµµã

	//fm_rx_set_vol(0);
	rda5807_mute();
	pa_mute_ctrl(true);
	//set_channel_mixvol_by_mode(ui_source_select);

	if(mode == 0)
	{
		if(dir == 0)
		{
			fmFrequency --;
		}
		else
		{
			fmFrequency ++;
		}
	}

	while(1)
	{
		cmdq=get_mq_msg();
		if(cmdq.cmd==UI_CMD_FM_SCAN||cmdq.cmd==UI_CMD_PLAY_PAUSE||cmdq.cmd==UI_CMD_FM_HALF_SCAN_ADD||cmdq.cmd==UI_CMD_FM_HALF_SCAN_SUB)
		{
			//fmFrequency --;
			fm_rx_set_freq(fmFrequency);
			rda5807_unmute();
			//fm_rx_set_vol(volume);
			pa_mute_ctrl(false);
			fm_scan_start =      false;
			fm_scan_end_flag = true;
			break;
		}
		
		if(cmdq.cmd == UI_CMD_MODE)
		{
			cmdq.cmd = UI_CMD_MODE;
			send_cmd_2_ui(&cmdq);
			break;
		}
		
		display_ui_fm(0);

		if (fm_rx_seek(fmFrequency))
		{
			fm_rx_set_freq(fmFrequency);
			rda5807_unmute();
			//fm_rx_set_vol(volume);
			pa_mute_ctrl(false);

			if(mode == 0)
			{
				fm_scan_start =      false;
				fm_scan_end_flag = true;

				//fre_manual_save();
				//Delay5Ms(200);
				
				break;
			}
			else
			{
				SaveChan(temp);
				Fre_Total_Num = temp;
				Cur_Fre_Num = Fre_Total_Num;
				
				display_ui_fm(1);
				Delay5Ms(200);

				if(Fre_Total_Num >= MAX_CH_NUM)
				{
					fm_scan_end_flag = true;
					break;
				}
				else
				{
					temp++;
				}
				
				//fm_rx_set_vol(0);
				rda5807_mute();
				pa_mute_ctrl(true);
			}
			
		}
		
		if(mode == 0)
		{
			if(dir == 0)
			{
				fmFrequency --;
			}
			else
			{
				fmFrequency ++;
			}
		}
		else
		{
			fmFrequency ++;
		}
		
		if(fmFrequency > FM_MAX)
		{
			fmFrequency = FM_MIN;
		}
		else if(fmFrequency < FM_MIN)
		{
			fmFrequency = FM_MAX;
		}
			
		if(fmFrequency == CurFrequency)     //Ò»¸öÑ­»·
		{
			//fm_rx_set_freq(fmFrequency);	
			fm_scan_end_flag = true;
			break;
		}
	}

	if(mode == 1)
	{
		if(Fre_Total_Num == 0)
		{
			Fre_Total_Num = 0;
			Cur_Fre_Num = 0;
			Frequency_Save[0] = 0;
		}
		temp = Fre_Total_Num;
		//at24c02_write_one_byte(MEM_FRE_TOTAL_NUM, temp);
		//Delay5Ms(10);
	
	
		if(Fre_Total_Num>=1)
		{
			Cur_Fre_Num = 1;
			fmFrequency= Frequency_Save[0]+FM_MIN;
			
			//if(Fre_Total_Num>1)
			//fm_rx_set_freq(fmFrequency);	
				
			//temp=fmFrequency-FM_MIN;
			//at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);
			//Delay5Ms(10);
			//at24c02_write_one_byte(MEM_CUR_FRE_NUM ,Cur_Fre_Num);
			//Delay5Ms(10);
		}
	}

	Delay5Ms(10);
	display_ui_fm(0);
	if(mode != 0)
	{
		//fm_rx_set_vol(volume);
		rda5807_unmute();
		pa_mute_ctrl(false);
	}
}

 

/**************************************************



**************************************************/
void fre_manual_save(void)
{
	int temp;

	temp = Search_channel_in_save();

	//printf("Search_channel_in_save === %d\r\n",temp);
	//fm_manual_save_status = false;
	//fm_manual_save_cnt = 200;
	
	if(temp > 0)
	{
		Cur_Fre_Num = temp;
		display_ui_fm(1);
	}
	else
	{	
		if(Fre_Total_Num < MAX_CH_NUM)
		{
			//printf("Fre_Total_Num1 === %d\r\n",Fre_Total_Num);
			Fre_Total_Num = Fre_Total_Num +1;
			SaveChan(Fre_Total_Num);
			Cur_Fre_Num = Fre_Total_Num;
			//printf("Fre_Total_Num2 === %d\r\n",Fre_Total_Num);
			display_ui_fm(1);
			temp = Fre_Total_Num;
			at24c02_write_one_byte(MEM_FRE_TOTAL_NUM, temp);
			Delay5Ms(10);
			at24c02_write_one_byte(MEM_CUR_FRE_NUM ,temp);
			Delay5Ms(10);
		}
		
	}

	//usleep(500000);
	//display_ui_fm(0);
}

/**************************************************



**************************************************/
void fre_save(void)
{
   unsigned char temp;
   temp=fmFrequency-FM_MIN;
   //at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);
   //Delay5Ms(10);
}

/************************************************



************************************************/
void fre_num_play(unsigned char c_num)
{
	unsigned char temp;
	if((c_num>0)&&(c_num<=Fre_Total_Num))
	{
		Cur_Fre_Num=c_num;
		fmFrequency = Frequency_Save[Cur_Fre_Num-1] + FM_MIN;
		fm_rx_set_freq(fmFrequency);
		display_ui_fm(1);

		temp=fmFrequency-FM_MIN;
		at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);
		Delay5Ms(10);

		temp=Cur_Fre_Num;
		at24c02_write_one_byte(MEM_CUR_FRE_NUM ,temp);
		Delay5Ms(10);

	}
}


/****************************************************************




*****************************************************************/
void fm_fre_test_add_sub(bool dir)
{
	u8 temp;

	if(dir)
	{
		if(fmFre_lev < 2)
			fmFre_lev ++;
	}
	else
	{
		if(fmFre_lev > 0)
			fmFre_lev --;
		//fmFre_lev = 0;
	}

	fmFrequency = fmFre[fmFre_lev];
	fm_rx_set_freq(fmFrequency);
	display_ui_fm(0);

	//temp=fmFrequency-FM_MIN;
	//at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);

}




/****************************************************************




*****************************************************************/
void fm_fre_add_sub(bool dir)
{
	u8 temp;
	if(dir)
	{
		fmFrequency ++;
		if(fmFrequency > FM_MAX)
		fmFrequency = FM_MIN;
	}
	else
	{
		fmFrequency --;
		if(fmFrequency <FM_MIN)
			fmFrequency = FM_MAX;
	}
	pa_mute_ctrl(true);
	//fm_rx_set_vol(0);
	rda5807_mute();

	radio_freq_valid();
	//fm_rx_set_freq(fmFrequency);
	display_ui_fm(0);

	//temp=fmFrequency-FM_MIN;
	//at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);
	//Delay5Ms(10);

	//fm_rx_set_vol(volume);
	rda5807_unmute();
	pa_mute_ctrl(false);

}

/****************************************************************




*****************************************************************/
void fm_ch_add_sub(bool dir)
{
	u8 temp;

	if(dir)
	{
		Cur_Fre_Num ++;
		if(Cur_Fre_Num > Fre_Total_Num)
		{
			if(Fre_Total_Num == 0)
			{
				Cur_Fre_Num = 0;
			}
			else
			{
				Cur_Fre_Num = 1;
			}
		}
			
	}
	else
	{
		if(Cur_Fre_Num > 0)
			Cur_Fre_Num --;
		if(Cur_Fre_Num ==0)
			Cur_Fre_Num = Fre_Total_Num;
	}

	//printf("Cur_Fre_Num=%d---Fre_Total_Num =%d\n",Cur_Fre_Num,Fre_Total_Num);
	fmFrequency = Frequency_Save[Cur_Fre_Num-1] + FM_MIN;
	fm_rx_set_freq(fmFrequency);
	
	display_ui_fm(1);

	//temp=fmFrequency-FM_MIN;
	//at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);
	//Delay5Ms(10);

	//temp=Cur_Fre_Num;
	//at24c02_write_one_byte(MEM_CUR_FRE_NUM ,temp);
	//Delay5Ms(10);
}



/****************************************************************




*****************************************************************/
void fm_auto_seek(bool dir)
{
	u8 temp;
	while (1)
	{
		if(dir)
		{
			fmFrequency ++;
			if(fmFrequency > FM_MAX)
				fmFrequency = FM_MIN;
		}

		else
		{
			fmFrequency --;
			if(fmFrequency <FM_MIN)
				fmFrequency = FM_MAX;
		}
		display_ui_fm(0);
		if (fm_rx_seek(fmFrequency))
			break;
	}

	fm_rx_set_freq(fmFrequency);

	temp=fmFrequency-FM_MIN;
	at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);
	Delay5Ms(10);

}








/*----------------------------------------------------------------------------*/
/**@brief FMÄ£Ê½ÏÂ±£´æµ±Ç°µÄÆµÂÊÖµ
   @param index ±£´æÎ»ÖÃµÄË÷ÒýÖµ
   @return ÎÞ
   @note
*/
/*----------------------------------------------------------------------------*/
 void SaveChan(u8 index)
{
    u8 temp;

    if(index > MAX_CH_NUM)
        return;
    temp = fmFrequency - FM_MIN;
    Frequency_Save[index -1] = temp;

    //at24c02_write_one_byte(MEM_SAVE_CHANNEL + index -1,temp);
    //Delay5Ms(10);
}

/*----------------------------------------------------------------------------*/
/**@brief FMÄ£¿é³õÊ¼»¯
   @param ÎÞ
   @return ÎÞ
   @note
*/
/*----------------------------------------------------------------------------*/
void fm_rx_init(void)
{
    switch(g_FMType_RX)
    {
#ifdef FM_RDA5807_EN
    case RDA5807P:
        rda5807p_init();
        break;
#endif

#ifdef FM_QN8035_EN
    case QN8035:
        QN8035_init();
        break;
#endif

#ifdef FM_BK1080_EN
    case BK1080:
        bk1080_init();
        break;
#endif

#ifdef FM_CL6017S_EN
    case CL6017S:
        cl6017s_init();
        break;
#endif

#ifdef FM_CL6017G_EN
    case CL6017G:
        CL6017G_init();
        break;
#endif

#ifdef FM_KT0830_EN
    case KT0830:
        kt0830_init();
        break;
#endif

#ifdef FM_AR1010_EN
    case AR1010:
        AR1010_init();
        break;
#endif

#ifdef FM_SP3777_EN
    case SP3777:
        sp3777_init();
        break;
#endif

#ifdef FM_AR1019_EN
    case AR1019:
        AR1019_init();
        break;
#endif

    }
}

/*----------------------------------------------------------------------------*/
/**@brief ÉèÖÃFMÄ£¿éÆµÂÊÖµ
   @param freq ½«ÒªÉèÖÃµÄÆµÂÊÖµ
   @return ÎÞ
   @note ÆµÂÊÖµ·¶Î§875~1080
*/
/*----------------------------------------------------------------------------*/
void fm_rx_set_freq(u16 freq)
{
    if ((freq>=FM_MIN) && (freq<=FM_MAX))
    {
        switch(g_FMType_RX)
        {
#ifdef FM_RDA5807_EN
        case RDA5807P:
            //rda5807p_set_freq(freq);
            rda5807_set_freq(freq*10);
            break;
#endif

#ifdef FM_QN8035_EN
        case QN8035:
            QND_TuneToCH(freq);
            break;
#endif

#ifdef FM_BK1080_EN
        case BK1080:
            bk1080_set_freq(freq);
            break;
#endif

#ifdef FM_CL6017S_EN
       case CL6017S:
            cl6017s_set_freq(freq);
            break;
#endif

#ifdef FM_CL6017G_EN
       case CL6017G:
            CL6017G_set_freq(freq);
            break;
#endif
#ifdef FM_KT0830_EN
        case KT0830:
            kt0830_set_freq(freq);
            break;
#endif

#ifdef FM_AR1010_EN
        case AR1010:
            AR1010_set_freq(freq);
            break;
#endif

#ifdef FM_SP3777_EN
        case SP3777:
            sp3777_set_freq(freq);
            break;
#endif

#ifdef FM_AR1019_EN
    case AR1019:
        AR1019_set_freq(freq);
        break;
#endif
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief ÉèÖÃFMÄ£¿éÒôÁ¿
   @param vol ½«ÒªÉèÖÃµÄÒôÁ¿Öµ
   @return ÎÞ
   @note ÒôÁ¿Öµ·¶Î§0~15
*/
/*----------------------------------------------------------------------------*/
void fm_rx_set_vol(u8 vol)
{
    switch(g_FMType_RX)
    {
#ifdef FM_RDA5807_EN
    case RDA5807P:
        rda5807p_set_vol(vol);
        break;
#endif

#ifdef FM_QN8035_EN
    case QN8035:
        qn8035_set_vol(vol);
        break;
#endif

#ifdef FM_BK1080_EN
     case BK1080:
        bk1080_set_vol(vol);
        break;
#endif

#ifdef FM_CL6017S_EN
    case CL6017S:
        cl6017s_set_vol(vol);
        break;
#endif

#ifdef FM_CL6017G_EN
    case CL6017G:
        CL6017G_set_vol(vol);
        break;
#endif
#ifdef FM_KT0830_EN
    case KT0830:
        kt0830_set_vol(vol);
        break;
#endif

#ifdef FM_AR1010_EN
    case AR1010:
        AR1010_set_vol(vol);
        break;
#endif

#ifdef FM_SP3777_EN
    case SP3777:
        sp3777_set_vol(vol);
        break;
#endif

#ifdef FM_AR1019_EN
    case AR1019:
        AR1019_set_vol(vol);
        break;
#endif
    }
}

/*----------------------------------------------------------------------------*/
/**@brief ¹Ø±ÕFMÄ£¿é
   @param ÎÞ
   @return ÎÞ
   @note
*/
/*----------------------------------------------------------------------------*/
void fm_rx_off(void)
{
    switch(g_FMType_RX)
    {
#ifdef FM_RDA5807_EN
    case RDA5807P:
        rda5807p_off();
        break;
#endif

#ifdef FM_QN8035_EN
    case QN8035:
        qn8035_off();
        break;
#endif

#ifdef FM_BK1080_EN
    case BK1080:
        bk1080_off();
        break;
#endif

#ifdef FM_CL6017S_EN
    case CL6017S:
        cl6017s_off();
        break;
#endif

#ifdef FM_CL6017G_EN
    case CL6017G:
        CL6017G_off();
        break;
#endif

#ifdef FM_KT0830_EN
    case KT0830:
        kt0830_off();
        break;
#endif

#ifdef FM_AR1010_EN
    case AR1010:
        AR1010_off();
        break;
#endif

#ifdef FM_SP3777_EN
    case SP3777:
        sp3777_off();
        break;
#endif

#ifdef FM_AR1019_EN
    case AR1019:
        AR1019_off();
        break;
#endif
    }

}

/*----------------------------------------------------------------------------*/
/**@brief FMÄ£¿é×Ô¶¯ËÑÌ¨
   @param freq ½«ÒªËÑË÷µÄÆµÂÊÖµ
   @return Ö¸¶¨µÄÆµÂÊÖµÏÂÓÐÌ¨·µ»Ø1£¬·ñÔò·µ»Ø0
   @note
*/
/*----------------------------------------------------------------------------*/
u8 fm_rx_seek(u16 freq)
{
    u8 result;

    switch(g_FMType_RX)
    {
#ifdef FM_RDA5807_EN
    case RDA5807P:
        //result = rda5807p_seek(freq);
        result = radio_freq_valid();
        break;
#endif

#ifdef FM_QN8035_EN
    case QN8035:
        result = qn8035_seek(freq);
        break;
#endif

#ifdef FM_BK1080_EN
    case BK1080:
        result = bk1080_seek(freq);
        break;
#endif

#ifdef FM_CL6017S_EN
    case CL6017S:
        result = cl6017s_seek(freq);
        break;
#endif

#ifdef FM_CL6017G_EN
    case CL6017G:
        result = CL6017G_seek(freq);
        break;
#endif
#ifdef FM_KT0830_EN
    case KT0830:
        result = kt0830_seek(freq);
        break;
#endif

#ifdef FM_AR1010_EN
    case AR1010:
        result = AR1010_seek(freq);
        break;
#endif

#ifdef FM_SP3777_EN
    case SP3777:
        result = sp3777_seek(freq);
        break;
#endif

#ifdef FM_AR1019_EN
    case AR1019:
        result = AR1019_seek(freq);
        break;
#endif
    }

    return result;
}

/*----------------------------------------------------------------------------*/
/**@brief ÅÐ¶ÏFMÄ£¿éÀàÐÍ
   @param ÎÞ
   @return ÎÞ
   @note
*/
/*----------------------------------------------------------------------------*/
u8 fm_rx_ID(void)
{
    u8 result;
    result = NO_FM;

#ifdef FM_RDA5807_EN
	if(rda5807_read_chipid())
    //if( rda5807p_online())
        result = RDA5807P;
#endif

#ifdef FM_CL6017S_EN
    if( cl6017s_online() )
        result = CL6017S;
#endif

#ifdef FM_CL6017G_EN
    if( cl6017G_online() )
        result = CL6017G;
#endif

#ifdef FM_AR1010_EN
    if( AR1010_online() )
        result = AR1010;
#endif

#ifdef FM_BK1080_EN
    if( bk1080_online() )
        result = BK1080;
#endif

#ifdef FM_KT0830_EN
    if( kt0830_online() )
        result = KT0830;
#endif

#ifdef FM_SP3777_EN
    if( sp3777_online() )
        result = SP3777;
#endif

#ifdef FM_QN8035_EN
    if( qn8035_online() )
        result = QN8035;
#endif

#ifdef FM_AR1019_EN
    if( AR1019_online() )
        result = AR1019;
#endif

    return result;
}


void fm_clear(void)
{
	u8 temp;
	
	fmFrequency = 875;
	Fre_Total_Num = 0;
	Cur_Fre_Num = 0;
	
	temp = fmFrequency-FM_MIN;
	at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);
	Delay5Ms(10);
	
	temp = Fre_Total_Num;
	at24c02_write_one_byte(MEM_FRE_TOTAL_NUM, temp);
	Delay5Ms(10);
	
	temp=Cur_Fre_Num;
	at24c02_write_one_byte(MEM_CUR_FRE_NUM ,temp);
	Delay5Ms(10);
	
	temp = fmFrequency - FM_MIN;
    Frequency_Save[0] = temp;
    at24c02_write_one_byte(MEM_SAVE_CHANNEL,temp);
	Delay5Ms(10);

	//Fre_Total_Num = at24c02_read_one_byte(MEM_FRE_TOTAL_NUM);
	//printf("Fre_Total_Num ===%d\r\n",Fre_Total_Num);
}

int Search_channel_in_save(void)
{
	int i;
	
	if((Fre_Total_Num == 0)&&(fmFrequency == FM_MIN))
	{
		return -1;
	}
	
	for(i = 0;i < Fre_Total_Num+1;i++)
	{
		//printf("Frequency_Save[%d] ===%d\r\n",i,Frequency_Save[i]);
		if(Frequency_Save[i] == (fmFrequency - FM_MIN))
		{
			return i+1;
		}			
	}

	return -1;
}

/****************************************************************************
 * Name: radio_on
 *
 * Description:
 *    æ”¶éŸ³æœºå¤ä½
 * 
 * Parameters:
 *
 * Returned Value:
 * 
 * Assumptions:
 *
 ****************************************************************************/
void radio_off(void)
{
    if (!rda5807_reset())
    {//æ”¶éŸ³æœºå¤ä½å¤±è´¥
        printf("FM Reset fail\n");
    }
}



/****************************************************************************
 * Name: radio_on
 *
 * Description:
 *    æ‰“å¼€æ”¶éŸ³æœº
 * 
 * Parameters:
 *
 * Returned Value:
 * 
 * Assumptions:
 *
 ****************************************************************************/
void radio_on(void)
{
	//radio_main();

	unsigned short ChipID;
	rda5807_init_iic();
	Delay5Ms(2);
    if (rda5807_reset())
    {
        //usleep(1000);
        Delay5Ms(10);
        if (rda5807_power_on())
        {
			Delay5Ms(10);
            if (rda5807_init())
            {
				//usleep(10000);
				Delay5Ms(125);
                if(rda5807_set_freq(fmFrequency*10))
                {
                    printf("%s:FM set freq success.\n",__func__);
                }
                else
                {
                    printf("%s:FM set freq fail.\n",__func__);
                }

            }
            else
            {
                printf("%s:FM int fail.\n",__func__);
            }
        }
        else
        {
            printf("%s:FM Power On fail.\n",__func__);
        }
    }
    else
    {
        printf("FM Reset fail\n");
    }

}


/****************************************************************************
 * Name: radio_freq_valid
 *
 * Description:
 *    æ ¡éªŒæ”¶éŸ³æœºå½“å‰æ˜¯å¦æœ‰æ•ˆé¢‘é“
 * 
 * Parameters:
 *
 * Returned Value:
 * 
 * Assumptions:
 *
 ****************************************************************************/
unsigned char   radio_freq_valid(void)
{
	int rssi = 0;
	int channel = 0;
	int temp = 0;
	int set_chan = 0;
	
	fm_rx_set_freq(fmFrequency);
	Delay5Ms(20);
	
    unsigned char buf[10];
    int i;
    for(i=0;i<2;++i)
    {
        if(rda5807_read(buf,10)) 
        {
			rssi = buf[2]>>1;
        	//printf("buf[0]:%x,buf[1]:%x,buf[2]:%x,buf[3]:%x rssi:%d\n",buf[0],buf[1],buf[2],buf[3],rssi);
            if ((buf[2] & 0x01) && (buf[0] & 0x40) && (rssi >= 7))
            {//æœ‰æ•ˆé¢‘çŽ‡
				temp = ((buf[0] & 0x03) << 8) | buf[1];
				
				#ifdef	_FM_STEP_50K_
				channel = (temp*50 + 87000)/10;
				#else
				channel = (temp*100 + 87000)/10;
				#endif
				printf("valid channel :%d temp :%d\n", channel, temp);  
				return 1;
            }
        }
		else 
		{
			printf("fm read failed \n");
		}
    }
	return 0;
}


