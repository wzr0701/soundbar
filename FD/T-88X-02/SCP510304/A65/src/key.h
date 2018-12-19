#ifndef __KEY_H__
#define __KEY_H__
#include "mytype.h"

sbit POWER_KEY = P1^3;

#define KEY_NUM       1
#define KEY_LEVEL     1 //°´¼ü×´Ì¬
typedef struct
{
	UINT16  cnt;
	UINT8 state;      
	//UINT8 ShortPressDownEvent;
	UINT8 ShortPressUpEvent;
	UINT8 LongPressDownEvent;
	//UINT8 LongPressUpEvent;
}KeyTerminal;

typedef struct
{
	KeyTerminal  key[KEY_NUM];
}KeyForm;
typedef struct
{
	UINT8 f1ms    :1;
	UINT8 f10ms   :1;
	UINT8 f20ms   :1;
	UINT8 f50ms   :1;
	UINT8 f100ms  :1;
	UINT8 f1000ms :1;
	UINT16 cnt;
}SysRun;
extern SysRun run;
void key_init(void);
void KeyAnalysis(UINT8 key);
void KeyPressCnt(void);
void KeyScan(void);
void KeyHandler(void);
void RunScan(void);
void RunHandler(void);


#endif
