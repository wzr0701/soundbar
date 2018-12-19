#ifndef __POWERUP_H__
#define __POWERUP_H__
#include "mytype.h"



sbit P_LED_POWER = P1^0;
sbit P_3V3_POWER = P1^1;
sbit P_1V2_POWER = P0^5;
//sbit P_14v_POWER = P1^0;
sbit P_RESET = P3^0;


#define PowerLED(x)  if(x) \
												P_LED_POWER = 1; \
											else \
												P_LED_POWER = 0;

#define Power3_3V(x)  if(x) \
												P_3V3_POWER = 1; \
											else \
												P_3V3_POWER = 0;

#define Power1_2V(x)  if(x) \
												P_1V2_POWER = 1; \
											else \
												P_1V2_POWER = 0;
	/*
#define Power14V(x)  if(x) \
												P_14v_POWER = 1; \
											else \
												P_14v_POWER = 0;
	*/
#define Reset(x)  		if(x) \
												P_RESET = 1; \
											else \
												P_RESET = 0;
#define  DELAY_TIME 10
#define  DELAY_OFF_TIME 200
extern UINT8 ENABLE_POWER_ON;
extern UINT8 ENABLE_POWER_OFF;
extern UINT8 POWER_ON_OFF;
extern UINT8 step;
extern UINT16 cnt;
extern UINT16 DelayOFF;
extern UINT16 DelayON;
void IrPower(void);
void ProcessPowerUp(void);
void ProcessPowerDown(void);
void DelayEnablePowerOFF(void);
void AutoPowerUp(void);
void HDMIPower(void);
#endif
