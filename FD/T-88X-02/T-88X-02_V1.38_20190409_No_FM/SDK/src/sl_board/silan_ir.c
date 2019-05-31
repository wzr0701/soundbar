/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <semaphore.h>
#include <errno.h>
#include <debug.h>
#include <nuttx/io.h>
#include <arch/board/board.h>
#include <nuttx/arch.h>
#include <nuttx/module.h>
#include <nuttx/input.h>
#include <silan_resources.h>
#include "zhuque_bsp_gpio.h"
#include "sl_ui_io_led.h"


 /****************************************************************************
  * Pre-processor Definitions
  ****************************************************************************/
//#define IR_DEBUG

#define TIMER_COUNTER 0
#define TIMER_COMPARE 4
#define TIMER_CONTROL 8

#define LEADER_MIN 1330 * 10 //us
#define LEADER_MAX 1370 * 10
#define DATA_1_MIN 205 * 10
#define DATA_1_MAX 245 * 10
#define DATA_0_MIN 92 * 10
#define DATA_0_MAX 132 * 10
#define REPEAT_MIN 1100 * 10
#define REPEAT_MAX 1150 * 10

#define cpuclk 10
#define CLK_LEADER_MIN (cpuclk * 1300) /* Leader minimum */
#define CLK_LEADER_MAX (cpuclk * 1390) /* Leader maximum */
#define CLK_DATA_1_MIN (cpuclk * 185)  /* Data 1 minimum  */
#define CLK_DATA_1_MAX (cpuclk * 275)  /* Data 1 maximum */
#define CLK_DATA_0_MIN (cpuclk * 72)   /* Data 0 minimum */
#define CLK_DATA_0_MAX (cpuclk * 152)  /* Data 0 maximum  */
#define CLK_REP_MIN (cpuclk * 1000)    /* REP  0 minimum */
#define CLK_REP_MAX (cpuclk * 1150)    /* REP  0 maximum  */
#define CLK_REPEAT_MIN (cpuclk * 1075) /* REPEAT  11.25 minimum */
#define CLK_REPEAT_MAX (cpuclk * 1175) /* REPEAT  11.25 maximum  */
#define CLK_REPEAT (cpuclk * 10800)    /* REPEAT  108 maximum  */

#define IR_BIT_NUM 32

#define cust_code 0x1fe

#define CODENUM (sizeof(ir_key_table) / sizeof(ir_key_table[0]))

#define TIMER_BASE (SILAN_TIMER1_BASE + 0x10 * (CONFIG_SILAN_IR_TIMER - 1))

 //????á?D???μ????￠ê?3??μ?ê
#define REPEATE_TIMES 2
#define REPEATE_TIMES_CNT 5

struct input_event save_ir_event;
bool ir_short_flag=false;
bool ir_long_flag=false;

int save_ir_cnt = 100;

extern bool next_folder_flag;
extern bool prev_folder_flag;
extern bool fm_scan_flag;
extern bool ir_reset_flag;
extern bool ir_playpause_flag;
extern bool fm_manual_save_flag;

 /****************************************************************************
  * Public Types
  ****************************************************************************/

 /****************************************************************************
  * Public Data
  ****************************************************************************/
 static const struct
 {
	 unsigned char ircode;
	 int keycode;
 } ir_key_table[] =
