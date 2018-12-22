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
/*蓝牙调试开关*/
#define BT_DEBUG    0
/*蓝牙通信UART速率9600开关*/
#define BT_UART_9600    0
/*与蓝牙模块通信IO口引脚定义*/
#define UART_TX_PIN (13)
#define UART_RX_PIN (14)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/
/*与蓝牙模块通信的字符串*/
static const char * AT_CMDS[] = {
	"ATPR\r\n",			//进入配对
	"ATAC\r\n",            //连接最后配对的
	"ATDC\r\n",            //断开连接
	"AT+CE\n",            //ACK
	"AT+CF\n",            //拒接电话
	"AT+CG\n",            //挂断电话
	"AT+CH\n",            //重拨最后一个电
	"AT+ADD\n",            //音量加
	"AT+SUB\n",            //音量减
	"ATCL\r\n",            //清除记忆列表
	"AT+CO\n",            //
	"AT+CV\n",            //语音拨号
	"AT+CT\n",            //
	"ATPA\r\n",            //播放/暂停
	"ATPU\r\n",            //暂停
	"ATPN\r\n",            //下一曲
	"ATPV\r\n",            //上一曲
	"AT+PF\n",            //下一个动作
	"AT+PH\n",            //倒带
	"AT+ON\n",            //开机
	"AT+OFF\n",           //关机
	"AT+GET+MODE\n",      //获取当前模式
	"AT+GET+VOL\n",       //获取当前音量值
	"AT+BT\n",			//蓝牙模式
	"AT+AUX\n",           //AUX模式
	"AT+D8836\n",         //退出模式
	"AT+WMN\n",           //下一个模式
	//蓝牙状态命令
	//"ATCN\r\n",		    //设备连接成功
	//"ATWC\r\n",            //设备断开连接
	"CN",		    //设备连接成功
	"WC",            //设备断开连接
	"AT+MA\n",            //蓝牙播放暂停
	"AT+MB\n",            //蓝牙开始播放
	"ATCX\r\n",            //查询蓝牙设备连接状态
	"AT+GET+AUX\n",       //AUX连接状态查询
	"AT+AUX01\n",         //AUX连接
	"AT+AUX00\n",         //AUX断开连接
	"ATIS\r\n",            //复位I2S
	"ATDS\r\n",            //关闭I2S
	"ATVE\r\n",            //查看版本号
	//带参数命令
	"AT+WM+",               //特定模式设置
	"ATVER",              //版本号
	"AT+NUM+",              //数字
	"AT+VOL+",              //音量
	"AT+EQ+",               //音效
	"AT+WAV+",              //TONE

};
/*与蓝牙模块通信串口文件描述符*/
int bt_fd = 0;
/*蓝牙同步信号量*/
static sem_t bt_state_sem;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bt_chk_AT_cmd
 *
 * Description:
 *    检测不带参数的AT命令
 *
 * Parameters:
 *    cmd 命令字符串
 *    val 返回的值
 *
 * Returned Value:
 *    -1不是AT命令，命令对就的序号
 *
 * Assumptions:
 *
 ****************************************************************************/
int bt_chk_AT_cmd(char *cmd, int *pVal)
{
    const int cmds_num = sizeof(AT_CMDS)/sizeof(const char *);
    int ret = AT_START;
    int i;
    char * ptr = NULL;

    for(i=0;i<cmds_num;++i)
    {
        if ((ptr = strstr(cmd, AT_CMDS[i])) != NULL)
        {//蓝牙连接
            int j = 0;
            int len = strlen(AT_CMDS[i]);
            if(i < AT_SET_MODE)
            {
                //清除字符串中的命令
                for(j=0;j<len;++j)
                {
                    ptr[j]=' ';
                }
                ret = i;
            }
            else
            {
                //计算参数值
                int val = 0;
                for(j=len;j<len+3;++j)
                {
                    if('0' <= ptr[j] && ptr[j] <= '9')
                    {
                        val *= 10;
                        val += ptr[j] - '0';
                    }
                    else
                    {
                        break;
                    }
                }

                if(ptr[j]==0)
                {
					//数据未读完
                }
                else if(j == len || ptr[j] != '\r')
                {//无效命令
                    ++j;
                    for(;j>=0;--j)
                    {
                        ptr[j]=' ';
                    }
                }
                else if(ptr[j] == '\r')
                {//有效命令
                    //清除字符串中的命令
                    ++j;
                    for(;j>=0;--j)
                    {
                        ptr[j]=' ';
                    }
                    ret = i;
                    *pVal = val;
                }
            }
            break;
        }
    }

    return ret;
}

/****************************************************************************
 * Name: bt_read
 *
 * Description:
 *    通过串口读取蓝牙模块反馈的信息
 *
 * Parameters:
 *    buf 接收数据用的buf
 *    buf_szie 缓存的大小
 *
 * Returned Value:
 *    -1 获取数据失败， >= 0 获取数据成功
 *
 * Assumptions:
 *    buf的长度必须大于等于buf_szie
 *
 ****************************************************************************/
