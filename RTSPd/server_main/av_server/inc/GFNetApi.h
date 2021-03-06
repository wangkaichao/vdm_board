 /******************************************************************************
* 版权信息：
* 系统名称：
* 文件名称：GFNet.h
* 文件说明：该文件定义了对外的数据结构和函数接口

* 更新说明：20090409：	修改登录用户数，可以同时登录16人
						调整连接中心模式，修改部分同步造成的不稳定问题

			20090415	调整了一个接收异步，修改交互异步可能造成的不稳定

			20110421    修改登录串口数据发送和报警等发送部分
						增加一个socket 进登录的select socket列表，在有串口数据和报警需要
						发送给PC的时候，通过发送数据到新socket 快速唤醒select阻塞，
						进行串口数据和报警的发送
			20110805    1，修改登录串口数据发送方式，改正了以前在多用户登录的时候串口数据只发送中心IP的情况，
							数据发给所有登录用户
						2，增加图片发送接口
			20120224	修改UPNP，适应D-LINK的端口映射问题
******************************************************************************/

#ifndef		GF_SERVER_NET_H_
#define		GF_SERVER_NET_H_

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <time.h>

/////////////////////////**********************宏定义
#define USER_NAME_LENGTH_IN		16
#define USER_PSW_LENGTH_IN		16

/////////////////////////**********************数据结构

//侦头
typedef struct _MP4_FRAME_HEAD
{
	unsigned int 	nMagicCode;
	unsigned int	nTimeTick;
	unsigned int	nVideoSize;
	unsigned int	bKeyFrame;
	unsigned int	nAudioSize;
	unsigned int	nReserve;	
}MP4_FRAME_HEAD,*PMP4_FRAME_HEAD;

typedef enum _GFERR_CODE
{
	GFERR_SUCCESS,					//操作成功
	GFERR_FAILURE,					//操作失败
	GFERR_REFUSE_REQ,				//请求被拒绝
	GFERR_USER_FULL,				//登录用户已满
	GFERR_PREVIEW_FULL,				//预览用户已满
	GFERR_TASK_FULL,				//系统任务繁忙，待会尝试连接
	GFERR_CHANNEL_NOT_EXIST,		//要打开的通道不存在或已满
	GFERR_DEVICE_NAME,				//打开的设备不存在
	GFERR_IS_TALKING,				//正在对讲
	GFERR_QUEUE_FAILUE,				//队列出错
	GFERR_USER_PASSWORD,			//用户名或密码和系统不匹配
	GFERR_SHARE_SOCKET,
	GFERR_RELAY_NOT_OPEN,
	GFERR_RELAY_MULTI_PORT,
	GFERR_CAPTURE_OVERTIME,
	
	GFERR_INVALID_PARAMETER=100,	//输入参数无效
	GFERR_LOGON_FAILURE,			//登录失败
	GFERR_TIME_OUT,					//操作超时
	GFERR_SOCKET_ERR,				//SOCKET错误
	GFERR_NOT_LINKSERVER,			//还未连接服务器
	GFERR_BUFFER_EXTCEED_LIMIT,		//使用缓冲超过限制	
	GFERR_LOW_PRIORITY,				//操作权限不足
	GFERR_BUFFER_SMALL,				//缓冲太小
	GFERR_IS_BUSY,					//系统任务正忙
	GFERR_UPDATE_FILE,				//升级文件错误
	GFERR_UPDATE_UNMATCH,			//升级文件和机器不匹配
	GFERR_PORT_INUSE,				//端口被占用
	GFERR_RELAY_DEVICE_EXIST,		//

    //2007.6.13
	GFERR_CONNECT_REFUSED,			//连?邮北�?�?
	GFERR_PROT_NOT_SURPPORT,		//不支持?

	GFERR_FILE_OPEN_ERR,            //?蚩�?
	GFERR_FILE_SEEK_ERR,            //fseek
	GFERR_FILE_WRITE_ERR,           //fwrite
	GFERR_FILE_READ_ERR,            //fread
	GFERR_FILE_CLOSING,             //            
}GFERR_CODE;

