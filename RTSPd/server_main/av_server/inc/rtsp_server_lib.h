#ifndef __RTSPLIB_H_
#define __RTSPLIB_H_
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <inttypes.h>
#include <sys/time.h>
#endif
#include "rtsp_server.h"

typedef  void BackgroundHandlerProc(void* clientData, int Mask);
typedef  enum StreamMode StreamMode;
typedef  struct _user_auth UserAuth;
typedef  struct _Handler_Descriptor HandlerDescriptor;
typedef  struct _Handler_Descriptor *  PHandlerDescriptor;
typedef  struct _client_session ClientSession;
typedef  struct _rtsp_server RtspServer;
typedef  struct _rtsp_av_attr Rtsp_av_attr;
typedef  struct _rtsp_env RtspEnv;
typedef  struct _ENC_FRAME_HEAD ENC_FRAME_HEAD;
typedef  struct _EXT_FRAME_VIDEO EXT_FRAME_VIDEO;
typedef  struct _EXT_FRAME_AUDIO EXT_FRAME_AUDIO;
typedef  union _EXT_FRAME_TYPE EXT_FRAME_TYPE;
typedef  struct _EXT_FRAME_HEAD EXT_FRAME_HEAD;
typedef  struct  _RTP_header RTP_header;
typedef  struct _RTP_over_tcp_header RTP_over_tcp_header;
typedef  struct tagFUIndicator FUIndicatorDef;
typedef  struct tagFUHeader FUHeaderDef;
typedef  struct tagFU_A FU_ADef;

#define MAX_USER_NUM 3
#define MAX_CLIENT_NUM 64
#define headerSize (sizeof(ENC_FRAME_HEAD)+sizeof(EXT_FRAME_HEAD))

#define HONEYWELL_RTSP 0

enum StreamMode
{
	RTP_TCP,
	RTP_UDP,
	RAW_UDP
};

typedef enum STREAM_TYPE{
	AUDIO_TYPE = 1,
	MEDIA_TYPE,
	MIXED_TYPE
}STREAM_TYPE;


#ifndef PACKED
#define PACKED		__attribute__((packed, aligned(1)))
#endif

struct _user_auth
{	
	char	fUserName[64];
	char	fPassWord[64];
	int		fIsMd5Auth;
}PACKED;

struct _Handler_Descriptor
{
	int						socketNum;//该handle所要处理的socket请求
	BackgroundHandlerProc		*handlerProc;
	void						*clientData;
	struct _Handler_Descriptor	*fNextHandler;
	struct _Handler_Descriptor	*fPrevHandler;
}PACKED;

struct _rtsp_server;
//rtsp会话上下文
struct _client_session
{
	struct _rtsp_server	*ourServer;
	//用于音视频进行RTP推送的缓冲区
	char			 		*pVideoBuf;
	//最大视频帧长度
	int					maxVideoLen;
	int 			  		sock;
	int			 		bUse;
	int					bPlay;
	int					bIsActive;
	//RTSP 消息响应是否多播，由 onvif 配置过来
	int					bIsMulticast;
	int			  		bUseMinStream;//用来区分主次码流0 --主 1--次
	//客户请求的通道号，rtsp://192.168.1.188/av0_0 从av0中提取
	//一般本系统只有一个视频通道，因此一般该字段为0
	int			  		nSrcChannel;//源通道号，0--视频流
	int     			Is_Valid;//是否已经对客户进行过授权验证
	//期望得到的下一帧用于RTP推送的视频帧号
	unsigned			lastVideoFrameNo;
	
