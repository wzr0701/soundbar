#ifndef SL_BSP_H
#define SL_BSP_H


#define HW_MUTE_PIN			  45//35

#define HW_BT_MUTE_PIN		12
#define HW_BT_POWER_PIN		37

#define LED_R_PIN	    	22
#define LED_G_PIN		    23
#define LED_B_PIN			46

#define HDMI_DET            15

#if 0
#define LED_LINE1_PIN		21
#define LED_LINE2_PIN		34
#define LED_OPT_PIN			7
#define LED_HDMI_PIN		33
#define LED_BT_PIN			8

#endif
#define LED_PIN_SEG1			26
#define LED_PIN_SEG2			27
#define LED_PIN_SEG3			32
#define LED_PIN_SEG4			33
#define LED_PIN_SEG5			34

#define LED_PIN_GRID1			21
#define LED_PIN_GRID2			23
#define LED_PIN_GRID3			37
#define LED_PIN_GRID4			38
#define LED_PIN_GRID5			42
#define LED_PIN_GRID6			46
#define LED_PIN_GRID7			13
#define LED_PIN_GRID8			14
#define LED_PIN_GRID9			7
#define LED_PIN_GRID10			8
#define LED_PIN_GRID11			25
#define LED_PIN_GRID12			45

void bsp_init(void);
void Hw_Amp_Mute(void);
void Hw_Amp_UnMute(void);
void show_led_mode(int mode);
void show_led_off(void);
void Bt_Power_On(void);
void Bt_Power_Off(void);
void Bt_Mute(void);
void Bt_UnMute(void);
void close_led_mode(int mode);
void mute_led_on(void);
int HDMI_Detect(void);
void dsp_init_check(void);




#endif
