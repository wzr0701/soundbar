/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "bt_uart.h"
#include <fcntl.h>
#include <silan_resources.h>
#include <silan_addrspace.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <zhuque_bsp_gpio.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
#define BT_DEBUG    1
#define UART_TX_PIN 13
#define UART_RX_PIN 14

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/
/*与蓝牙模块通信的字符串*/
const char BT_auto_connect[] = "AT+CC\r\n";
const char BT_dis_connect[] = "AT+CD\r\n";
const char BT_next[] = "AT+PD\r\n";
const char BT_prv[] = "AT+PE\r\n";
const char BT_paire[] = "AT+CA\r\n";
const char BT_pause[] = "AT+PA\r\n";
const char BT_play[] = "AT+PA\r\n";

const char BT_dis_paire[] = "AT+CB\r\n";
const char BT_mode[] = "AT+BT\n";
const char AUX_mode[] = "AT+AUX\n";
const char GET_aux[] = "AT+GET+AUX\n";
const char PWR_up[] = "AT+PWRUP\n";
const char PWR_off[] = "AT+PWROFF\n";


//const char BT_mode[] = "COM+MBT\n";
//const char AUX_mode[] = "COM+MAX\n";
const char BT_power[] = "AT+CP\r\n";
const char BT_stop[] = "AT+MC\r\n";
const char BT_vol_down[] = "AT+CL\r\n";
const char BT_vol_up[] = "AT+CK\r\n";
const char COM_iq[] = "AT+MO\r\n";
const char COM_sd[] = "COM+MSD\n";
const char FM_get_fq[] = "FM+GF\n";
const char FM_mode[] = "COM+MFM\n";
const char FM_search[] = "FM+SC\n";
const char FM_stop[] = "FM+ST\n";
const char AT_d8836[] = "AT+D8836\n";

/*与蓝牙模块通信串口文件描述符*/
int bt_fd = 0;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bt_read
 *
 * Description:
 *    通过串口读取蓝牙模块反馈的信息
 *
 * Parameters:
 *    buf_receive 接收数据用的buf
 *
 * Returned Value:
 *    -1 获取数据失败， 0 获取数据成功
 *
 * Assumptions:
 *
 ****************************************************************************/
int bt_read(char *buf_receive)
{
    int ret = -1;

    if (bt_fd < 0)
    {   //无效串口文件描述符
        #if 1//BT_DEBUG
        printf("%s:UART for bluetooth is not open\n", __func__);
        #endif
    }
    else
    {
        fd_set fds;
        struct timeval tm;
        FD_ZERO(&fds);
        FD_SET(bt_fd, &fds);
        tm.tv_sec = 0;
        tm.tv_usec = 2500;

        if (select(bt_fd + 1, &fds, NULL, NULL, &tm) < 0)
        {
            #if 1 //BT_DEBUG
            printf("%s:select err or timeout\n", __func__);
            #endif
        }
        else
        {
            if (FD_ISSET(bt_fd, &fds) > 0)
            {   ///有数据可以从串口读取
                char buf[16];
                int count;

                memset(buf, 0, 16);
                memset(buf_receive, 0, 16);
                count = read(bt_fd, buf, 16);
                if (count < 0)
                {
                    #if 1 //BT_DEBUG
                    printf("%s:Read nothing from UART\n", __func__);
                    #endif
                }
                else
                {   //从串口读到了数据
                    strcpy(buf_receive, buf);
                    if (buf_receive[count - 1] == '\n')
                    {   //读到的数据以回车结束
                        buf_receive[count] = '\0';

                        ret = count;
                    }
                    else
                    {   //读到的数据不是以回车结束
                        char *p = buf_receive + count;
                        int number;
                        bool read_end = false;
                        while (!read_end)
                        {
                            *p = '\0';

                            memset(buf, 0, 16);

                            number = read(bt_fd, buf, 16 - count);

                            if (number < 0)
                            {   //没能从串口读取到数据或已读出的数据超过缓冲区
                                //清除从串口循环读取数据标志
                                read_end = true;
                                ret = count;
                            }
                            else
                            {
                                count += number;
                                strcat(buf_receive, buf);

                                if(count >= 16)
                                {
                                    read_end = true;
                                    ret = count;
                                }
                                else
                                {
                                    p += number;

                                    if (buf[number - 1] == '\n')
                                    {   //读到的数据以回车结束
                                        //清除从串口循环读取数据标志
                                        read_end = true;

                                        buf_receive[count] = '\0';

                                        ret = count;
                                    }
                                }
                            }
                        }
                    }

                    printf("%s:buf_rev %s count:%d\r\n", __func__, buf_receive, count);
                }
            }
            else
            {
                #if 1 //BT_DEBUG
                //printf("%s:Nothing can be read from UART\n", __func__);
                #endif
            }
        }
    }

    return ret;
}

/****************************************************************************
 * Name: bt_uart_init
 *
 * Description:
 *    初始化与蓝牙通信的串口，波特率9600
 *
 * Parameters:
 *
 * Returned Value:
 *    true 初始化成功， false 初始化失败
 *
 * Assumptions:
 *
 ****************************************************************************/