	//要读取的下一视频帧在内存池中的起始内存块号，也即搜索起点
	//即下一次搜索的起始位置
	unsigned			lastVideoPos;	
	//上一次接收到准备发送的视频帧的时间戳
	unsigned			lastTimeStamp;
	//期望得到的下一音频帧号
	unsigned			lastAudioFrameNo;
	//下一次音频帧搜索的起始位置
	unsigned			lastAudioPos;
	unsigned			lastAudioTimeStamp;//added by mazhidong
	//最近发送的音视频帧的Pts时间0--视频 1--音频
	unsigned			lastPts[2];
	struct timeval 		nextGetFram;
	struct timeval		rtcpKeepAliveTime;
	int					bKeepAliveSend;
	//是否是第一视频帧
	int					bFirst;
	//是否是第一音频帧
	int					bAudioFirst;
	int                 needSynAudio;
	int					bFile;
	//文件描述符检索的是回放流，而不是实时流
	FILE				*fd;
	char				fileName[64];
	int					bFileEnd;
	//流传输模式，比如RTP封装的TCP 还是UDP，或者是裸的UDP传输模式
	StreamMode 			streamingMode;
	//客户会话的媒体流编号，可能存在多个媒体流传送的情形
	//比如音视频流
	int					mediaNum;
	//码流的describle消息中的control字段
	//用于检索特定通道特定流(主次)的音视频流
	//trackId[0]--视频流后缀一般为0 trackId[1]--音频后缀 一般为1
	int					trackId[2];
	//比如"RAW/RAW/UDP"
	char 				streamingModeString[64];
	//指定目的地址接受
	char 				destinationAddressStr[64];
	//指定目的端口接收
	unsigned int		destinationPort;
	unsigned int		As[2];
	unsigned int 		destinationTTL;
	unsigned int		cseq;
	//保存视频，音频的RTP,RTCP的端口号
	unsigned short 		clientRTPPortNum[2];
	unsigned short 		clientRTCPPortNum[2];
	unsigned short 		serverRTPPortNum[2];
	unsigned short 		serverRTCPPortNum[2];
	//传输RTC,RTCP流的采用的tcp socket，一般是复用RTSP会话的
	int					tcpSocket;
	//音视频流的rtp流的UDP socket
	int					rtpSocket[2];
	//音视频流的rtcp流的UDP socket
	int					rtcpSocket[2];
	//对应TCP的交织传输通道号0--视频1--音频
	unsigned int 		rtpChannelId[2];
	unsigned int 		rtcpChannelId[2];
	unsigned long long  session;
	//RTP流的源端标识符信息，是个随机数
	unsigned int       	ssrc[2];
	unsigned short     	seq[2];
	//最近发送的音视频的RTP时间0--视频1--音频(由pts时间换算过来)
	unsigned int       	rtptime[2];	
	//此字段应该为后缀信息，
	//比如 rtsp://192.168.1.154:554/av0_0 streamName为:av0_0
	char				streamName[32];
	//保存Base64编码的sps,pps帧信息
	char				sps_pps[256];
	char				profilelevelid[32];
	//应答消息缓冲区
	char				sendBuf[4096];
	//接受客户端rtsp请求缓冲区
	char				recvBuf[4096];	
	int                      HttpFlag;//是否采用rtsp over http的传送方式  
	int			 HttpStartFlag;
	int			 Http_Error_Flag;
	pthread_t                tid;
	int			 Rtcp_Accept_Flag;
	//当前操作的媒体流类型 比如:AUDIO_TYPE
	STREAM_TYPE        media_stream_type;
	//rtsp请求多播传送RTP流
	int				   bSCZRequestMulticast;
	unsigned int       nMultiClientNo;
	int				   bForMultiKeep;
	int				   bDecCountNum;
	//是否成功接收到Play消息
	int				   bPlaySuccess;
	int				   bKilled; //组播序号为1的SESSION是否被删除过
	long			   lLastHBTime; //上次心跳时间
	int				   nKeepPlayCount; //统计2次才超时
	int				   nSkipFrameNum; //统计获取码流失败的计数
	//获取不到媒体数据的次数
	int				   nZeroFrameNum;
	long			   lRtpSendTime;//上次码流发送时间
	int				   nKeepHBCount;
	int				   nKeepRTPCount;

	unsigned int		getfaultframe;	// 获取帧错误统计计数

	unsigned            isHaveNoVideo;
	unsigned            isHaveNoAudio;
}PACKED;


