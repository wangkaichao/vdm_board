/******************************************************************************
* 版权信息：
* 系统名称：
* 文件名称：GFNetStruct.h
* 文件说明：该文件定义了和客户端交互的数据结构和宏定义
******************************************************************************/
#ifndef GF_NET_STRUCT_H_
#define GF_NET_STRUCT_H_

#include "global.h"
#include "GFNetApi.h"
#include "GFNetComm.h"


#define TRUE	1
#define FALSE	0

//#define	GF_NET_RELEASE

//5K
#define RECEIVE_SEND_CMD_SIZE	5120

#define	SERVER_MAX_LOGON_NUM	16
#define SERVER_MAX_TCPVIEW_NUM	16
#define SERVER_MAX_TCPPIC_NUM	4

#define SERVER_MAX_MULTIVIEW_NUM 60
#define SERVER_MAX_TASK_NUM		32

#define MAX_REMOTE_PLAY_NUM     6



#define TASK_STACK_SYS		32768
#define TASK_STACK_USER		32768

#define DEFAULT_POOL_NUM		1024
#define DEFAULT_POOL_SIZE		1024
#define MAX_POOL_SIZE			8192

#define SOCK_SELECT_TIME_OVER	3000 //单位ms
#define OVER_KEEP_TIME_COUNT	30
#define DEFAULT_AUDIO_BUF_COUNT		100

///////////////////////////////////////// DWT
#define	DWT_SLEEP_TICK		5
#define DWT_NET_USE
#ifdef	DWT_NET_USE



#define	COMM_SEND_NEW_METHOD	1




#define	TCP_SEND_TRY		16000
#define	KEEP_ALIVE_TIMES	3*2
#define KEEP_ALIVE_IDLE		20*2
#define	KEEP_ALIVE_INTAL	10*2

#else
//#define	TCP_SEND_TRY		6000
#define	TCP_SEND_TRY		16000
#define	KEEP_ALIVE_TIMES	3
#define KEEP_ALIVE_IDLE		20
#define	KEEP_ALIVE_INTAL	10

#endif
///////////////////////////////////////// DWT

#define STREAM_FLAG_VP		0x0b
#define STREAM_FLAG_VI		0x0e
#define STREAM_FLAG_A		0x0d

#define	MILLISECOND_TICK	(200000)


#define	ALARM_BUFFER_OFFSET	100

#define	ALARM_PIC_BUF		4
#define	ALARM_COMM_BUFF		20
#define	ALARM_MSG_BUF		10

#define GPS_MSG_BUF_NUM		8 //ljm 2010-03-19s

#define	TALK_ARRAY_NUM		100
#define	PIC_ARR_NUM			3

typedef int   (*NSSNotifyFrame)(int bAudio,int nChannel);


