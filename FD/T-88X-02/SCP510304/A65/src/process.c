#include "mytype.h"
#include "process.h"


S_IR g_Ir;
void Time1_Close(void)
{
    if (TF1)
    {
        TF1 = 0;
        TR1 = 0;
        g_Ir.Buff = 0;
        g_Ir.LastCount = 0;
    }
}