bool bt_uart_init(void)
{
    bool ret = false;
    unsigned long value;
    //设置UART3的IO口状态
    zhuque_bsp_gpio_set_mode(UART_TX_PIN, GPIO_OUT, PULLING_HIGH);
    zhuque_bsp_gpio_set_value(UART_TX_PIN, GPIO_VALUE_LOW);
    zhuque_bsp_gpio_set_mode(UART_RX_PIN, GPIO_IN, PULLING_HIGH);
    zhuque_bsp_gpio_set_value(UART_RX_PIN, GPIO_VALUE_LOW);
    //设置UART3的对应的IO口功能
    value = REG32(KSEG1(SILAN_PADMUX_CTRL));
    value |= (1 << SILAN_PADMUX_UART3_1);
    REG32(KSEG1(SILAN_PADMUX_CTRL)) = value;
    //打开UART3设备
    //bt_fd = open("/dev/ttyS1", O_RDWR | O_NDELAY);
    bt_fd = open("/dev/ttyS2", O_RDWR | O_NDELAY);
    if (bt_fd > 0)
    {   //UART3打开成功
        //设置波特率9600
        struct termios term;
        bzero(&term, sizeof(struct termios));
        tcgetattr(bt_fd, &term);
        cfsetispeed(&term, B115200);
        cfsetospeed(&term, B115200);
        tcsetattr(bt_fd, TCSANOW, &term);
        ret = true;
    }
    else
    {
        printf("open ttys2 fail\n");
    }

    return ret;
}

/****************************************************************************
 * Name: bt_uart_close
 *
 * Description:
 *    关闭蓝牙设备句柄
 *
 * Parameters:
 *
 * Returned Value:
 *    true 初始化成功， false 初始化失败
 *
 * Assumptions:
 *
 ****************************************************************************/
bool bt_uart_close(void)
{
//    unsigned long value;
//    if(bt_fd > 0)
//    {   //UART3已经打开
//        //关闭UART3
//        close(bt_fd);
//    }
//    //清除设备句柄
//    bt_fd = -1;

//    //关闭UART3的IO口功能
//    value = REG32(KSEG1(SILAN_PADMUX_CTRL));
//    value &= ~(1 << SILAN_PADMUX_UART1);
//    REG32(KSEG1(SILAN_PADMUX_CTRL)) = value;
//    //输出低
//    zhuque_bsp_gpio_set_mode(UART_TX_PIN, GPIO_OUT, PULLING_DOWN);
//    zhuque_bsp_gpio_set_value(UART_TX_PIN, GPIO_VALUE_LOW);
//    zhuque_bsp_gpio_set_mode(UART_RX_PIN, GPIO_OUT, PULLING_DOWN);
//    zhuque_bsp_gpio_set_value(UART_RX_PIN, GPIO_VALUE_LOW);
    return true;
}

/****************************************************************************
 * Name: handle_bt_cmd
 *
 * Description:
 *    通过串口向蓝牙模块发送命令
 *
 * Parameters:
 *    cmd 要发送的命令
 *
 * Returned Value:
 *    0 发送成功， -1 发送失败失败
 *
 * Assumptions:
 *
 ****************************************************************************/
int handle_bt_cmd(int cmd)
{
    int ret = -1;
    char buf[12] = {0};

    if (bt_fd < 0)
    {
        printf("%s:UART for bluetooth is not open\n", __func__);
    }
    else
    {
        switch (cmd)
        {
            case BT_POWER:
                strcpy(buf, BT_power);
                break;
            case BT_MODE:
                strcpy(buf, BT_mode);
                break;
            case AUX_MODE:
                strcpy(buf, AUX_mode);
                break;
            case FM_MODE:
                strcpy(buf, FM_mode);
                break;
            case BT_PRV:
                strcpy(buf, BT_prv);
                break;
            case BT_NEXT:
                strcpy(buf, BT_next);
                break;
            case BT_VOL_DOWN:
                strcpy(buf, BT_vol_down);
                break;
            case BT_VOL_UP:
                strcpy(buf, BT_vol_up);
                break;
            case BT_PAIRE:
                strcpy(buf, BT_paire);
                break;
            case BT_DIS_PAIRE:
                strcpy(buf, BT_dis_paire);
                break;
            case BT_AUTO_CONNECT:
                strcpy(buf, BT_auto_connect);
                break;
            case BT_DIS_CONNECT:
                strcpy(buf, BT_dis_connect);
                break;
            case BT_PAUSE:
                strcpy(buf, BT_pause);
                break;
            case BT_PLAY:
                strcpy(buf, BT_play);
                break;
            case BT_STOP:
                strcpy(buf, BT_stop);
                break;
            case FM_SEARCH:
                strcpy(buf, FM_search);
                break;
            case FM_STOP:
                strcpy(buf, FM_stop);
                break;
            case FM_GET_FQ:
                strcpy(buf, FM_get_fq);
                break;
            case SD_MODE:
                strcpy(buf, COM_sd);
                break;
            case COM_IQ:
                strcpy(buf, COM_iq);
                break;
            case AT_D8836:
                strcpy(buf, AT_d8836);
                break;
			case GET_AUX:
				strcpy(buf, GET_aux);
                break;
			case PWR_UP:
				strcpy(buf, PWR_up);
                break;
			case PWR_OFF:
				strcpy(buf, PWR_off);
                break;
            default:
                break;
        }

        write(bt_fd, buf, 12);

        printf("%s: buf:%s\n", __func__, buf);

        ret = 0;
    }

    return ret;
}

void handle_bt_send_volume(int volume)
{
    if (bt_fd < 0)
    {
        printf("%s:UART for bluetooth is not open\n", __func__);
    }
    else
    {
		char const str1[] = "AT+VOL+";
		char buf[16] = {0};
		int len = 0;
		len = strlen(str1);
		memcpy(buf, str1, len);
		sprintf(buf+len, "%02d\r\n", volume);
		printf("88888888%s \n", buf);

		write(bt_fd, buf, 12);

		printf("%s: buf:%s\n", __func__, buf);
    }
}


