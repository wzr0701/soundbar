#ifndef SL_UI_FM_H__
#define SL_UI_FM_H__



#define FM_MIN    875
#define FM_MAX   1080

#define MAX_CH_NUM        40



enum
{
    NO_FM,
    RDA5807P,
    QN8035,
    BK1080,
    CL6017S,
    CL6017G,
    KT0830,
    SP3777,
    AR1010,
    AR1019
};

enum
{
    OSC_32K,
    OSC_12M
};





void fm_rx_init(void);

unsigned char fm_rx_seek(unsigned int freq);

void fm_rx_set_freq(unsigned int  freq);

void fm_rx_set_vol(unsigned char vol);

void fm_rx_off(void);

unsigned char fm_rx_ID(void);
 unsigned char FmScan(int mode,int dir);
 
 void SaveChan(unsigned char index);

unsigned char FM_Mode(void);
void fm_auto_seek(bool dir);
void fm_ch_add_sub(bool dir);
void fm_fre_test_add_sub(bool dir);
void fm_fre_add_sub(bool dir);


void fre_num_play(unsigned char c_num);
void fre_save(void);
void fm_clear(void);

void fre_manual_save(void);


#endif



