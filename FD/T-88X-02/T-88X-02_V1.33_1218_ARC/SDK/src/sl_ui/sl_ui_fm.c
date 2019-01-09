/*--------------------------------------------------------------------------*/
/**@file fm_rcv.c
   @brief FM模块驱动接口
   @details FM模块驱动操作函数
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
#include "QN8035.h"
#include "at24c02.h"
#include "sl_ui_display.h"
#include "sl_ui_fm.h"
#include "sl_ui_cmd_deal.h"



#define FM_QN8035_EN

//#define WriteToDev



#define FM_VOL_MAX           15



extern ui_cmd_t get_mq_msg(void);
extern unsigned char g_FMType_RX;
extern bool fm_test_flag;
extern bool fm_scan_start;



u8 Frequency_Save[MAX_CH_NUM];
u8 Fre_Total_Num;

u16 fmFrequency;
u8 Cur_Fre_Num;
u8 volume=15;

char fmFre_lev = 0;
u16 fmFre[3] = {900,960,1043};


/*----------------------------------------------------------------------------*/
/**@brief FM模式主函数
   @param 无
   @return 退出FM模式后将要进入的模式
   @note
*/
/*----------------------------------------------------------------------------*/

void Delay5Ms(u8 t_ms)
{
	usleep(5000*t_ms);
}


/*----------------------------------------------------------------------------*/
/**@brief FM模式主函数
   @param 无
   @return 退出FM模式后将要进入的模式
   @note
*/
/*----------------------------------------------------------------------------*/


unsigned char FM_Mode(void)
{

	u8 i;

#ifndef WriteToDev
	fmFrequency = at24c02_read_one_byte(MEM_FM_FREQUENCY);
	fmFrequency += FM_MIN;
	if((fmFrequency > FM_MAX) || (fmFrequency < FM_MIN))
		fmFrequency = FM_MIN;

	//printf("fmFrequency ===%d\r\n",fmFrequency);

	Fre_Total_Num = at24c02_read_one_byte(MEM_FRE_TOTAL_NUM);
	if(Fre_Total_Num > MAX_CH_NUM)
		Fre_Total_Num = 1;

	//printf("Fre_Total_Num ===%d\r\n",Fre_Total_Num);

	Cur_Fre_Num = at24c02_read_one_byte(MEM_CUR_FRE_NUM);
	if(Cur_Fre_Num > Fre_Total_Num)
		Cur_Fre_Num = 1;

	//printf("Cur_Fre_Num ===%d\r\n",Cur_Fre_Num);

	for(i = 0;i < Fre_Total_Num;i++)
	{
		Frequency_Save[i] = at24c02_read_one_byte(MEM_SAVE_CHANNEL + i);
		if(Frequency_Save[i] > (FM_MAX- FM_MIN))
		Frequency_Save[i] = 0;
		printf("Frequency_Save[%d]===%d\r\n",Frequency_Save[i]);
	}
#endif

	if(fm_test_flag == true)
	{
		fmFrequency = 900;
	}
	volume=15;
	fm_rx_init();
	Delay5Ms(15);   //为了兼容5807P，在此加上延时
	fm_rx_set_freq(fmFrequency);
#ifdef FM_CTL_VOL
	fm_rx_set_vol(volume);
#else
	fm_rx_set_vol(FM_VOL_MAX);
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief FM模式下的自动搜台
   @param 无
   @return 无
   @note
*/
/*----------------------------------------------------------------------------*/
 u8 FmScan(int mode,int dir)
{
	u8 temp;
	static u16 CurFrequency = 0;
	ui_cmd_t cmdq;
	temp = 1;

	if(mode == 1)
	{
		fmFrequency=875;    
	}
	
	CurFrequency = fmFrequency;    //保存起始频点
	Fre_Total_Num = 0;

	fm_rx_set_vol(0);
	pa_mute_ctrl(true);

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
			fmFrequency --;
			fm_rx_set_freq(fmFrequency);
			break;
		}
		display_ui_fm(0);

		if (fm_rx_seek(fmFrequency))
		{
			fm_rx_set_freq(fmFrequency);
			fm_rx_set_vol(volume);
			pa_mute_ctrl(false);

			SaveChan(temp);
			Fre_Total_Num = temp;

			Cur_Fre_Num = Fre_Total_Num;
			display_ui_fm(1);
			Delay5Ms(200);

			if(Fre_Total_Num >= MAX_CH_NUM)
			break;

			temp++;

			if(mode == 0)
			{
				fm_scan_start =      false;
				break;
			}
			else
			{
				fm_rx_set_vol(0);
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
		else if(mode == 1)
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
			
		if(fmFrequency == CurFrequency)     //一个循环
		{
			fm_rx_set_freq(fmFrequency);	
			break;
		}

	}

	if(Fre_Total_Num == 0)
	{
		Fre_Total_Num = 1;
		Cur_Fre_Num = 1;
		Frequency_Save[0] = 0;
	}
	temp = Fre_Total_Num;
	at24c02_write_one_byte(MEM_FRE_TOTAL_NUM, temp);
	Delay5Ms(10);


	if(Fre_Total_Num>=1)
	{
		Cur_Fre_Num = 1;
		fmFrequency= Frequency_Save[0]+FM_MIN;
		
		if(Fre_Total_Num>1)
			fm_rx_set_freq(fmFrequency);	
			
		temp=fmFrequency-FM_MIN;
		at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);
		Delay5Ms(10);
	}

	Delay5Ms(10);
	display_ui_fm(0);
	if(mode != 0)
	{
		fm_rx_set_vol(volume);
		pa_mute_ctrl(false);
	}
}

