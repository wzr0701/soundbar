#include "led_driver.h"
#include <fcntl.h>
#include <silan_resources.h>
#include <silan_addrspace.h>
#include <stdio.h>
#include <string.h>
#include <zhuque_bsp_gpio.h>
#include "sl_bsp.h"
#include <nuttx/input.h>



#define LEDSCREEN_TIMER 		4
#define LEDSCREEN_SCAN_CYCLE 	2500//us

const unsigned char grid_pin_tab[12] = {
	LED_PIN_GRID11, 
	LED_PIN_GRID9,
	LED_PIN_GRID10, 
	LED_PIN_GRID5,
	LED_PIN_GRID12, 
	LED_PIN_GRID1,
	LED_PIN_GRID6, 
	LED_PIN_GRID7,
	LED_PIN_GRID8, 
	LED_PIN_GRID2,
	LED_PIN_GRID4,
	LED_PIN_GRID3
};
const unsigned char seg_pin_tab[5] = {LED_PIN_SEG1, LED_PIN_SEG2, LED_PIN_SEG3, LED_PIN_SEG4, LED_PIN_SEG5};


unsigned short dig_tab[5]={0}; //数码管显示1-5位 最后两个字节为两个冒号

void clear_seg(int val)
{
	int j;
	for (j = 0; j < 5; j++)
	{
		zhuque_bsp_gpio_set_mode(seg_pin_tab[j], GPIO_OUT, PUSH_PULL); 
		zhuque_bsp_gpio_set_value(seg_pin_tab[j], val);
	}
}

void clear_grid(int val)
{
	int j;
	for (j = 0; j < 12; j++)
	{
		zhuque_bsp_gpio_set_mode(grid_pin_tab[j], GPIO_OUT, PUSH_PULL); 
		zhuque_bsp_gpio_set_value(grid_pin_tab[j], val);
	}
}

void clear_seg_grid(void)
{
	int j;
	for (j = 0; j < 5; j++)
	{
		zhuque_bsp_gpio_set_mode(seg_pin_tab[j], GPIO_OUT, PUSH_PULL); 
		zhuque_bsp_gpio_set_value(seg_pin_tab[j], 0);
	}
	for (j = 0; j < 12; j++)
	{
		zhuque_bsp_gpio_set_mode(grid_pin_tab[j], GPIO_OUT, PUSH_PULL); 
		zhuque_bsp_gpio_set_value(grid_pin_tab[j], 1);
	}
}


int ir_repeat_code_times = 0;  //收到重复码次数
int ir_repeat_flag = 0; 	   	//有重复码
int ir_repeat_wdtimer_start = 0; //开启检测

int ir_repeat_retry_times = 0; //重试检测次数

/*
void repeat_det_handle(void)
{
	if (ir_repeat_wdtimer_start == 0) return;

	if (ir_repeat_flag)
	{
		ir_repeat_flag = 0;
		ir_repeat_code_times++;
		ir_repeat_retry_times = 0;
	}
	else 
	{
		if (++ir_repeat_retry_times >= 2) 
		{
			struct input_event key_event;
			if (ir_repeat_code_times < 8) //短按
			{
				
				key_event.type = EV_IR;
				key_event.code = CODE_BT_MODE;
				key_event.value = 0;
			}
			else	//长按
			{
				key_event.type = EV_IR;
				key_event.code = CODE_BT_PAIR;
				key_event.value = 0; 
			}
			input_add_event(&key_event);
			ir_repeat_wdtimer_start = 0;
			ir_repeat_retry_times = 0;
			ir_repeat_code_times = 0;
			ir_repeat_flag = 0;
		}
	}
}
*/