int bt_read(char *buf, int buf_szie)
{
    int ret = -1;

    if (bt_fd < 0)
    {   //无效串口文件描述符
        #if BT_DEBUG
        printf("%s:UART for bluetooth is not open\n", __func__);
        #endif
    }
    else if(NULL == buf)
    {
        #if BT_DEBUG
        printf("%s:error buffer\n", __func__);
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
            #if BT_DEBUG
            printf("%s:select err or timeout\n", __func__);
            #endif
        }
        else
        {
            if (FD_ISSET(bt_fd, &fds) > 0)
            {   ///有数据可以从串口读取
                memset(buf, 0, buf_szie);
                int count = read(bt_fd, buf, buf_szie);
                if (count <= 0)
                {   //串口中没有数据
                    #if BT_DEBUG
                    printf("%s:Read nothing from UART\n", __func__);
                    #endif
                    ret = 0;
                }
                else
                {   //从串口读到了数据
                    ret = count;
                    printf("%s:buf_rev %s count:%d\r\n", __func__, buf, count);
                }
            }
            else
            {   //串口中无数据可读
                #if BT_DEBUG
                printf("%s:Nothing can be read from UART\n", __func__);
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
	static bool sem_is_inited = false;
	if(!sem_is_inited)
	{
		sem_is_inited = true;
		//初始化蓝牙发送同步信号量
		sem_init(&bt_state_sem, 0, 1);
	}

	bool ret = false;
	unsigned long value;

	//UART3
	zhuque_bsp_gpio_set_mode(UART_TX_PIN, GPIO_OUT, PULLING_HIGH);
	zhuque_bsp_gpio_set_value(UART_TX_PIN, GPIO_VALUE_LOW);
	zhuque_bsp_gpio_set_mode(UART_RX_PIN, GPIO_IN, PULLING_HIGH);
	zhuque_bsp_gpio_set_value(UART_RX_PIN, GPIO_VALUE_LOW);

	value = REG32(KSEG1(SILAN_PADMUX_CTRL));
	value |= (1 << SILAN_PADMUX_UART3_1);

	//  value |= (1 << SILAN_PADMUX_UART3_2);
	REG32(KSEG1(SILAN_PADMUX_CTRL)) = value;

	//bt_fd = open("/dev/ttyS1", O_RDWR | O_NDELAY);
	bt_fd = open("/dev/ttyS2", O_RDWR | O_NDELAY);
	if (bt_fd > 0)
	{
#if BT_UART_9600
		struct termios term;
		bzero(&term, sizeof(struct termios));
		tcgetattr(bt_fd, &term);
		cfsetispeed(&term, B9600);
		cfsetospeed(&term, B9600);
		tcsetattr(bt_fd, TCSANOW, &term);
#endif
		ret = true;
	}
	else
	{
		printf("open ttys1 fail\n");
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
    sem_wait(&bt_state_sem);

    unsigned long value;
    if(bt_fd > 0)
    {   //UART3已经打开
        //关闭UART3
        close(bt_fd);
    }
    //清除设备句柄
    bt_fd = -1;

    //关闭UART3的IO口功能
    value = REG32(KSEG1(SILAN_PADMUX_CTRL));
    value &= ~(1 << SILAN_PADMUX_UART3_2);
    REG32(KSEG1(SILAN_PADMUX_CTRL)) = value;
    //输出低
    zhuque_bsp_gpio_set_mode(UART_TX_PIN, GPIO_OUT, PULLING_DOWN);
    zhuque_bsp_gpio_set_value(UART_TX_PIN, GPIO_VALUE_LOW);
    zhuque_bsp_gpio_set_mode(UART_RX_PIN, GPIO_OUT, PULLING_DOWN);
    zhuque_bsp_gpio_set_value(UART_RX_PIN, GPIO_VALUE_LOW);

    sem_post(&bt_state_sem);
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
 *    value 命令参数
 *
 * Returned Value:
 *    0 发送成功， -1 发送失败失败
 *
 * Assumptions:
 *
 ****************************************************************************/
int handle_bt_cmd(AT_CMD cmd, int value)
{
    sem_wait(&bt_state_sem);
    int ret = -1;
    char buf[16] = {0};

    if (bt_fd < 0)
    {
        printf("%s:UART for bluetooth is not open\n", __func__);
    }
    else
    {
        if(cmd > AT_START && cmd < AT_MAX)
        {
            bool execute = true;

            memcpy(buf, AT_CMDS[cmd], 16);

            if(cmd >= AT_SET_MODE)
            {
                if(value >= 0 && value < 100)
                {
                    int len = strlen(AT_CMDS[cmd]);
                    sprintf(buf+len, "%02d\r\n" , value);
                }
                else
                {
                    execute = false;
                }
            }

            if(execute)
            {
                write(bt_fd, buf, 6);
                printf("%s: buf:%s\n", __func__, buf);
            }

            ret = 0;
        }
    }

    sem_post(&bt_state_sem);

    return ret;
}


