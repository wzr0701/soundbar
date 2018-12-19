#include <fcntl.h>
#include <silan_resources.h>
#include <silan_addrspace.h>
#include <stdio.h>
#include <string.h>
#include <zhuque_bsp_gpio.h>
#include "sl_bsp.h"
#include "led_api.h"
#include "led_driver.h"
#include "sl_ui_handle.h"
#include "sl_ui_cmd.h"


static unsigned short SegNum_Table[]={CHAR_0, CHAR_1, CHAR_2, CHAR_3 ,CHAR_4, CHAR_5, CHAR_6, CHAR_7,
CHAR_8, CHAR_9, CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E, CHAR_F, CHAR_G, CHAR_H, CHAR_I, CHAR_J,
CHAR_K, CHAR_L, CHAR_M, CHAR_N, CHAR_O, CHAR_P, CHAR_Q, CHAR_R, CHAR_S, CHAR_T, CHAR_U, CHAR_V, 
CHAR_W, CHAR_X, CHAR_Y, CHAR_Z};

void led_test(void)
{
//	int i;
//	while(1)
//	{
//		clear_seg_grid();
//		zhuque_bsp_gpio_set_mode(LED_PIN_SEG4, GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(LED_PIN_SEG4, 1);
//		clear_grid(0);
//		zhuque_bsp_gpio_set_mode(13, GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(13, 0);
//		zhuque_bsp_gpio_set_mode(14, GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(14, 0);
//		usleep(100000);

//		clear_seg_grid();
//		zhuque_bsp_gpio_set_mode(LED_PIN_SEG5, GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(LED_PIN_SEG5, 1);
//		clear_grid(0);
//		zhuque_bsp_gpio_set_mode(13, GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(13, 0);
//		zhuque_bsp_gpio_set_mode(14, GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(14, 0);
//		usleep(100000);
		
		
//		for (i = 0; i < sizeof(SegNum_Table)/sizeof(SegNum_Table[0]); i++)
//		{
//			dig_tab[0] = SegNum_Table[i]; 
//			dig_tab[1] = SegNum_Table[i];
//			dig_tab[2] = SegNum_Table[i];
//			dig_tab[3] = SegNum_Table[i];	
//			usleep(1000000);
//		}
//	}

//	const unsigned char seg_pin_tab[5] = {LED_PIN_SEG1, LED_PIN_SEG2, LED_PIN_SEG3, LED_PIN_SEG4, LED_PIN_SEG5};

//	const unsigned char grid_pin_tab[12] = {
//		LED_PIN_GRID11, 
//		LED_PIN_GRID9,
//		LED_PIN_GRID10, 
//		LED_PIN_GRID5,
//		LED_PIN_GRID12, 
//		LED_PIN_GRID1,
//		LED_PIN_GRID6, 
//		LED_PIN_GRID7,
//		LED_PIN_GRID8, 
//		LED_PIN_GRID2,
//		LED_PIN_GRID4,
//		LED_PIN_GRID3
//	};
//		
//	int i;
//	while(1)
//	{
//		//
//		clear_seg_grid();
//		zhuque_bsp_gpio_set_mode(seg_pin_tab[0], GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(seg_pin_tab[0], 1);
//		for(i = 0; i < 12; i++)
//		{
//			clear_grid(1);
//			zhuque_bsp_gpio_set_mode(grid_pin_tab[i], GPIO_OUT, PUSH_PULL); 
//			zhuque_bsp_gpio_set_value(grid_pin_tab[i], 0);
//			usleep(1000000);
//		}
//		//
//		clear_seg_grid();
//		zhuque_bsp_gpio_set_mode(seg_pin_tab[1], GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(seg_pin_tab[1], 1);
//		for(i = 0; i < 12; i++)
//		{
//			clear_grid(1);
//			zhuque_bsp_gpio_set_mode(grid_pin_tab[i], GPIO_OUT, PUSH_PULL); 
//			zhuque_bsp_gpio_set_value(grid_pin_tab[i], 0);
//			usleep(1000000);
//		}
//		//
//		clear_seg_grid();
//		zhuque_bsp_gpio_set_mode(seg_pin_tab[2], GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(seg_pin_tab[2], 1);
//		for(i = 0; i < 12; i++)
//		{
//			clear_grid(1);
//			zhuque_bsp_gpio_set_mode(grid_pin_tab[i], GPIO_OUT, PUSH_PULL); 
//			zhuque_bsp_gpio_set_value(grid_pin_tab[i], 0);
//			usleep(1000000);
//		}
//		//
//		clear_seg_grid();
//		zhuque_bsp_gpio_set_mode(seg_pin_tab[3], GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(seg_pin_tab[3], 1);
//		for(i = 0; i < 12; i++)
//		{
//			clear_grid(1);
//			zhuque_bsp_gpio_set_mode(grid_pin_tab[i], GPIO_OUT, PUSH_PULL); 
//			zhuque_bsp_gpio_set_value(grid_pin_tab[i], 0);
//			usleep(1000000);
//		}
//		//
//		clear_seg_grid();
//		zhuque_bsp_gpio_set_mode(seg_pin_tab[4], GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(seg_pin_tab[4], 1);
//		for(i = 0; i < 12; i++)
//		{
//			clear_grid(1);
//			zhuque_bsp_gpio_set_mode(grid_pin_tab[i], GPIO_OUT, PUSH_PULL); 
//			zhuque_bsp_gpio_set_value(grid_pin_tab[i], 0);
//			usleep(1000000);
//		}
//	}

}

