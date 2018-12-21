#ifndef __BT_UART_H__
#define __BT_UART_H__

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
typedef enum {
    AT_START = -1,
    AT_ENTER_PAIR,              //进入配对
    AT_CONNECT_LAST_DEVICE,     //连接最后配对的设备
    AT_DISCONNECT,              //断开连接
    AT_ACK,                     //ACK
    AT_REJECT_PHONE,            //拒接电话
    AT_HANG_UP_PHONE,           //挂断电话
    AT_REDIAL,                  //重拨最后一个电话
    AT_VOL_UP,                  //音量加
    AT_VOL_DOWN,                //音量减
    AT_CLEAR_LIST,              //清除记忆列表
    AT_HF_TRANSFER_TOGGLE,      //
    AT_VOICD_DIAL,              //语音拨号
    AT_ENTER_DUT_MODE,          //
    AT_PLAY_OR_PAUSE,           //播放/暂停
    AT_STOP,                    //暂停
    AT_NEXT_TRACE,              //下一曲
    AT_PREV_TRACE,              //上一曲
    AT_FORWARD_ACTION,          //下一个动作
    AT_REWIND_ACTION,           //倒带
    AT_ON,                      //开机
    AT_OFF,                     //关机
    AT_GET_MODE,                //获取当前模式
    AT_GET_VOL,                 //获取当前音量值
    AT_BT,                      //蓝牙模式
    AT_AUX,                     //AUX模式
    AT_D8836,                   //退出模式
    AT_NEXT_MODE,               //下一个模式
    //蓝牙状态命令
    AT_DEVICE_CONNECTED,        //设备连接成功
    AT_DEVICE_DISCONNECTED,     //设备断开连接
    AT_BT_PAUSE,                //蓝牙播放暂停
    AT_BT_PALY,                 //蓝牙开始播放
    AT_QUERY_CONNECT_STATE,     //查询蓝牙连接状态
    AT_GET_AUX,                 //AUX连接状态查询
    AT_AUX_CONNECTED,           //AUX连接
    AT_AUX_DISCONNECTED,        //AUX断开连接
    AT_IS,
    AT_DS,
    AT_GET_VERSION,             //查看版本号
	AT_MODE_COA,
	AT_MODE_HDMI,
	AT_MODE_FM,
	AT_MODE_BT,
	AT_MODE_USB,
	AT_MODE_AUX,
	AT_MODE_OPT,
	AT_PLAY_PAUSE,           //播放/暂停
    AT_PLAY_NEXT,              //下一曲
    AT_PLAY_PREV,              //上一曲
    AT_MIC_ON,
    AT_MIC_OFF,
    AT_MOVIE_ON,
    AT_MOVIE_OFF,
	
    //带参数命令
    AT_SET_MODE,                //模式设置
    AT_VERSION,                //版本号
    AT_SET_MAINVOL,
    AT_SET_TREBLE,
    AT_SET_BASS,
    AT_SET_ECHO,
    AT_SET_MICVOL,
    AT_CUR_MAINVOL,
    AT_CUR_TREBLE,
    AT_CUR_BASS,
    AT_CUR_ECHO,
    AT_CUR_MICVOL,
    AT_MODE_NUMBER,             //数字
    AT_SET_VOL,                 //音量设置
    AT_EQ,                      //音效设置
    AT_TONE,                    //TONE
    AT_MAX,
} AT_CMD;

/****************************************************************************
 * Public Data
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
 *
 * Returned Value:
 *    -1不是AT命令，命令对就的序号
 *
 * Assumptions:
 *
 ****************************************************************************/
int bt_chk_AT_cmd(char *cmd, int *pVal);

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
int bt_read(char *buf_receive, int buf_size);

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
bool bt_uart_init(void);

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
bool bt_uart_close(void);

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
int handle_bt_cmd(AT_CMD cmd, int value);
#endif