#if 0
{
	{0x50, CODE_IR_POWER },
	{0x30, CODE_IR_MUTE },
	{0xc0, CODE_IR_VOLUE_DOWN	},
	{0x40, CODE_IR_VOLUE_UP},
	{0xe0, CODE_IR_NEXT		      },
	{0xa0, CODE_IR_PREV		      },
	{0x00, CODE_IR_FM_TUNE_ADD },
	{0x80,CODE_IR_FM_TUNE_SUB	 },
	//{0x00, CODE_IR_NEXT_FOLD },
	//{0x80,CODE_IR_PREV_FOLD	 },
	{0x32,CODE_IR_PLAY_PAUSE},

	{0x38, CODE_IR_SOURCE	 },
	{0xb8,CODE_IR_GO_BT	 },
	///////////////////////////////////
	{0x48,CODE_IR_NUM_0	 },
	{0x28, CODE_IR_NUM_1      },
	{0xa8,CODE_IR_NUM_2	 },
	{0x68,CODE_IR_NUM_3      },
	{0xe8,CODE_IR_NUM_4	 },
	{0x18, CODE_IR_NUM_5       },
	{0x98, CODE_IR_NUM_6	 },
	{0x58, CODE_IR_NUM_7      },
	{0x08, CODE_IR_NUM_8	 },
	{0xd8, CODE_IR_NUM_9      },
	{0xf8,CODE_IR_ENTER      },

	////////////////////////////////////////////

	{0x60, CODE_IR_FM_SCAN  	 },
	{0xd0, CODE_IR_FM_CHAN_ADD    },
	{0x70, CODE_IR_FM_CHAN_SUB    },

	{0xf0, CODE_IR_ECHO_ADD    },
	{0x02, CODE_IR_ECHO_SUB    },

	{0x10, CODE_IR_TRE_ADD 		       },
	{0x90, CODE_IR_TRE_SUB                       },
	{ 0xc8,CODE_IR_BASS_ADD 		       },
	{0x88, CODE_IR_BASS_SUB                    },
	{ 0xb2,CODE_IR_RESET		       },
	{0x78,CODE_IR_MEM                       },

 };
#else
 {
	 {0x50, CODE_IR_POWER },
	 {0x30, CODE_IR_MUTE },
	 {0xc0, CODE_IR_VOLUE_DOWN	 },
	 {0x40, CODE_IR_VOLUE_UP},
	 {0x78, CODE_IR_NEXT		   },
	 {0x38, CODE_IR_PREV		   },
	 {0x00, CODE_IR_FM_TUNE_ADD },
	 {0x80,CODE_IR_FM_TUNE_SUB	  },
	 //{0x00, CODE_IR_NEXT_FOLD },
	 //{0x80,CODE_IR_PREV_FOLD	  },
	 {0x60,CODE_IR_PLAY_PAUSE},
	 {0xa0, CODE_IR_SOURCE	  },
	 //{0xb8,CODE_IR_GO_BT  },
	 ///////////////////////////////////
	 {0x48,CODE_IR_NUM_0  },
	 {0x28, CODE_IR_NUM_1	   },
	 {0xa8,CODE_IR_NUM_2  },
	 {0x68,CODE_IR_NUM_3	  },
	 {0xe8,CODE_IR_NUM_4  },
	 {0x18, CODE_IR_NUM_5		},
	 {0x98, CODE_IR_NUM_6	  },
	 {0x58, CODE_IR_NUM_7	   },
	 {0x08, CODE_IR_NUM_8	  },
	 {0xd8, CODE_IR_NUM_9	   },
	 {0xf8,CODE_IR_ENTER	  },

	 ////////////////////////////////////////////

	 {0xe0, CODE_IR_FM_SCAN 	  },
	 //{0xd0, CODE_IR_FM_CHAN_ADD    },
	 //{0x70, CODE_IR_FM_CHAN_SUB    },

	 {0xf0, CODE_IR_ECHO_ADD	},
	 {0x02, CODE_IR_ECHO_SUB	},

	 //{0x10, CODE_IR_TRE_ADD 			},
	 //{0x90, CODE_IR_TRE_SUB 			},
	 //{ 0xc8,CODE_IR_BASS_ADD			},
	 //{0x88, CODE_IR_BASS_SUB			},
	 //{ 0xb2,CODE_IR_RESET				},
	 {0xb8,CODE_IR_MIC  				},//MIC ON/OFF
	 {0xd0, CODE_IR_MIC_ADD	},
	 {0x70, CODE_IR_MIC_SUB	},

	 {0x10,CODE_IR_TRE					},
	 {0x90,CODE_IR_BASS					},

	 {0x32,CODE_IR_MOVIE				},

  };

