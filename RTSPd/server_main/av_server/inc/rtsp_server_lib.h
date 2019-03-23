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
	int						socketNum;//��handle��Ҫ�����socket����
	BackgroundHandlerProc		*handlerProc;
	void						*clientData;
	struct _Handler_Descriptor	*fNextHandler;
	struct _Handler_Descriptor	*fPrevHandler;
}PACKED;

struct _rtsp_server;
//rtsp�Ự������
struct _client_session
{
	struct _rtsp_server	*ourServer;
	//��������Ƶ����RTP���͵Ļ�����
	char			 		*pVideoBuf;
	//�����Ƶ֡����
	int					maxVideoLen;
	int 			  		sock;
	int			 		bUse;
	int					bPlay;
	int					bIsActive;
	//RTSP ��Ϣ��Ӧ�Ƿ�ಥ���� onvif ���ù���
	int					bIsMulticast;
	int			  		bUseMinStream;//����������������0 --�� 1--��
	//�ͻ������ͨ���ţ�rtsp://192.168.1.188/av0_0 ��av0����ȡ
	//һ�㱾ϵͳֻ��һ����Ƶͨ�������һ����ֶ�Ϊ0
	int			  		nSrcChannel;//Դͨ���ţ�0--��Ƶ��
	int     			Is_Valid;//�Ƿ��Ѿ��Կͻ����й���Ȩ��֤
	//�����õ�����һ֡����RTP���͵���Ƶ֡��
	unsigned			lastVideoFrameNo;
	