int scan_state = 0;
int test_flag = 0;
int ir_rep_cnt = 0;
void led_scan_handle(void)
{
	
//	if (test_flag) 
//	{
//		zhuque_bsp_gpio_set_mode(10, GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(10, 1); 
//		test_flag = 0;
//	}
//	else
//	{
//		zhuque_bsp_gpio_set_mode(10, GPIO_OUT, PUSH_PULL); 
//		zhuque_bsp_gpio_set_value(10, 0); 
//		test_flag = 1;
//	}
	
	clear_seg_grid();
	int i;
	switch(scan_state)
	{
		case 0:
			zhuque_bsp_gpio_set_mode(seg_pin_tab[0], GPIO_OUT, PUSH_PULL); 
			zhuque_bsp_gpio_set_value(seg_pin_tab[0], 1);
			for(i = 0; i < 12; i++)
			{
				if (dig_tab[0] & (1 << i))
				{
					zhuque_bsp_gpio_set_mode(grid_pin_tab[i], GPIO_OUT, PUSH_PULL); 
					zhuque_bsp_gpio_set_value(grid_pin_tab[i], 0);
				}		
			}
			break;

		case 1:
			zhuque_bsp_gpio_set_mode(seg_pin_tab[1], GPIO_OUT, PUSH_PULL); 
			zhuque_bsp_gpio_set_value(seg_pin_tab[1], 1);
			for(i = 0; i < 12; i++)
			{
				if (dig_tab[1] & (1 << i))
				{
					zhuque_bsp_gpio_set_mode(grid_pin_tab[i], GPIO_OUT, PUSH_PULL); 
					zhuque_bsp_gpio_set_value(grid_pin_tab[i], 0);
				}		
			}
			break;

		case 2:
			zhuque_bsp_gpio_set_mode(seg_pin_tab[2], GPIO_OUT, PUSH_PULL); 
			zhuque_bsp_gpio_set_value(seg_pin_tab[2], 1);
			for(i = 0; i < 12; i++)
			{
				if (dig_tab[2] & (1 << i))
				{
					zhuque_bsp_gpio_set_mode(grid_pin_tab[i], GPIO_OUT, PUSH_PULL); 
					zhuque_bsp_gpio_set_value(grid_pin_tab[i], 0);
				}		
			}
			break;

		case 3:
			zhuque_bsp_gpio_set_mode(seg_pin_tab[3], GPIO_OUT, PUSH_PULL); 
			zhuque_bsp_gpio_set_value(seg_pin_tab[3], 1);
			for(i = 0; i < 12; i++)
			{
				if (dig_tab[3] & (1 << i))
				{
					zhuque_bsp_gpio_set_mode(grid_pin_tab[i], GPIO_OUT, PUSH_PULL); 
					zhuque_bsp_gpio_set_value(grid_pin_tab[i], 0);
				}		
			}
			break;
		
		case 4:
			zhuque_bsp_gpio_set_mode(seg_pin_tab[4], GPIO_OUT, PUSH_PULL); 
			zhuque_bsp_gpio_set_value(seg_pin_tab[4], 1);
			for (i = 0; i < 4; i++)
			{
				if (dig_tab[i] & seg_k) 
				{
					zhuque_bsp_gpio_set_mode(grid_pin_tab[i], GPIO_OUT, PUSH_PULL); 
					zhuque_bsp_gpio_set_value(grid_pin_tab[i], 0);
				}
				if (dig_tab[i] & seg_l) 
				{
					zhuque_bsp_gpio_set_mode(grid_pin_tab[4 + i], GPIO_OUT, PUSH_PULL); 
					zhuque_bsp_gpio_set_value(grid_pin_tab[4 + i], 0);
				}
			}
			if (dig_tab[4] & 0x01)
			{
				zhuque_bsp_gpio_set_mode(grid_pin_tab[8], GPIO_OUT, PUSH_PULL); 
				zhuque_bsp_gpio_set_value(grid_pin_tab[8], 0);
			}
			if (dig_tab[4] & 0x02)
			{
				zhuque_bsp_gpio_set_mode(grid_pin_tab[9], GPIO_OUT, PUSH_PULL); 
				zhuque_bsp_gpio_set_value(grid_pin_tab[9], 0);
			}
			break;
	}
	
	if (++scan_state >= 5)
	{
		scan_state = 0;
	}

	set_pad_spdif();
	/*
	if (++ir_rep_cnt >= 44)
	{
		ir_rep_cnt = 0;
		repeat_det_handle();
	}*/
}

void led_timer_start(void)
{
    //停止计时
    silan_timer_stop(LEDSCREEN_TIMER);
    //启动LED屏扫描计时器
    silan_timer_initialize(LEDSCREEN_TIMER, LEDSCREEN_SCAN_CYCLE, led_scan_handle);
    //启动LED计时
    silan_timer_start(LEDSCREEN_TIMER);
}

void led_timer_stop(void)
{
    //停止LED刷新TIMER
    silan_timer_stop(4);
}


void led_initial(void)
{
	//led pin io initial
	bsp_led_io_init();
	led_timer_start();
}