/**************************************************



**************************************************/
void fre_save(void)
{
   unsigned char temp;
   temp=fmFrequency-FM_MIN;
   at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);
   Delay5Ms(10);
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
	fm_rx_set_vol(0);

	fm_rx_set_freq(fmFrequency);
	display_ui_fm(0);

	temp=fmFrequency-FM_MIN;
	at24c02_write_one_byte(MEM_FM_FREQUENCY ,temp);
	Delay5Ms(10);

	fm_rx_set_vol(volume);
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
			Cur_Fre_Num = 1;
	}
	else
	{
		Cur_Fre_Num --;
		if(Cur_Fre_Num ==0)
			Cur_Fre_Num = Fre_Total_Num;
	}

	//printf("Cur_Fre_Num=%d---Fre_Total_Num =%d\n",Cur_Fre_Num,Fre_Total_Num);
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
/**@brief FM模式下保存当前的频率值
   @param index 保存位置的索引值
   @return 无
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

    at24c02_write_one_byte(MEM_SAVE_CHANNEL + index -1,temp);

    Delay5Ms(10);
}

/*----------------------------------------------------------------------------*/
/**@brief FM模块初始化
   @param 无
   @return 无
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
/**@brief 设置FM模块频率值
   @param freq 将要设置的频率值
   @return 无
   @note 频率值范围875~1080
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
            rda5807p_set_freq(freq);
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
/**@brief 设置FM模块音量
   @param vol 将要设置的音量值
   @return 无
   @note 音量值范围0~15
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
/**@brief 关闭FM模块
   @param 无
   @return 无
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
/**@brief FM模块自动搜台
   @param freq 将要搜索的频率值
   @return 指定的频率值下有台返回1，否则返回0
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
        result = rda5807p_seek(freq);
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
/**@brief 判断FM模块类型
   @param 无
   @return 无
   @note
*/
/*----------------------------------------------------------------------------*/
u8 fm_rx_ID(void)
{
    u8 result;
    result = NO_FM;

#ifdef FM_RDA5807_EN
    if( rda5807p_online())
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
	Fre_Total_Num = 1;
	Cur_Fre_Num = 1;
	
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