//通知应用程序窗口消息命令
typedef enum _GFMSG_NOTIFY
{
	GFMSG_CONNECT_CLOSE,			//登录连接关闭
	GFMSG_CHANNEL_CLOSE,			//通道连接关闭
	GFMSG_TALK_CLOSE,				//对讲连接关闭
	GFMSG_ALARM_OUTPUT,				//报警输出
	GFMSG_UPDATE_SEND_PERCENT,		//升级程序发送百分比
	GFMSG_UPDATE_SAVE_PERCENT,		//升级写入发送百分比
	GFMSG_BROADCAST_CLOSE,			//语音广播中一个断开
	GFMSG_SENSOR_CAPTURE,			//探头触发的抓拍
	GFMSG_COM_DATA,					//串口采集数据
}GFMSG_NOTIFY;

typedef enum _GFCMD_NET
{
	GFCMD_GET_ALL_PARAMETER,		//0. 得到所有编码器参数
	GFCMD_SET_DEFAULT_PARAMETER,	//1. 恢复所有编码器默认参数
	GFCMD_SET_RESTART_DVS,			//2. 重启编码器
	GFCMD_GET_SYS_CONFIG,			//3. 获取系统设置
	GFCMD_SET_SYS_CONFIG,			//4. 设置系统设置
	GFCMD_GET_TIME,					//5. 获取编码器时间
	GFCMD_SET_TIME,					//6. 设置编码器时间
	GFCMD_GET_AUDIO_CONFIG,			//7. 获取音频设置
	GFCMD_SET_AUDIO_CONFIG,			//8. 设置音频设置
	GFCMD_GET_VIDEO_CONFIG,			//9. 获取视频设置
	GFCMD_SET_VIDEO_CONFIG,			//10.设置视频设置
	GFCMD_GET_VMOTION_CONFIG,		//11.获取移动侦测设置
	GFCMD_SET_VMOTION_CONFIG,		//12.设置移动侦测设置
	GFCMD_GET_VMASK_CONFIG,			//13.获取图像屏蔽设置
	GFCMD_SET_VMASK_CONFIG,			//14.设置图像屏蔽设置
	GFCMD_GET_VLOST_CONFIG,			//15.获取视频丢失设置
	GFCMD_SET_VLOST_CONFIG,			//16.设置视频丢失设置
	GFCMD_GET_SENSOR_ALARM,			//17.获取探头报警侦测设置
	GFCMD_SET_SENSOR_ALARM,			//18.设置探头报警侦测设置
	GFCMD_GET_USER_CONFIG,			//19.获取用户设置
	GFCMD_SET_USER_CONFIG,			//20.设置用户设置
	GFCMD_GET_NET_CONFIG,			//21.获取网络设置结构
	GFCMD_SET_NET_CONFIG,			//22.设置网络设置结构
	GFCMD_GET_COM_CONFIG,			//23.获取串口设置
	GFCMD_SET_COM_CONFIG,			//24.设置串口设置
	GFCMD_GET_YUNTAI_CONFIG,		//25.获取内置云台信息
	GFCMD_SET_YUNTAI_CONFIG,		//26.设置内置云台信息
	GFCMD_GET_VIDEO_SIGNAL_CONFIG,	//27.获取视频信号参数（亮度、色度、对比度、饱和度）
	GFCMD_SET_VIDEO_SIGNAL_CONFIG,	//28.设置视频信号参数（亮度、色度、对比度、饱和度）
	GFCMD_SET_PAN_CTRL,				//29.云台控制
	GFCMD_SET_COMM_SENDDATA,		//30.透明数据传输
	GFCMD_SET_COMM_START_GETDATA,	//31.开始采集透明数据
	GFCMD_SET_COMM_STOP_GETDATA,	//32.停止采集透明数据
	GFCMD_SET_OUTPUT_CTRL,			//33.继电器控制
	GFCMD_SET_PRINT_DEBUG,			//34.调试信息开关
	GFCMD_SET_ALARM_CLEAR,			//35.清除报警
	GFCMD_GET_ALARM_INFO,			//36.获取报警状态和继电器状态
	GFCMD_SET_TW2824,				//37.设置多画面芯片参数(保留)
	GFCMD_SET_SAVE_PARAM,			//38.设置保存参数
	GFCMD_GET_USERINFO,				//39.获取当前登陆的用户信息
	GFCMD_GET_DDNS,					//40.获取DDNS
	GFCMD_SET_DDNS,					//41.设置DDNS
	GFCMD_GET_CAPTURE_PIC,			//42.前端抓拍
	GFCMD_GET_SENSOR_CAP,			//43.获取触发抓拍设置
	GFCMD_SET_SENSOR_CAP,			//44.设置触发抓拍设置
	GFCMD_GET_EXTINFO,				//45.获取扩展配置
	GFCMD_SET_EXTINFO,				//46.设置扩展配置
	GFCMD_GET_USERDATA,				//47.获取用户配置
	GFCMD_SET_USERDATA,				//48.设置用户配置
	GFCMD_GET_NTP,					//49.获取NTP配置
	GFCMD_SET_NTP,					//50.设置NTP配置
	GFCMD_GET_UPNP,					//51.获取UPNP配置
	GFCMD_SET_UPNP,					//52.设置UPNP配置
	GFCMD_GET_MAIL,					//53.获取MAIL配置
	GFCMD_SET_MAIL,					//54.设置MAIL配置
	GFCMD_GET_ALARMNAME,			//55.获取报警名配置
	GFCMD_SET_ALARMNAME,			//56.设置报警名配置
	GFCMD_GET_WFNET,				//57.获取无线网络配置
	GFCMD_SET_WFNET,				//58.设置无线网络配置
	GFCMD_GET_SEND_DEST,			//59.设置视频定向发送目标机
	GFCMD_SET_SEND_DEST,			//60.设置视频定向发送目标机
	GFCMD_GET_AUTO_RESET,			//61.取得定时重新注册
	GFCMD_SET_AUTO_RESET,			//62.设置定时重新注册
	GFCMD_GET_REC_SCHEDULE,			//63.取得录像策略
	GFCMD_SET_REC_SCHEDULE,			//64.设置录像策略
	GFCMD_GET_DISK_INFO,			//65.取得磁盘信息
	GFCMD_SET_MANAGE,				//66.设置命令和操作
	GFCMD_GET_CMOS_REG,				//67.取得CMOS参数
	GFCMD_SET_CMOS_REG,				//68.设置CMOS参数
	GFCMD_SET_SYSTEM_CMD,			//69.设置执行命令
	GFCMD_SET_KEYFRAME_REQ,			//70.设置关键帧请求
	GFCMD_GET_CONFIGENCPAR,			//71.取得视频参数
	GFCMD_SET_CONFIGENCPAR,			//72.设置视频参数
	//--------------------------
	GFCMD_GET_ALL_PARAMETER_NEW,	//73.获取所有参数
	GFCMD_FING_LOG,					//74.查找日志(查询方式:0－全部，1－按类型，2－按时间，3－按时间和类型 0xFF-关闭本次搜索)
	GFCMD_GET_LOG,					//75.读取查找到的日志	
	GFCMD_GET_SUPPORT_AV_FMT,		//76.获取设备支持的编码格式、宽高及音频格式
	GFCMD_GET_VIDEO_CONFIG_NEW,		//77.视频参数－new
	GFCMD_SET_VIDEO_CONFIG_NEW,		//78.
	GFCMD_GET_VMOTION_CONFIG_NEW,	//79.移动报警参数－new
	GFCMD_SET_VMOTION_CONFIG_NEW,	//80.
	GFCMD_GET_VLOST_CONFIG_NEW,		//81.视频丢失报警参数－new
	GFCMD_SET_VLOST_CONFIG_NEW,		//82.
	GFCMD_GET_SENSOR_ALARM_NEW,		//83.探头报警参数－new
	GFCMD_SET_SENSOR_ALARM_NEW,		//84.
	GFCMD_GET_NET_ALARM_CONFIG,		//85.网络故障报警参数－new
	GFCMD_SET_NET_ALARM_CONFIG,		//86.
	GFCMD_GET_RECORD_CONFIG,		//87.定时录像参数
	GFCMD_SET_RECORD_CONFIG,		//88.
	GFCMD_GET_SHOOT_CONFIG,			//89.定时抓拍参数
	GFCMD_SET_SHOOT_CONFIG,			//90.
	GFCMD_GET_FTP_CONFIG,			//91.FTP参数
	GFCMD_SET_FTP_CONFIG,			//92.
	GFCMD_GET_RF_ALARM_CONFIG,		//93.无线报警参数
	GFCMD_SET_RF_ALARM_CONFIG,		//94.
	GFCMD_GET_EXT_DATA_CONFIG,		//95.其它扩展参数(如平台设置其它参数)
	GFCMD_SET_EXT_DATA_CONFIG,		//96.
	GFCMD_GET_FORMAT_PROCESS,		//97.获取硬盘格式化进度
	GFCMD_GET_PING_CONFIG,			//98.PING 设置获取
	GFCMD_SET_PING_CONFIG,			//99.PING 设置设置

	//解码器命令
	DDCMD_GET_ALL_PARAMETER = 100,	//获取解码器所有设置
	DDCMD_GET_TIME,					//获取系统时间
	DDCMD_SET_TIME,					//设置系统时间
	DDCMD_GET_SYS_CONFIG,			//获取系统配置
	DDCMD_SET_SYS_CONFIG,			//设置系统配置
	DDCMD_GET_NET_CONFIG,			//获取网络配置
	DDCMD_SET_NET_CONFIG,			//设置网络配置
	DDCMD_GET_COM_CONFIG,			//获取串口配置
	DDCMD_SET_COM_CONFIG,			//设置串口配置
	DDCMD_GET_VIDEO_CONFIG,			//获取视频配置
	DDCMD_SET_VIDEO_CONFIG,			//设置视频配置
	DDCMD_GET_ALARM_OPT,			//获取报警选项
	DDCMD_SET_ALARM_OPT,			//设置报警选项
	DDCMD_GET_USER_INFO,			//获取用户设置信息
	DDCMD_SET_USER_INFO,			//设置用户设置信息
	DDCMD_GET_ALARM_RECORD,			//获取报警记录信息
	DDCMD_GET_ADRRESS_BOOK,			//获取地址薄配置
	DDCMD_SET_ADRRESS_BOOK,			//设置地址薄配置
	DDCMD_SET_COMM,					//设置发送串口数据
	DDCMD_SET_CMD,					//设置透明的命令
	DDCMD_GET_YUNTAI_INFO,			//获取云台信息
	DDCMD_GET_YUNTAI_CONFIG,		//获取云台配置
	DDCMD_SET_YUNTAI_CONFIG,		//设置云台配置
	DDCMD_GET_ONELINK_ADDR,			//获取解码器单路连接的信息
	DDCMD_SET_ONELINK_ADDR,			//设置解码器单路连接的信息
	DDCMD_GET_CYCLELINK_ADDR,		//获取解码器循环连接的信息
	DDCMD_SET_CYCLELINK_ADDR,		//设置解码器循环连接的信息
	DDCMD_GET_DDNS,					//获取DDNS
	DDCMD_SET_DDNS,					//设置DDNS
	
	GFCMD_GET_VPN_CONFIG = 200,
	GFCMD_SET_VPN_CONFIG,
	GFCMD_GET_3G_CONFIG,
	GFCMD_SET_3G_CONFIG,
	GFCMD_GET_GPS_CONFIG,
	GFCMD_SET_GPS_CONFIG,
		
	GFCMD_GET_VERSION_STRING = 400,	//3518针对平台，获取版本字符串
	
	NETCMD_GET_VI_SENSOR=1000,
	NETCMD_SET_VI_SENSOR,
	NETCMD_GET_VI_SCENE,
	NETCMD_SET_VI_SCENE,
	NETCMD_GET_VI_CFG,
	NETCMD_SET_VI_CFG,
	NETCMD_GET_DOME_PTZ_CFG,
	NETCMD_SET_DOME_PTZ_CFG,	
	NETCMD_GET_DOME_PRESET_CFG,
	NETCMD_SET_DOME_PRESET_CFG,	
	NETCMD_MAX_CONFIG,	
}GFCMD_NET;

typedef enum _NET_MSG_NOTIFY
{
	MSG_LOGIN_LINK,
	MSG_LOGIN_UNLINK,
	MSG_PREVIEW_LINK,
	MSG_PREVIEW_UNLINK,
	MSG_TALK_LINK,
	MSG_TALK_UNLINK,
	MSG_BROADCAST_LINK,
	MSG_BROADCAST_UNLINK,
	MSG_FILE_LINK,
	MSG_FILE_UNLINK,
	MSG_PIC_LINK,
	MSG_PIC_UNLINK,
	MSG_PIC_REQUEST,
	MSG_CENTER_LINK,
	MSG_CENTER_UNLINK,
}NET_MSG_NOTIFY;

unsigned long GFSNet_WriteFrame(int nChannelNo,const char *pFrameData,const char *pAudioData);


#endif
