struct _rtsp_env;
struct _rtsp_server
{
	struct _rtsp_env	*fOurEnv;
	//554
	int					rtspPort;//rtsp端口554
	int					rtspSocket;//
	int					bPasvMode;
	volatile int		clientNum;
	int					fIsUserAuth;
	UserAuth			fUserAuth;
	//rtsp会话队列
	ClientSession		client[MAX_CLIENT_NUM];
	//客户socket读请求监听队列
	HandlerDescriptor	*readHanderHead;
	//当前请求
	HandlerDescriptor	*readHander;
	//客户socket写请求监听队列头节点，一般是个空结点
	HandlerDescriptor	*writeHanderHead;
	//当前正在进行的写操作
	HandlerDescriptor	*writeHander;

	int					bMulticast;		//单、组播
	char				szMulticastIP[16];	//多播IP，主机字节序
	//所有会话主次码流视频端口
	unsigned short		nVideoPort[2];	//多播端口，视频，主码流
	unsigned short		nAudioPort[2];	//多播端口，音频，主码流
	
	volatile unsigned int		nMultiClientNum[2];

	//是否有客户申请主码流数据
	int isGetMainStream;
	//是否有客户申请次码流数据
	int isGetSubStream;
}PACKED;

#define MAX_PATH_LEN 256
//存放RTSP音视频属性
//这些信息来自于配置SysConfig.sysVideo[nCh]
struct _rtsp_av_attr
{
	int				audioSampleRate;//音频采样速率
	int				bAudioOpen;//音频是否打开
	//音频净荷类型G.711 为8
	int				audioPt;
	//视频净荷类型H264的为96
	int				videoPt;
	//比如"PCMA"  ---G.711A
	char			audioCodec[16];//音频编码类型
	//比如H264
	char			videoCodec[16];//视频编码类型
};

struct _rtsp_env
{
	RtspServer  	*rtspSever;
	//来自客户端的socket请求数，不包括554端口的监听socket
	int				socketNum;
	int				fMaxNumSockets;
	fd_set			fReadSet;
	int				bPassive;
	int				mtu;
	int 			maxChn;
	int				fmaxDelayTime;
	pthread_t 		rtspServerThread;
	pthread_t 		rtspWriteHandleThread;
	Rtsp_av_attr	*AvAttr;
	UrlAnalysis		*pf_urlAnalysis;
	char			filePath[256];
	pthread_t		Exitting_Tid;
	pthread_t		checkRtspPlayThread; //add by yangjie
	int				bTimeFlag;

	pthread_mutex_t	muRtspNtp;
}PACKED;

#define DEBUG printf("line%d\n",__LINE__);

#ifndef ENC_STRUCT_DEFINITION
#define ENC_STRUCT_DEFINITION
struct _ENC_FRAME_HEAD
{
	unsigned short	zeroFlag;
	unsigned char   oneFlag;
	unsigned char	streamFlag;

	unsigned long	nByteNum;
	unsigned long	nTimestamp;
}PACKED;

//========================================================================
//鎵╁睍甯уご
//========================================================================
#define	GF_EXT_HEAD_FLAG	0x06070809
#define	GF_EXT_TAIL_FLAG	0x0a0b0c0d

//瑙嗛鍙傛暟
struct _EXT_FRAME_VIDEO
{
	unsigned short	nVideoEncodeType;	//瑙嗛缂栫爜绠楁硶
	unsigned short	nVideoWidth;		//瑙嗛鍥惧儚瀹?
	unsigned short	nVideoHeight;		//瑙嗛鍥惧儚楂?
	unsigned char   nPal;               //鍒跺紡
	unsigned char   bTwoFeild;			//鏄惁鏄袱鍦虹紪鐮?濡傛灉鏄袱鍦虹紪鐮侊紝PC绔渶瑕佸仛deinterlace)
	unsigned char   nFrameRate;			//甯х巼
	unsigned char   bMinStream;			//主码流为0  次码流为1 //ljm 2010-03-24
	unsigned char   szReserve[7];		//20090305 CYG(2)
}PACKED;

