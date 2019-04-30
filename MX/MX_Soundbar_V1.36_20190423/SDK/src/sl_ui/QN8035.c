/*--------------------------------------------------------------------------*/
/**@file QN8035.C
   @brief FMÄ£¿éQN8035Çý¶¯
   @details FMÄ£¿éQN8035µÄÇý¶¯º¯Êý
   @author LZZ
   @date 2010-08-17
   @note Í¨¹ýºê¶¨ÒåÀ´Ñ¡ÔñÇý¶¯·½Ê½
*/
/*----------------------------------------------------------------------------*/

#include "iic.h"
#include <typedef.h>
#include <silan_resources.h>
#include <silan_addrspace.h>
#include <stdio.h>
#include <string.h>
#include "QN8035.h"

#define DEVICE_QN8035_ADDR   0x20

#define FM_I2C_CLK_PIN 32

#define FM_I2C_DATA_PIN 27



//×¢Òâ:ÔÚQN8035.hÎÄ¼þÖÐ¶ÔÓÐÍâ²¿Ê±ÖÓ32.768KMzºÍ24MHzµÄÇø±ð!

#define USE_P05_AS_FM_CLOCK          1
#define USE_CRYSTAL_AS_FM_CLOCK      2
#define USE_MODULE                   3 //QN Ã»ÓÐÄ£¿é

/*
  ´ËÏîºê¶¨ÒåËµÃ÷ÊÇÊ¹ÓÃºÎÖÖFMÇý¶¯·½Ê½
  USE_P05_AS_FM_CLOCK£¬Ö¸Ê¹ÓÃP05×÷ÎªÊ±ÖÓÐÅºÅ
  USE_CRYSTAL_AS_FM_CLOCK£¬Ê¹ÓÃ32K»ò24M×÷ÎªÇý¶¯ÐÅºÅ
  USE_MODULE£¬Ê¹ÓÃÄ£¿é
*/
#define FM_CLOCK USE_CRYSTAL_AS_FM_CLOCK

extern void Delay5Ms(unsigned char  t_ms);


u16 FREQ2CHREG(u16 freq)
{
    return 	(freq-6000)/5;
}

void QNF_SetMute(UINT8 On)
{
    QND_WriteReg(REG_DAC, On?0x1B:0x10);
}

/**********************************************************************
UINT8 QNF_SetCh(UINT16 freq)
**********************************************************************
Description: set channel frequency

Parameters:
    freq:  channel frequency to be set
Return Value:
    1: success
**********************************************************************/
#if USING_VALID_CH
void QNF_SetCh(UINT16 freq)
{
    UINT8 temp;

	 freq = FREQ2CHREG(freq);
	//writing lower 8 bits of CCA channel start index
	QND_WriteReg(CH_START, (UINT8)freq);
	//writing lower 8 bits of CCA channel stop index
	QND_WriteReg(CH_STOP, (UINT8)freq);
	//writing lower 8 bits of channel index
	QND_WriteReg(CH, (UINT8)freq);
	//writing higher bits of CCA channel start,stop and step index
	temp = (UINT8) ((freq >> 8) & CH_CH);
	temp |= ((UINT8)(freq >> 6) & CH_CH_START);
	temp |= ((UINT8) (freq >> 4) & CH_CH_STOP);
	temp |= QND_STEP_CONSTANT;//(step << 6);
	QND_WriteReg(CH_STEP, temp);
}
#else
void QNF_SetCh(UINT16 start,UINT16 stop)
{
    UINT8 temp;

	 start = FREQ2CHREG(start);
	 stop = FREQ2CHREG(stop);
	//writing lower 8 bits of CCA channel start index
	QND_WriteReg(CH_START, (UINT8)start);
	//writing lower 8 bits of CCA channel stop index
	QND_WriteReg(CH_STOP, (UINT8)stop);
	//writing lower 8 bits of channel index
	QND_WriteReg(CH, (UINT8)start);
	//writing higher bits of CCA channel start,stop and step index
	temp = (UINT8) ((start >> 8) & CH_CH);
	temp |= ((UINT8)(start >> 6) & CH_CH_START);
	temp |= ((UINT8) (stop >> 4) & CH_CH_STOP);
	temp |= QND_STEP_CONSTANT;//(step << 6);
	QND_WriteReg(CH_STEP, temp);
}
#endif

