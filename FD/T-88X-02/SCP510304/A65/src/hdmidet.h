#ifndef _HDMIDET_H_
#define _HDMIDET_H_

#include "mytype.h"

sbit HDMI_CEC_DET_PIN = P1^0;

extern UINT16 hdmi_cec_det_delay;


void CEC_init(void);
void HDMI_CEC_Det(void);
void DelayEnableHdmiDet(void);


#endif
