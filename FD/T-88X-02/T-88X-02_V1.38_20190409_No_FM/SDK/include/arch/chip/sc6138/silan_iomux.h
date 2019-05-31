/*
 * silan_iomux.h
 */

#ifndef __SILAN_AP1508_IOMUX_H__
#define __SILAN_AP1508_IOMUX_H__

typedef enum {
	SILAN_IOMUX_UART1_1,
	SILAN_IOMUX_UART1_2,
	SILAN_IOMUX_UART2,
	SILAN_IOMUX_UART3_1,
	SILAN_IOMUX_UART3_2,
	SILAN_IOMUX_UART4_1,
	SILAN_IOMUX_UART4_2,
	SILAN_IOMUX_GMAC,
	SILAN_IOMUX_I2C1,
	SILAN_IOMUX_I2C2,
	SILAN_IOMUX_SDMMC,
	SILAN_IOMUX_SDIO_1,
	SILAN_IOMUX_SDIO_2,
	SILAN_IOMUX_SPI1,
	SILAN_IOMUX_PWM0,
	SILAN_IOMUX_PWM1_1,
	SILAN_IOMUX_PWM1_2,
	SILAN_IOMUX_PWM2_1,
	SILAN_IOMUX_PWM2_2,
	SILAN_IOMUX_PWM3_1,
	SILAN_IOMUX_PWM3_2,
	SILAN_IOMUX_PWM4,
	SILAN_IOMUX_PWM5,
	SILAN_IOMUX_PWM6,
	SILAN_IOMUX_PWM7,
	SILAN_IOMUX_JTAG,
}silan_iomux_mode_t;

int silan_iomux_config(silan_iomux_mode_t mod, int open);
#endif
