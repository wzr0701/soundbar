#ifndef SL_UI_TOUCH_KEY_H__
#define SL_UI_TOUCH_KEY_H__

#include <zhuque_bsp_gpio.h>

#define KEY_INT_PIN                  16




bool touch_key_send_device_addr(bool w_r);

bool touch_key_read(unsigned char * buf, int length);

bool touch_key_send_sub_addr(unsigned char addr);

void touch_key_int(void);



#endif