	//Ҫ��ȡ����һ��Ƶ֡���ڴ���е���ʼ�ڴ��ţ�Ҳ���������
	//����һ����������ʼλ��
	unsigned			lastVideoPos;	
	//��һ�ν��յ�׼�����͵���Ƶ֡��ʱ���
	unsigned			lastTimeStamp;
	//�����õ�����һ��Ƶ֡��
	unsigned			lastAudioFrameNo;
	//��һ����Ƶ֡��������ʼλ��
	unsigned			lastAudioPos;
	unsigned			lastAudioTimeStamp;//added by mazhidong
	//������͵�����Ƶ֡��Ptsʱ��0--��Ƶ 1--��Ƶ
	unsigned			lastPts[2];
	struct timeval 		nextGetFram;
	struct timeval		rtcpKeepAliveTime;
	int					bKeepAliveSend;
	//�Ƿ��ǵ�һ��Ƶ֡
	int					bFirst;
	//�Ƿ��ǵ�һ��Ƶ֡
	int					bAudioFirst;
	int                 needSynAudio;
	int					bFile;
	//�ļ��������������ǻط�����������ʵʱ��
	FILE				*fd;
	char				fileName[64];
	int					bFileEnd;
	//������ģʽ������RTP��װ��TCP ����UDP�����������UDP����ģʽ
	StreamMode 			streamingMode;
	//�ͻ��Ự��ý������ţ����ܴ��ڶ��ý�������͵�����
	//��������Ƶ��
	int					mediaNum;
	//������describle��Ϣ�е�control�ֶ�
	//���ڼ����ض�ͨ���ض���(����)������Ƶ��
	//trackId[0]--��Ƶ����׺һ��Ϊ0 trackId[1]--��Ƶ��׺ һ��Ϊ1
	int					trackId[2];
	//����"RAW/RAW/UDP"
	char 				streamingModeString[64];
	//ָ��Ŀ�ĵ�ַ����
	char 				destinationAddressStr[64];
	//ָ��Ŀ�Ķ˿ڽ���
	unsigned int		destinationPort;
	unsigned int		As[2];
	unsigned int 		destinationTTL;
	unsigned int		cseq;
	//������Ƶ����Ƶ��RTP,RTCP�Ķ˿ں�
	unsigned short 		clientRTPPortNum[2];
	unsigned short 		clientRTCPPortNum[2];
	unsigned short 		serverRTPPortNum[2];
	unsigned short 		serverRTCPPortNum[2];
	//����RTC,RTCP���Ĳ��õ�tcp socket��һ���Ǹ���RTSP�Ự��
	int					tcpSocket;
	//����Ƶ����rtp����UDP socket
	int					rtpSocket[2];
	//����Ƶ����rtcp����UDP socket
	int					rtcpSocket[2];
	//��ӦTCP�Ľ�֯����ͨ����0--��Ƶ1--��Ƶ
	unsigned int 		rtpChannelId[2];
	unsigned int 		rtcpChannelId[2];
	unsigned long long  session;
	//RTP����Դ�˱�ʶ����Ϣ���Ǹ������
	unsigned int       	ssrc[2];
	unsigned short     	seq[2];
	//������͵�����Ƶ��RTPʱ��0--��Ƶ1--��Ƶ(��ptsʱ�任�����)
	unsigned int       	rtptime[2];	
	//���ֶ�Ӧ��Ϊ��׺��Ϣ��
	//���� rtsp://192.168.1.154:554/av0_0 streamNameΪ:av0_0
	char				streamName[32];
	//����Base64�����sps,pps֡��Ϣ
	char				sps_pps[256];
	char				profilelevelid[32];
	//Ӧ����Ϣ������
	char				sendBuf[4096];
	//���ܿͻ���rtsp���󻺳���
	char				recvBuf[4096];	
	int                      HttpFlag;//�Ƿ����rtsp over http�Ĵ��ͷ�ʽ  
	int			 HttpStartFlag;
	int			 Http_Error_Flag;
	pthread_t                tid;
	int			 Rtcp_Accept_Flag;
	//��ǰ������ý�������� ����:AUDIO_TYPE
	STREAM_TYPE        media_stream_type;
	//rtsp����ಥ����RTP��
	int				   bSCZRequestMulticast;
	unsigned int       nMultiClientNo;
	int				   bForMultiKeep;
	int				   bDecCountNum;
	//�Ƿ�ɹ����յ�Play��Ϣ
	int				   bPlaySuccess;
	int				   bKilled; //�鲥���Ϊ1��SESSION�Ƿ�ɾ����
	long			   lLastHBTime; //�ϴ�����ʱ��
	int				   nKeepPlayCount; //ͳ��2�βų�ʱ
	int				   nSkipFrameNum; //ͳ�ƻ�ȡ����ʧ�ܵļ���
	//��ȡ����ý�����ݵĴ���
	int				   nZeroFrameNum;
	long			   lRtpSendTime;//�ϴ���������ʱ��
	int				   nKeepHBCount;
	int				   nKeepRTPCount;

	unsigned int		getfaultframe;	// ��ȡ֡����ͳ�Ƽ���

	unsigned            isHaveNoVideo;
	unsigned            isHaveNoAudio;
}PACKED;


struct _rtsp_env;
struct _rtsp_server
{
	struct _rtsp_env	*fOurEnv;
	//554
	int					rtspPort;//rtsp�˿�554
	int					rtspSocket;//
	int					bPasvMode;
	volatile int		clientNum;
	int					fIsUserAuth;
	UserAuth			fUserAuth;
	//rtsp�Ự����
	ClientSession		client[MAX_CLIENT_NUM];
	//�ͻ�socket�������������
	HandlerDescriptor	*readHanderHead;
	//��ǰ����
	HandlerDescriptor	*readHander;
	//�ͻ�socketд�����������ͷ�ڵ㣬һ���Ǹ��ս��
	HandlerDescriptor	*writeHanderHead;
	//��ǰ���ڽ��е�д����
	HandlerDescriptor	*writeHander;

	int					bMulticast;		//�����鲥
	char				szMulticastIP[16];	//�ಥIP�������ֽ���
	//���лỰ����������Ƶ�˿�
	unsigned short		nVideoPort[2];	//�ಥ�˿ڣ���Ƶ��������
	unsigned short		nAudioPort[2];	//�ಥ�˿ڣ���Ƶ��������
	