//服务器信息
typedef struct _SERVER_INFO
{
	//服务器属性
//	char			szDeviceName[64];
	char			szServerID[64];
	int				nChannelNum;//通道
	int				nBasePort;//端口
	unsigned long 	multiAddr;//多播地址,整型值

	AV_PAR          avInfoStream[SER_MAX_CHANNEL];//通道的AV参数信息
	int             nVBufferCount;//发送缓冲大小,建议100以上
	int				nABufferCount;
	
	int				hListenSock;//TCP监听SOCKET
	pthread_t		hTaskTcpListen;//TCP监听任务

	int				hUdpListenSock;//UDP监听SOCKET
	pthread_t		hTaskUdpListen;//UDP监听任务

	unsigned long	hTaskUdpFind;

	int				hMultiListenSock;//多播监听SOCKET
	int             hMultiListenSockFind;
	
	pthread_t		hTaskMultiListen;//多播监听任务
   	pthread_mutex_t	mutexMultiWait;
	pthread_cond_t	condMultiWait;
    pthread_t	    hTaskMultiSend[SER_MAX_CHANNEL];//多播发送任务

	int				hTalkSock;//对讲SOCKET
	pthread_t		hTalkTask;//对讲任务
	unsigned long	fullTalkQueue;
	unsigned long	emptyTalkQueue;
    int             talkHaveData;
	unsigned long	talkArrayIndex;
	unsigned long	talkLastestIndex;
	unsigned char	talkArrayBuf[TALK_ARRAY_NUM][NET_TALK_AUDIO_G711+100];
	char			*pTalkBuffer;

	//升级
	char			*pUpdateFileBuf;
	char			szUpdateFileName[256];
	unsigned long	nUpdateFileLen;
	unsigned long	currentUpdateIP;
	unsigned long	currentUpdatePort;
	//对讲
	unsigned long			nTalkAudioEncodeType;
    unsigned long			nTalkAudioChannels;
    unsigned long			nTalkAudioBits;
    unsigned long			nTalkAudioSamples;
	int						bFlagTalking;
	int						nTalkingType;
	//报警
	unsigned long			centerIPAddress;
	char					*pAlarmStatusOut[ALARM_MSG_BUF];
	int						alarmStatusIndex;
	unsigned long			alarmStatusSend[ALARM_MSG_BUF][SERVER_MAX_LOGON_NUM];

	char                    *pGpsStatusOut[GPS_MSG_BUF_NUM];
	int                     gpsStatusIndex;
	unsigned long			gpsStatusSend[GPS_MSG_BUF_NUM][SERVER_MAX_LOGON_NUM];
	
	int						bMsgClientUsed[SERVER_MAX_LOGON_NUM];
	unsigned long			logonAddress[SERVER_MAX_LOGON_NUM];
	//
	char					*pAlarmCommCtrl[ALARM_COMM_BUFF];

	int						alarmCommIndex;
	//
	char					*pAlarmCap[ALARM_PIC_BUF];
	int						alarmCapInex;
	//
	unsigned long	hTaskKeepAlive;
	//
	int			bExistServer;
	int			bStartServer;
  //
  NSSNotifyFrame pNSSCallback;
  char			*pNSSFrame;

  pthread_mutex_t	muTalkMutex;
}SERVER_INFO;

typedef struct _CLIENT_INFO 
{
	unsigned long	ip;
	unsigned long	port;
	int				nPriority;
	pthread_t		nTaskID;
	int				hConnSocket;
	char			szUserName[USER_NAME_LENGTH_IN+1];
	char			szPsw[USER_PSW_LENGTH_IN+1];

	int				nOpenChannel;
	int			    bUdpSocket;

	unsigned long   nKeepAlive;

	unsigned long   nTaskNo;


	unsigned long	bUpdating;
	unsigned long	bRunning;

	int			    bCSModeClient;
	int				bSubChn;
	//
	pthread_mutex_t	mutex_logon;
	pthread_cond_t	cond_logon;

	unsigned long	sndByteCount;
	unsigned long	lostFrmCount;
	char			arrCommData[sizeof(COMM_HEAD)+sizeof(COMM_CTRL)];
}CLIENT_INFO;

typedef struct _CLIENT_INFO_HEAD
{
	//登录用户
	//unsigned long	fullClientQueue;
	//unsigned long   emptyClientQueue;
	//unsigned long   runningClientQueue;
	CLIENT_INFO		pClientInfoBuffer[SERVER_MAX_LOGON_NUM];
//	unsigned long	muLogonUser;

	//TCP,UDP预览用户
	//unsigned long	fullCPreviewQueue;
	//unsigned long   emptyPreviewQueue;
	//unsigned long   runningPreviewQueue;
	CLIENT_INFO		pPreviewInfoBuffer[SERVER_MAX_TCPVIEW_NUM];
//	unsigned long	muPreviewUser;

	//多播预览用户
	//unsigned long	fullMultiPreviewQueue;
	//unsigned long   emptyMultiPreviewQueue;
	CLIENT_INFO		pMultiPreviewBuffer[SERVER_MAX_MULTIVIEW_NUM];
//	unsigned long	muMultiPreviewUser;

	unsigned long	statusMultiSendChannel;
	//对讲用户
	unsigned long   bLogoffTalk;
	pthread_mutex_t talkMutex;
	pthread_cond_t	talkQueue;	
	CLIENT_INFO		talkClientInfo;
    
    //remote play user
    CLIENT_INFO     pRemotePlayUser[MAX_REMOTE_PLAY_NUM];
    
	//picture send
	CLIENT_INFO		pPictureUser[SERVER_MAX_TCPPIC_NUM];
}CLIENT_INFO_HEAD;

typedef struct _ARR_PIC_BUF
{
	int				bWriteNotSend;
	char			*pArrPictureBuf;
	int				nArrPicType;
	int				nArrWidth;
	int				nArrHeight;
	unsigned long	ArrtimeTick;
	int				nArrPictureSize;
}ARR_PIC_BUF;