/**********************************************************************
void QND_Init()
**********************************************************************
Description: Initialize device to make it ready to have all functionality ready for use.

Parameters:
    None
Return Value:
    1: Device is ready to use.
    0: Device is not ready to serve function.
**********************************************************************/
void QN8035_init(void)
{

    iic_init(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    QND_WriteReg(0x00, 0x81);
    Delay5Ms(5);

    //QN8035Ã»ÓÐÄ£¿é

    // change crystal frequency setting here
#if (FM_CLOCK == USE_CRYSTAL_AS_FM_CLOCK)
    QND_WriteReg(0x01,QND_SINE_WAVE_CLOCK);//   ÕýÏÒ²¨ÓÃÕâ¸ö

#elif (FM_CLOCK == USE_P05_AS_FM_CLOCK)
    QND_WriteReg(0x01,QND_DIGITAL_CLOCK);//   ·½²¨ÓÃÕâ¸ö
#endif

    //QND_WriteReg(XTAL_DIV0, QND_XTAL_DIV0);
    //QND_WriteReg(XTAL_DIV1, QND_XTAL_DIV1);
    //QND_WriteReg(XTAL_DIV2, QND_XTAL_DIV2);
    QND_WriteReg(0x54, 0x47);//mod PLL setting
    //select SNR as filter3,SM step is 2db
    QND_WriteReg(0x19, 0xc4);
    QND_WriteReg(0x40,0x70);	//set SNR as SM,SNC,HCC MPX   /////Ã»ÓÐ
    QND_WriteReg(0x33, 0x9c);//9e;//set HCC and SM Hystersis 5db
    QND_WriteReg(0x2d, 0xD6);//notch filter threshold adjusting
    QND_WriteReg(0x43, 0x10);//notch filter threshold enable
    QND_WriteReg(0x47,0x39);
    //enter receiver mode directly
    QND_WriteReg(0x00, 0x11);
    //Enable the channel condition filter3 adaptation,Let ccfilter3 adjust freely
    QND_WriteReg(0x1D,0xA9);
    QND_WriteReg(0x4f, 0x40);//dsiable auto tuning
    QND_WriteReg(0x34,SMSTART_VAL); ///set SMSTART
    QND_WriteReg(0x35,SNCSTART_VAL); ///set SNCSTART
    QND_WriteReg(0x36,HCCSTART_VAL); ///set HCCSTART
    QNF_SetMute(1);
}

/*----------------------------------------------------------------------------*/
/**@brief FMÄ£¿éQN8035ÉèÖÃÒôÁ¿
   @param volume ÒôÁ¿Öµ
   @return ÎÞ
   @note ÒôÁ¿·¶Î§0~15
*/
/*----------------------------------------------------------------------------*/
void qn8035_set_vol(u8 volume)
{

  volume = volume*3;
  if(volume>47)
  {
  	volume = 47;//qn8035 volume range is from 0~47
  }
  QND_RXConfigAudio(volume);
}

/*----------------------------------------------------------------------------*/
/**@brief FMÄ£¿éQN8035¹Ø±Õ
   @param ÎÞ
   @return ÎÞ
   @note
*/
/*----------------------------------------------------------------------------*/
void qn8035_off(void)
{
    QND_WriteReg(0x00, 0x21);
    Delay5Ms(10);
}


/**********************************************************************
void QND_TuneToCH(UINT16 ch)
**********************************************************************
Description: Tune to the specific channel. call QND_SetSysMode() before
call this function
Parameters:
ch
Set the frequency (10kHz) to be tuned,
eg: 101.30MHz will be set to 10130.
Return Value:
None
**********************************************************************/
void QND_TuneToCH(UINT16 ch)
{
    UINT8 reg;

    ch = ch * 10;
    //increase reference PLL charge pump current.
    QND_WriteReg(REG_REF,0x7A);

    /********** QNF_RXInit ****************/
    QND_WriteReg(0x1B,0x70);  //Let NFILT adjust freely
    //QNF_SetRegBit(0x2C,0x3F,0x12);  ///When SNR<ccth31, ccfilt3 will work
    //setting the threshold of Filter3 will be worked.
    QND_WriteReg(0x2C,0x52);
    //QNF_SetRegBit(0x1D,0x40,0x00);  ///Let ccfilter3 adjust freely
    //QNF_SetRegBit(0x41,0x0F,0x0A);  ///Set a hcc index to trig ccfilter3's adjust
    QND_WriteReg(0x45,0x50);        ///Set aud_thrd will affect ccfilter3's tap number
    //QNF_SetRegBit(0x40,0x70,0x70);  ///snc/hcc/sm snr_rssi_sel; snc_start=0x40; hcc_start=0x30; sm_start=0x20
    QND_WriteReg(0x40,0x70);
    //QNF_SetRegBit(0x19,0x80,0x80);  ///Use SNR for ccfilter selection criterion
    //selecting SNR as filter3 filter condition
    //QND_WriteReg(0x19, 0xC2);
    //QNF_SetRegBit(0x3E,0x80,0x80);  ///it is decided by programming this register
    //QNF_SetRegBit(0x41,0xE0,0xC0);  ///DC notching High pass filter bandwidth; remove low freqency dc signals
    QND_WriteReg(0x41,0xCA);
    QND_WriteReg(0x34,SMSTART_VAL); ///set SMSTART
    QND_WriteReg(0x35,SNCSTART_VAL); ///set SNCSTART
    QND_WriteReg(0x36,HCCSTART_VAL); ///set HCCSTART
    /********** End of QNF_RXInit ****************/

    QNF_SetMute(1);
#if USING_VALID_CH
    QNF_SetCh(ch);
#else
    QNF_SetCh(ch,ch);
#endif
    //enable CCA mode with user write into frequency
    QND_WriteReg(0x00, 0x13);
#if 1
    //Auto tuning
    QND_WriteReg(0x4F, 0x80);
    reg = QND_ReadReg(0x4F);
    reg >>= 1;
    QND_WriteReg(0x4F, reg);
#endif
    ///avoid the "POP" noise.
    Delay5Ms(CH_SETUP_DELAY_TIME/5);  //300ms
    ///decrease reference PLL charge pump current.
    QND_WriteReg(REG_REF,0x70);
    QNF_SetMute(0);
}

/***********************************************************************
void QND_RXSetTH(UINT8 th)
***********************************************************************
Description: Setting the threshold value of automatic scan channel
th:
  Setting threshold for quality of channel to be searched,
  the range of th value:CCA_SENSITIVITY_LEVEL_0 ~ CCA_SENSITIVITY_LEVEL_9
Return Value:
  None
***********************************************************************/
void QND_RXSetTH(UINT8 th)
{
    ///increase reference PLL charge pump current.
    QND_WriteReg(REG_REF,0x7A);
    //NFILT program is enabled
    QND_WriteReg(0x1B,0x78);
    //using Filter3
    QND_WriteReg(CCA1,0x75);
    //setting CCA IF counter error range value(256).
    QND_WriteReg(CCA_CNT2,0x03);//
#if PILOT_CCA
    QND_WriteReg(PLT1,0x00);
#endif
	//selection the time of CCA FSM wait SNR calculator to settle:20ms
	//0x00:	    20ms(default)
	//0x40:	    40ms
	//0x80:	    60ms
	//0xC0:	    100m
	//    QNF_SetRegBit(CCA_SNR_TH_1 , 0xC0, 0x00);
    //selection the time of CCA FSM wait RF front end and AGC to settle:20ms
    //0x00:     10ms
	//0x40:     20ms(default)
    //0x80:     40ms
	//0xC0:     60ms
	//    QNF_SetRegBit(CCA_SNR_TH_2, 0xC0, 0x40);
	//    QNF_SetRegBit(CCA, 30);
       //setting CCA RSSI threshold is 30
	QND_WriteReg(CCA,QND_ReadReg(CCA)&0xC0|20);  // 20 ~ 30
#if PILOT_CCA
	QND_WriteReg(CCA_SNR_TH_1,8+th); //setting SNR threshold for CCA
#else
	QND_WriteReg(CCA_SNR_TH_1,9+th); //setting SNR threshold for CCA	 8 ~ 10
#endif
}

/***********************************************************************
UINT16 QND_RXValidCH(UINT16 freq, UINT8 step);
***********************************************************************
Description: to validate a ch (frequency)(if it's a valid channel)
Freq: specific channel frequency, unit: 10Khz
  eg: 108.00MHz will be set to 10800.
Step:
  FM:
  QND_FMSTEP_100KHZ: set leap step to 100kHz
  QND_FMSTEP_200KHZ: set leap step to 200kHz
  QND_FMSTEP_50KHZ:  set leap step to 50kHz
Return Value:
  0: not a valid channel
  other: a valid channel at this frequency
***********************************************************************/
UINT8 QND_RXValidCH(UINT16 freq)
{
 	UINT8 regValue;
  	UINT8 timeOut;
  	UINT8 isValidChannelFlag;

#if PILOT_CCA
  	UINT8 snr,readCnt,stereoCount=0;
#endif
#if USING_VALID_CH
  	QNF_SetCh(freq);
#else
  	QNF_SetCh(freq,freq);
#endif
  	//QNF_SetRegBit(SYSTEM1,0x03,0x02);//enter channel scan mode,channel frequency is decided by internal CCA
  	//entering into RX mode and CCA mode,channels index decide by CCA.
  	QND_WriteReg(0x00, 0x12);
  	timeOut = 20;
  	do
  	{
    	regValue = QND_ReadReg(SYSTEM1);
    	Delay5Ms(1);   //delay 5ms
    	timeOut--;
  	}
  	//if it seeks a potential channel or time out,the loop will be quited
  	while((regValue & CHSC) && timeOut);
  	//TRACE("CHSC:%d,timeOut:%d \n",regValue&CHSC,timeOut);
  	//reading out the rxcca_fail flag of RXCCA status
  	isValidChannelFlag = (QND_ReadReg(STATUS1) & RXCCA_FAIL ? 0:1) && timeOut;
  	if(isValidChannelFlag)
  	{
#if PILOT_CCA
    	Delay5Ms(4);
    	snr = QND_ReadReg(SNR);
    	if(snr> 25) return 1;
    	for(readCnt=10;readCnt>0;readCnt--)
    	{
      		Delay5Ms(1);
      		stereoCount += ((QND_ReadReg(STATUS1) & ST_MO_RX) ? 0:1);
      		if(stereoCount >= 3) return 1;
    	}
#else
    	return 1;
#endif
  	}
  	return 0;
}

/************************************************************************
void QND_RXConfigAudio(UINT8 optiontype, UINT8 option )
*************************************************************************
Description: config audio
Parameters:
  optiontype: option
    QND_CONFIG_MONO; ¡®option¡¯control mono, 0: stereo receive mode ,1: mono receiver mode
    QND_CONFIG_MUTE; ¡®option¡¯control mute, 0:mute disable,1:mute enable
    QND_CONFIG_VOLUME: 'option' control the volume gain,range : 0~47(-47db~0db)

Return Value:
    none
**********************************************************************/
void QND_RXConfigAudio(UINT8 option )
{
    UINT8 regVal;

    regVal = QND_ReadReg(VOL_CTL);
	regVal = (regVal&0xC0)|(option/6)|(5-option%6<<3);
	QND_WriteReg(VOL_CTL,regVal);
}





 bool qn8035_send_device_addr(bool w_r)
{
    iic_send_byte(DEVICE_QN8035_ADDR|(w_r?1:0), FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    return iic_recv_ack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
}

 bool qn8035_send_sub_addr(unsigned char addr)
{
    iic_send_byte(addr, FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    return iic_recv_ack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
}




////////////////////////////////////////////
UINT8 QND_ReadReg(UINT8 addr)
{
    unsigned char read_tmp = 0;
    {
        //å‘é€å¼€å§‹ä¿¡å?
        iic_start(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
        //å‘é€å¸¦å†™ä¿¡å·çš„è®¾å¤‡ID
        if(qn8035_send_device_addr(false))
        {//å‘é€å¸¦å†™ä¿¡å·çš„è®¾å¤‡IDæˆåŠŸ
            //å‘é€å¯„å­˜å™¨åœ°å€
			if(qn8035_send_sub_addr(addr))
            {//å‘é€å¯„å­˜å™¨åœ°å€æˆåŠŸ
                //å‘é€å¼€å§‹ä¿¡å?
                iic_start(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
                //å‘é€å¸¦è¯»ä¿¡å·çš„è®¾å¤‡ID
                if(qn8035_send_device_addr(true))
                {//å‘é€å¸¦è¯»ä¿¡å·çš„è®¾å¤‡IDæˆåŠŸ
                    read_tmp = iic_read_byte(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
                    iic_send_nack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
                }
            }
        }
        //åœæ­¢ä¹‹å‰çš„æ“ä½?
        iic_stop(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    }
    return read_tmp;
}

void  QND_WriteReg(UINT8 addr, UINT8 value)
{

    //å‘é€å¼€å§‹ä¿¡å?
    iic_start(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
    //å‘é€å¸¦å†™ä¿¡å·çš„è®¾å¤‡ID
    if(qn8035_send_device_addr(false))
    {//å‘é€å¸¦å†™ä¿¡å·çš„è®¾å¤‡IDæˆåŠŸ
        //å‘é€å¯„å­˜å™¨åœ°å€
        if(qn8035_send_sub_addr(addr))
        {//å‘é€å¯„å­˜å™¨åœ°å€æˆåŠŸ
            //å‘é€æ•°æ?
            iic_send_byte(value, FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
             iic_recv_ack(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);
        }
    }
    //åœæ­¢ä¹‹å‰çš„æ“ä½?
    iic_stop(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);

}


u8 qn8035_seek(u16 freq)
{
    QNF_SetMute(1);
    QND_RXSetTH(0);
    return QND_RXValidCH(freq*10);//qn8035 step frequency unit is 10KHZ
}


void qn8035_mute(void)
{
  // QNF_SetRegBit(0x14, 0x80, 0x80);  //mute
    QNF_SetMute(1);
}

void qn8035_unmute(void)
{
    Delay5Ms(10);
    //QNF_SetRegBit(0x14, 0x80, 0x00); //unmute
    QNF_SetMute(0);
}


/*----------------------------------------------------------------------------*/
/**@brief FMÄ£¿éQN8035¼ì²â
   @param ÎÞ
   @return ¼ì²âµ½QN8035Ä£¿é·µ»Ø1£¬·ñÔò·µ»Ø0
   @note
*/
/*----------------------------------------------------------------------------*/
u8 qn8035_online(void)
{
    u8 temp;
    u8 i;
    iic_init(FM_I2C_CLK_PIN, FM_I2C_DATA_PIN);

    for(i = 0; i < 3; i++)
    {
		temp =QND_ReadReg(0x06) & 0xFC;
		printf("qn8035_online=%x\r\n",temp);
		if(temp == 0x84)
		{
			return 1;
		}
    }

    return 0;
}
