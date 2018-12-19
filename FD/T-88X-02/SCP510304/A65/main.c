#include "mytype.h"
#include "process.h"
#include "key.h"
#include "powerup.h"
#include "hdmidet.h"

extern void CLR_WDT();
void main(void)
{
 	System_Init();
	while (1)
	{
		Time1_Close();
		RunScan();
		RunHandler();
		HDMI_CEC_Det();
		//CLR_WDT(); //Çå³ý¿´ÃÅ¹·
	}
}