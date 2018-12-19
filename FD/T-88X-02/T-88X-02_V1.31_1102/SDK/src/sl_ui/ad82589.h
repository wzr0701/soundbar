#ifndef __AD82589__H__
#define __AD82589__H__
/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdbool.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
 
/****************************************************************************
 * Public Types
 ****************************************************************************/
 
/****************************************************************************
 * Public Data
 ****************************************************************************/
 
/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/
 
/****************************************************************************
 * Public Functions
 ****************************************************************************/
 
/****************************************************************************
 * Name: ad82589_init
 *
 * Description:
 *    ad82589初始化
 *
 * Parameters:
 *
 * Returned Value:
 *
 * Assumptions:
 *
 ****************************************************************************/
void ad82589_init(void);

/****************************************************************************
 * Name: ad82589_write_byte
 *
 * Description:
 *    通过IIC将特定值写到AD82589的特定寄存器
 *
 * Parameters:
 *    addr 寄存器地址
 *    byte 要写入的值
 *
 * Returned Value:
 *    true 成功，false失败
 *
 * Assumptions:
 *
 ****************************************************************************/
bool ad82589_write_byte(unsigned char addr, unsigned char byte);
#endif