void ui_led_clear(void)
{
	dig_tab[0] = 0; 
	dig_tab[1] = 0;
	dig_tab[2] = 0;
	dig_tab[3] = 0;
	dig_tab[4] = 0;
}

void led_clear_buffer(void)
{
	dig_tab[0] = 0; 
	dig_tab[1] = 0;
	dig_tab[2] = 0;
	dig_tab[3] = 0;
	dig_tab[4] = 0;
	clear_seg_grid();
}

void ui_led_update_display(void)
{


}

void ui_led_disp_mode(unsigned char mode)
{
	switch (mode)
	{
		case SOURCE_SELECT_LINEIN:
			dig_tab[0] = 0; 
			dig_tab[1] = CHAR_A;
			dig_tab[2] = CHAR_U;
			dig_tab[3] = CHAR_X;
			break;
		case SOURCE_SELECT_LINEIN1:
			dig_tab[0] = 0; 
			dig_tab[1] = CHAR_R;
			dig_tab[2] = CHAR_C;
			dig_tab[3] = CHAR_A;
			break;
		case SOURCE_SELECT_SPDIFIN:
			dig_tab[0] = 0; 
			dig_tab[1] = CHAR_O;
			dig_tab[2] = CHAR_P;
			dig_tab[3] = CHAR_T;
			break;
		case SOURCE_SELECT_HDMI:
			dig_tab[0] = CHAR_H; 
			dig_tab[1] = CHAR_D;
			dig_tab[2] = CHAR_M;
			dig_tab[3] = CHAR_I;
			break;
		case SOURCE_SELECT_BT:
			dig_tab[0] = 0; 
			dig_tab[1] = CHAR_B;
			dig_tab[2] = CHAR_T;
			dig_tab[3] = 0;
			break;		
		default:
			break;
	}
	dig_tab[4] = 0;
	
//	printf("\n888888888888test download %s", __func__);
//	printf("\n888888888888test download %s %d %d ", __func__, dig_tab[1], dig_tab[2]);
}

void ui_led_disp_state(unsigned char sta)
{
	switch(sta)
	{
		case 0:

			break;
		case 1:
			dig_tab[0] = CHAR_M; 
			dig_tab[1] = CHAR_I;
			dig_tab[2] = CHAR_C;
			dig_tab[3] = CHAR_N;
			break;
		case 2:
			dig_tab[0] = CHAR_M; 
			dig_tab[1] = CHAR_U;
			dig_tab[2] = CHAR_S;
			dig_tab[3] = CHAR_I;
			break;
		case 3:
			dig_tab[0] = CHAR_M; 
			dig_tab[1] = CHAR_O;
			dig_tab[2] = CHAR_V;
			dig_tab[3] = CHAR_I;
			break;
		case 4:
			dig_tab[0] = CHAR_D; 
			dig_tab[1] = CHAR_I;
			dig_tab[2] = CHAR_A;
			dig_tab[3] = CHAR_L;
			break;

	}
	dig_tab[4] = 0;
}

void ui_led_disp_filenum(int num)
{

}

void ui_led_disp_music_time(int time)
{

}

void ui_led_disp_repeat(unsigned char sta)
{

}


void ui_led_disp_trebass(unsigned char sta, int value)
{
	switch(sta)
	{
		case SET_DISP_TREBLE:
			dig_tab[0] = CHAR_T; 
			dig_tab[1] = CHAR_UNDERLINE;
			if (value >= 0)
				dig_tab[2] = 0;
			else 
				dig_tab[2] = CHAR_SPEC;
			dig_tab[3] = SegNum_Table[abs(value)];
			break;
		case SET_DISP_BASS:
			dig_tab[0] = CHAR_B; 
			dig_tab[1] = CHAR_UNDERLINE;
			if (value >= 0)
				dig_tab[2] = 0;
			else 
				dig_tab[2] = CHAR_SPEC;
			dig_tab[3] = SegNum_Table[abs(value)];
			break;		
	}
	dig_tab[4] = 0;
}

void ui_led_disp_volume(int vol)
{
	dig_tab[0] = CHAR_V;
	dig_tab[1] = CHAR_UNDERLINE; 
	dig_tab[2] = (vol > 0) ? SegNum_Table[vol/10] : 0;
	dig_tab[3] = SegNum_Table[vol%10];
	dig_tab[4] = 0;
}


void ui_led_disp_eq(unsigned char eq)
{

}

void ui_led_disp_mic(unsigned char eq, unsigned char vol)
{

}

void ui_led_disp_col1(unsigned char flag)
{
	if (flag)
	{
		dig_tab[4] |= 0x01;
	}
	else 
	{
		dig_tab[4] &= ~0x01;
	}
}

void ui_led_disp_col2(unsigned char flag)
{
	if (flag)
	{
		dig_tab[4] |= 0x02;
	}
	else 
	{
		dig_tab[4] &= ~0x02;
	}
}

void ui_led_disp_seek(void)
{

}

void ui_led_disp_channel(int       num)
{


}