//闊抽鍙傛暟
struct _EXT_FRAME_AUDIO
{
	unsigned short	nAudioEncodeType;	//闊抽缂栫爜绠楁硶
	unsigned short	nAudioChannels;		//閫氶亾鏁?
	unsigned short	nAudioBits;			//浣嶆暟
	unsigned char   szReserve[2];
	unsigned long	nAudioSamples;		//閲囨牱鐜?	
	unsigned long	nAudioBitrate;		//闊抽缂栫爜鐮佺巼
} PACKED;

union _EXT_FRAME_TYPE
{
	EXT_FRAME_VIDEO	szFrameVideo;
	EXT_FRAME_AUDIO	szFrameAudio;
} ;

struct _EXT_FRAME_HEAD
{
	unsigned long	nStartFlag;			//鎵╁睍甯уご璧峰鏍囪瘑
	unsigned short	nVer;				//鐗堟湰
	unsigned short	nLength;			//鎵╁睍甯уご闀垮害
	EXT_FRAME_TYPE	szFrameInfo;		
	unsigned long   nTimestamp;			//浠ユ绉掍负鍗曚綅鐨勬椂闂存埑
	unsigned long	nEndFlag;			//鎵╁睍甯уご璧峰鏍囪瘑
}PACKED;
#endif


#define MAX_DELAY  (3*1000)
#define RTP_VERSION		2		/* RTP鍖呭ご鐗堟湰 */
#define G726_TIME_FREQUENCY		8000	/* 棰戠巼 */
#define H264_TIME_FREQUENCY     90

#define TEST 0

struct  _RTP_header 
{
	unsigned char csrc_len:4;   /* expect 0 */
	unsigned char extension:1;  /* expect 1, see RTP_OP below */
	unsigned char padding:1;	/* expect 0 */
	unsigned char version:2;	/* expect 2 */

	unsigned char payload:7;	/* RTP_PAYLOAD_RTSP */
	unsigned char marker:1;		/* expect 1 */
	/* bytes 2, 3 */
	unsigned short seq_no;		/* 搴忓垪鍙?*/
	/* bytes 4-7 */
	unsigned int timestamp;		/* 鏃堕棿鎴?*/
	/* bytes 8-11 */
	unsigned int ssrc;			/* stream number is used here. */
}PACKED;

/* [[NEW_LIB_ROSSI]] NEW_ADD_SDCARD rossi for align(1) packed */
struct _RTP_over_tcp_header
{
	unsigned char  dollar;
	unsigned char  channelId;
	unsigned short packetSize;
}PACKED;

/**************************************************************************
  绫诲瀷:缁撴瀯浣?
  鍚嶇О:FUIndicatorDef
  浣滅敤:FU绫诲瀷绗簩瀛楄妭鐨勬寚绀?
 ***************************************************************************/
struct tagFUIndicator
{
	char TYPE : 5;		/* set to 28 or 29 */
	char NRI : 2;		/* the same to nal */
	char F : 1; 		/* set to 0 */
}PACKED;


/**************************************************************************
  绫诲瀷:缁撴瀯浣?
  鍚嶇О:FUHeaderDef
  浣滅敤:FU绗竴瀛楄妭澶寸粨鏋?
 ***************************************************************************/
struct tagFUHeader
{
	char Type : 5;		/* set to nal type */
	char R : 1; 		/* reserve */
	char E : 1; 		/* end flag */
	char S : 1; 		/* start flag */
}PACKED;

/**************************************************************************
  绫诲瀷:缁撴瀯浣?
  鍚嶇О:FU_ADef
  浣滅敤:FU澶寸粨鏋?
 ***************************************************************************/
struct tagFU_A
{
	FUIndicatorDef stFUIndicator;
	FUHeaderDef stFUHeader;
}PACKED;

extern char const* const libServer;
extern const uint8_t map2[];
extern RtspEnv g_Env;
#endif