typedef struct _CHANNEL_STREAM
{
	unsigned long	nChannelNo;

	NET_PACKET_VIDEO	*pPoolVideoPacket[MAX_POOL_SIZE];
	unsigned long	nPoolVideoCount;
	unsigned long   nWriteVideoBegin;
	unsigned long	nWriteVideoEnd;
	unsigned long	nWriteVideoCycle;
	pthread_mutex_t	muVideoWriteRead;

	unsigned long	nLastIFrameBegin;
	unsigned long   nLastIFrameEnd;
	unsigned long   nLastIFrameCycle;
	unsigned long	nLastIFrameCount;

	NET_PACKET_AUDIO	*pPoolAudioPacket[MAX_POOL_SIZE];
	unsigned long	nPoolAudioCount;
	unsigned long	nWriteAudioBegin;
	unsigned long	nWriteAudioEnd;
	unsigned long   nWriteAudioCycle;
	pthread_mutex_t	muAudioWriteRead;

	char			*pVideoBuffer;
	char			*pAudioBuffer;

	//写入的视频帧的帧号
	unsigned long   nVideoFrameTick;
	unsigned long   nAudioFrameTick;

	pthread_mutex_t	mutexWaitStreamData;
	pthread_cond_t	qWaitStreamData;


	//RTP等待流数据 mazhidong 2016-6-15
	pthread_mutex_t	mutexRtpWaitStreamData;
	pthread_cond_t	RtpWaitStreamData;

	unsigned long	existChannel;
	//
	int				sync_send_num;

	pthread_mutex_t	mutex_sync_send;
	pthread_cond_t	sync_send_queue;

	////////////////////////////////////////////
	NET_PACKET_VIDEO	*pPoolVideoPacketSub[MAX_POOL_SIZE];
	unsigned long	nPoolVideoCountSub;
	unsigned long   nWriteVideoBeginSub;
	unsigned long	nWriteVideoEndSub;
	unsigned long	nWriteVideoCycleSub;

	pthread_mutex_t	muVideoWriteReadSub;

	unsigned long	nLastIFrameBeginSub;
	unsigned long   nLastIFrameEndSub;
	unsigned long   nLastIFrameCycleSub;
	unsigned long	nLastIFrameCountSub;

	char			*pVideoBufferSub;
	unsigned long   nVideoFrameTickSub;	
	
	pthread_mutex_t	mutexWaitStreamDataSub;
	pthread_cond_t	qWaitStreamDataSub;

	//picture capture
	char			*pPictureBuf;
	int				nPicType;
	int				nWidth;
	int				nHeight;
	unsigned long	timeTick;
	int				nPictureSize;
	int				nPictureBufMax;
	unsigned long   nPictureBufTick;
	pthread_mutex_t	muPictureWriteRead;
	
	pthread_mutex_t	mutexPictureSend;
	pthread_cond_t	condPictureSend;

	unsigned long   nLastSendFrmNo;

	unsigned long   nLastestFrmPTS;
	unsigned long   nLastestFrmPTSSub;

	int				nRequstIFrmOK;
	int				hWaitDelaySock;
}CHANNEL_STREAM;

typedef struct _READ_POSITION
{
	unsigned long			read_begin;
	unsigned long			read_end;
	unsigned long			read_cycle;
	int						bAdjusting;
}READ_POSITION;

typedef struct _SEND_PIC{
	void				*picAddr;
	unsigned long		picLen;
	unsigned long		channel;
}SEND_PIC;

typedef struct _ALARM_HEAD
{
	unsigned long		alarmOnOff;
	unsigned long		alarmType;
	unsigned long		alarmIndex;
	unsigned long		alarmLong;
	unsigned long		res[10];
}ALARM_HEAD;

typedef struct _DUPLEX_CHN
{
	unsigned long		nOpenChannel:8;	
	unsigned long		nSubChannel:8;	
	unsigned long		res:16;	
}DUPLEX_CHN;

typedef struct _DUPLEX_PIC
{
	unsigned long		nOpenChannel:8;	
	unsigned long		nShootType:8;	
	unsigned long		res:16;	
}DUPLEX_PIC;

extern	SERVER_INFO			g_server_info;

extern  CLIENT_INFO_HEAD	g_user_info;

extern	CHANNEL_STREAM		g_channel_stream[SER_MAX_CHANNEL];




#endif


