#endif

 static int ir_irq_num = 38;
 static int ir_code_cnt = 0;
 static int ir_code_tbl[IR_BIT_NUM] = {0};
 static int ir_last_key = 0;

 static int repeat_timer = REPEATE_TIMES;
 static int repeat_timer_cnt = REPEATE_TIMES_CNT;
 /****************************************************************************
  * Public Function Prototypes
  ****************************************************************************/
 static int read_timer(void);
 static void ir_produce(void);

 extern unsigned int get_silan_busclk(void);
 /****************************************************************************
  * Public Functions
  ****************************************************************************/

 /****************************************************************************
  * Name: ir_produce
  *
  * Description:
  *    oìía?óê?3é1|′|àí3ìDò
  *
  * Parameters:
  *
  * Returned Value:
  *
  * Assumptions:
  *
  ****************************************************************************/
 static void ir_produce(void)
 {
	 unsigned int data = 0, i;
	 unsigned char data1, check, key;
	 struct input_event key_event;

	 for (i = 0; i < IR_BIT_NUM; i++)
	 {
		 data = ((data << 1) | ir_code_tbl[i]);
	 }
#ifdef IR_DEBUG
	 printf("ir_produce data %x\n", data);
#endif
	 data1 = data & 0xff;
	 check = ~((data >> 8) & 0xff);
#ifdef IR_DEBUG
	 printf("ir_produce data1 %x check %x\n", data1, check);
	 printf("data H :%x\n", ((data >> 16) & 0xffff));
#endif
	 if ((data1 == check) && (((data >> 16) & 0xffff) == cust_code))
	 {
		 key = ~(data & 0xff);
//#ifdef IR_DEBUG
		 //printf("keycode : %x\n", key);
//#endif

		 //ir_last_key = key;
		 for (i = 0; i < CODENUM; i++)
		 {
			 if (key == ir_key_table[i].ircode)
			 {

#ifdef IR_DEBUG
				 printf("keycode : %x\n", key);
#endif
				 save_ir_event.type = EV_IR;
				 save_ir_event.code = CODE_IR_PRESS;//
				 save_ir_event.value = ir_key_table[i].keycode; //key_priv.key_num;

				 ir_short_flag = true;
				 ir_long_flag = false;
				 
				 next_folder_flag = false;
				 prev_folder_flag = false;
				 fm_scan_flag = true;
				 ir_reset_flag = false;
				 ir_playpause_flag = false;
				 fm_manual_save_flag = false;

#ifdef IR_DEBUG
				 printf("sc6138 adkey push 0x%x\n", key_event.value);
#endif
				 //input_add_event(&save_ir_event);

				 //update last IR key value
				 ir_last_key = ir_key_table[i].keycode;

				save_ir_cnt = 0;
			 }
		 }
	 }

	 ir_code_cnt = 0;
	 memset(ir_code_tbl, 0, IR_BIT_NUM);
	 silan_timer_stop(CONFIG_SILAN_IR_TIMER);
	 silan_timer_initialize(CONFIG_SILAN_IR_TIMER, 0xffffffff, NULL);

	 return;
 }

 /****************************************************************************
  * Name: ir_produce
  *
  * Description:
  *    oìía?D??·t??3ìDò
  *
  * Parameters:
  *
  * Returned Value:
  *    ×üê?0
  *
  * Assumptions:
  *
  ****************************************************************************/
 static int ir_isr(int irq)
 {
	 unsigned int width, usec;
	 static unsigned int prev_usec = 0;
	 struct input_event key_event;
	 //  struct input_event key_event;

	 int value;

	 dma_interrupt_service();

	 zhuque_bsp_gpio_get_value(ir_irq_num, (uint32_t *)&value);
	 //  printf("ir_isr value:%d\r\n",value);

	 if (value > 0)
	 {
		 return 0;
	 }

	 if (ir_code_cnt >= IR_BIT_NUM)
	 {
		 return 0;
	 }

	 silan_timer_start(CONFIG_SILAN_IR_TIMER);
	 usec = read_timer();
	 if (prev_usec > usec)
	 {
		 prev_usec = 0;
	 }

	 width = usec - prev_usec;

#ifdef IR_DEBUG
	 printf("width:%ld\r\n", width);
#endif
	 prev_usec = usec;
	 if ((width >= CLK_DATA_1_MIN) && (width <= CLK_DATA_1_MAX))
	 {
		 ir_code_tbl[ir_code_cnt++] = 1;
	 }
	 else if ((width >= CLK_DATA_0_MIN) && (width <= CLK_DATA_0_MAX))
	 {
		 ir_code_tbl[ir_code_cnt++] = 0;
	 }
	 else if ((width >= CLK_LEADER_MIN) && (width <= CLK_LEADER_MAX))
	 {
		 ir_code_cnt = 0;
		 // ir_last_key = 0;
		 memset(ir_code_tbl, 0, IR_BIT_NUM);
		 // reset key repeat counter
		 repeat_timer = REPEATE_TIMES;
	 }
	 else if ((width >= CLK_REPEAT_MIN) && (width <= CLK_REPEAT_MAX))
	 {
#ifdef IR_DEBUG
		 printf("xxxxxxxx11.5ms\n");
#endif
		 save_ir_cnt = 0;
		 if (--repeat_timer == 0)
		 {	
			 save_ir_cnt = 0;
			 //send key event for long press
			 //if ((CODE_VOLUME_INC == ir_last_key) ||( CODE_VOLUME_DEC == ir_last_key) )
			 //if (ir_last_key > 0)
			 if ((CODE_IR_VOLUE_UP == ir_last_key) ||( CODE_IR_VOLUE_DOWN == ir_last_key)  ||
			 	(CODE_IR_ECHO_ADD == ir_last_key) ||( CODE_IR_ECHO_SUB == ir_last_key)  ||
			 	(CODE_IR_MIC_ADD == ir_last_key) ||( CODE_IR_MIC_SUB == ir_last_key) )
			 {
			 		ir_short_flag = false;
					ir_long_flag = true;
					
					key_event.type = EV_IR;
					key_event.code = CODE_IR_LONG_PRESS;
					key_event.value = ir_last_key;
					input_add_event(&key_event);
					//printf("ir_code_repeat %d\n", key_event.code);
			 }
			 else
			 {
				if (--repeat_timer_cnt == 0)
				{
					ir_short_flag = false;
					ir_long_flag = true;
					
					key_event.type = EV_IR;
					key_event.code = CODE_IR_LONG_PRESS;
					key_event.value = ir_last_key;
					input_add_event(&key_event);
					//printf("repeat_timer_cnt = %d\n", repeat_timer_cnt);

					repeat_timer_cnt = REPEATE_TIMES_CNT;
				}
			 }

			 // reset key repeat counter
			 repeat_timer = REPEATE_TIMES;
		 }
	 }
 //#ifdef IR_DEBUG
 ///  printf("ir_code_cnt %d\n", ir_code_cnt);
 //#endif
	 if (ir_code_cnt >= IR_BIT_NUM)
	 {
#ifdef IR_DEBUG
		 printf("enter ir_produce !\n\n");
#endif
		 ir_produce();
	 }

	 return 0;
 }

 /****************************************************************************
  * Name: read_timer
  *
  * Description:
  *    ?áè?ê±???μ
  *
  * Parameters:
  *
  * Returned Value:
  *    ?￠??êy
  *
  * Assumptions:
  *
  ****************************************************************************/
 static int read_timer(void)
 {
	 int val, cnt, usec;

	 val = readl(TIMER_BASE + TIMER_COUNTER);
	 cnt = get_silan_busclk() / 1000000;
	 //printf("busclk=%d\n", get_silan_cpuclk());
	 usec = val / cnt;

	 return usec;
 }

 /****************************************************************************
  * Name: sc6138_ir_pin_set
  *
  * Description:
  *    éè??oìía?óê?ê1ó?μ?òy??
  *
  * Parameters:
  *
  * Returned Value:
  *    ?￠??êy
  *
  * Assumptions:
  *
  ****************************************************************************/
 int sc6138_ir_pin_set(int pin)
 {
	 ir_irq_num = pin;
	 return 0;
 }

 /****************************************************************************
  * Name: sc6138_irinitialize
  *
  * Description:
  *    oìía?óê??ì2a3?ê??ˉ
  *
  * Parameters:
  *
  * Returned Value:
  *    ?￠??êy
  *
  * Assumptions:
  *
  ****************************************************************************/
 static int sc6138_irinitialize(void)
 {
	 //printf(">>>>>>>>>>>>>>>sc6138_irinitialize !\n ");
	 zhuque_bsp_gpio_set_mode(ir_irq_num, GPIO_IN, PULLING_NONE);
	 zhuque_bsp_gpio_register_interrupt(ir_irq_num, FALLING_EDGE, ir_isr);
	 silan_timer_initialize(CONFIG_SILAN_IR_TIMER, 0xffffffff, NULL);
	 //silan_timer_start(CONFIG_SILAN_IR_TIMER);
	 return 0;
 }
 //éè???a?ú?′DDoìía?ì2é3?ê??ˉ3ìDò
 zhuque_device_init(sc6138_irinitialize);