	volatile unsigned int		nMultiClientNum[2];

	//�Ƿ��пͻ���������������
	int isGetMainStream;
	//�Ƿ��пͻ��������������
	int isGetSubStream;
}PACKED;

#define MAX_PATH_LEN 256
//���RTSP����Ƶ����
//��Щ��Ϣ����������SysConfig.sysVideo[nCh]
struct _rtsp_av_attr
{
	int				audioSampleRate;//��Ƶ��������
	int				bAudioOpen;//��Ƶ�Ƿ��
	//��Ƶ��������G.711 Ϊ8
	int				audioPt;
	//��Ƶ��������H264��Ϊ96
	int				videoPt;
	//����"PCMA"  ---G.711A
	char			audioCodec[16];//��Ƶ��������
	//����H264
	char			videoCodec[16];//��Ƶ��������
};

struct _rtsp_env
{
	RtspServer  	*rtspSever;
	//���Կͻ��˵�socket��������������554�˿ڵļ���socket
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
//扩展帧头
//========================================================================
#define	GF_EXT_HEAD_FLAG	0x06070809
#define	GF_EXT_TAIL_FLAG	0x0a0b0c0d

//视频参数
struct _EXT_FRAME_VIDEO
{
	unsigned short	nVideoEncodeType;	//视频编码算法
	unsigned short	nVideoWidth;		//视频图像�?
	unsigned short	nVideoHeight;		//视频图像�?
	unsigned char   nPal;               //制式
	unsigned char   bTwoFeild;			//是否是两场编�?如果是两场编码，PC端需要做deinterlace)
	unsigned char   nFrameRate;			//帧率
	unsigned char   bMinStream;			//������Ϊ0  ������Ϊ1 //ljm 2010-03-24
	unsigned char   szReserve[7];		//20090305 CYG(2)
}PACKED;

//音频参数
struct _EXT_FRAME_AUDIO
{
	unsigned short	nAudioEncodeType;	//音频编码算法
	unsigned short	nAudioChannels;		//通道�?
	unsigned short	nAudioBits;			//位数
	unsigned char   szReserve[2];
	unsigned long	nAudioSamples;		//采样�?	
	unsigned long	nAudioBitrate;		//音频编码码率
} PACKED;

union _EXT_FRAME_TYPE
{
	EXT_FRAME_VIDEO	szFrameVideo;
	EXT_FRAME_AUDIO	szFrameAudio;
} ;

struct _EXT_FRAME_HEAD
{
	unsigned long	nStartFlag;			//扩展帧头起始标识
	unsigned short	nVer;				//版本
	unsigned short	nLength;			//扩展帧头长度
	EXT_FRAME_TYPE	szFrameInfo;		
	unsigned long   nTimestamp;			//以毫秒为单位的时间戳
	unsigned long	nEndFlag;			//扩展帧头起始标识
}PACKED;
#endif


#define MAX_DELAY  (3*1000)
#define RTP_VERSION		2		/* RTP包头版本 */
#define G726_TIME_FREQUENCY		8000	/* 频率 */
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
	unsigned short seq_no;		/* 序列�?*/
	/* bytes 4-7 */
	unsigned int timestamp;		/* 时间�?*/
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
  类型:结构�?
  名称:FUIndicatorDef
  作用:FU类型第二字节的指�?
 ***************************************************************************/
struct tagFUIndicator
{
	char TYPE : 5;		/* set to 28 or 29 */
	char NRI : 2;		/* the same to nal */
	char F : 1; 		/* set to 0 */
}PACKED;


/**************************************************************************
  类型:结构�?
  名称:FUHeaderDef
  作用:FU第一字节头结�?
 ***************************************************************************/
struct tagFUHeader
{
	char Type : 5;		/* set to nal type */
	char R : 1; 		/* reserve */
	char E : 1; 		/* end flag */
	char S : 1; 		/* start flag */
}PACKED;

/**************************************************************************
  类型:结构�?
  名称:FU_ADef
  作用:FU头结�?
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
