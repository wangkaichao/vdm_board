/******************************************************************************
* 版权信息：
* 系统名称：
* 文件名称：GFNetComm.h
* 文件说明：该文件定义了和客户端交互的数据结构和宏定义
******************************************************************************/
#ifndef GF_NET_COMM_H_
#define GF_NET_COMM_H_

//#include "GFNet.h"
//#include "../global.h"

#define   PORTNUM			4       //路数

#define SER_MAX_CHANNEL   	4

#define GF_MAGIC_FLAG		GF_GetMagicFlag()

#define	UNITE_PIC_FLAG		0x01020304
#define		AUDIO_SND_COUNT    5
//20090305 CYG
//#define GF_MAX_FRAME_LEN		(640 * 1024) //(256*1024)
//#define GF_MAX_PIC_LEN		(648 * 1024)

#define COMM_BUFFER_SIZE		12270	 //5102
#define NET_PACKET_VIDEO_SIZE	1024
#define NET_PACKET_AUDIO_SIZE	1344     //ljm modify 2010-04-13 
#define NET_TALK_AUDIO_LEN		132
#define	NET_TALK_AUDIO_G726		640		 //2009.7.8 CYG 204
#define NET_TALK_AUDIO_G711     1344     //ljm 2010-04-12 add 
#define NET_PACKET_UPDATE_SIZE	1024
#define	NET_TRNAS_PIECE_SIZE	512

#define SEND_FILE_END_FLAG		0xffff

#define MULTI_COMMAND_ADDR		"234.200.200.200"
#define MULTI_FIND_PORT			59124
#define UDP_FIND_PORT			59123

#define 		USER_NAME_LENGTH		16
#define 		USER_PSW_LENGTH			16

typedef enum _SOCK_TYPE
{
	SOCK_LOGON = 0,
	SOCK_DATA = 1,
	SOCK_TALK = 2,
	SOCK_FILE = 3,
	SOCK_QUERY = 4,
	SOCK_PIC = 5,
}SOCK_TYPE;

typedef enum _PROTOCOL_TYPE
{
    PROTOCOL_TCP = 0,
	PROTOCOL_UDP = 1,
	PROTOCOL_MULTI = 2
}PROTOCOL_TYPE;

typedef enum _GFNET_CMD_TYPE
{
	GFNET_LOGON_SERVER,
	GFNET_KEEP_ALIVE,
	GFNET_UPDATE_FILE,
	GFNET_TALKBACK,
	GFNET_TALKBACK_STOP,
	GFNET_SEARCH_SERVER,
	GFNET_CONFIG_SERVER,
	GFNET_MODIFY_MAC,
	GFNET_BURN_MACADDR,
	GFNET_ALARM_STATUS,
	GFNET_OPEN_MULTICHANNEL,
	GFNET_CLOSE_MULTICHANNEL,
	GFNET_CLOSE_UDPCHANNEL,
	GFNET_BROAD_SELF,
	GFNET_SEND_SELF,
	//
	GFNET_CONNECT_CENTER,
	GFNET_LOGON_CENTER,
	GFNET_PREVIEW_CENTER,
	GFNET_TALK_CENTER,
	GFNET_BROADCAST_CENTER,
	GFNET_VIEWPUSH_CENTER,
	//
	GFNET_SENSOR_PIC,
	GFNET_COMM_DATA,
	//
	GFNET_CS_PREVIEW_CLOSE,

	//2007.6.13
	GFNET_FILE_OPEN,	//fopen
	GFNET_FILE_CLOSE,	//fclose
	GFNET_FILE_READ,	//fread  25
	GFNET_FILE_WRITE,	//fwrite
	GFNET_FILE_SEEK,    //fseek
	GFNET_FILE_TELL,    //ftell
	GFNET_FILE_EOF ,    //feof

	GFNET_REC_STOP_SEARCH,  //
	GFNET_REC_SEARCH_NVS,
	GFNET_REC_SEARCH_FILE,

	GFNET_READ_FIRST_FRAME, // 读?象文?
	GFNET_READ_NEXT_FRAME,  // 读?象文?	
	GFNET_REC_SEARCH_FILE_NEW=50,
	GFNET_FILE_OPEN_NEW,
	GFNET_FILE_SEEK_NEW,
	GFNET_FILE_SPEED,
	GFNET_FILE_STEP,
	GFNET_NETREPLAY_STREAM,
	GFNET_NETREPLAY_DATA,
	GFNET_NETREPLAY_END,
	GFNET_LOGON_SERVER2,
        
 	GFNET_ALARM_LOST = 100,
	GFNET_ALARM_LOST_NEW,	//添加
	GFNET_ALARM_STATUS_NEW,
	GFNET_REQUEST_PIC,
	GFNET_GPS_STATUS,
	GFNET_MAX_CMD,
}GFNET_CMD_TYPE;

/*
typedef enum _GFNET_ERR_CODE
{
	ERR_SUCCESS,
	ERR_FAILURE,
	ERR_USER_NOT_EXIST,
	ERR_USER_IS_FULL,
	ERR_VIEW_IS_FULL,
	ERR_TASK_IS_FULL,
	ERR_CHANNEL_NOT_EXIST,
	ERR_DEVICE_NOT_EXIST,
	ERR_IS_TALKING,
	ERR_QUEUE_FAILE,
	ERR_DEVICE_NAME,
}GFNET_ERR_CODE;


*/

typedef struct _SEARCH_NVS
{
	unsigned long flag;
	unsigned long cbSize;
	unsigned long version;
	unsigned long multiip;
	unsigned long multiport;
}SEARCH_NVS;
//视频音频参数
typedef struct _AV_PAR
{
    //video parameter
	unsigned long	nVideoEncodeType;
    unsigned long   nVideoHeight;
    unsigned long   nVideoWidth;
    //audio parameter
	unsigned long   nAudioEncodeType;
    unsigned long   nAudioChannels;
	//unsigned long nAudioBits;
	unsigned short	nAudioBits;
    unsigned short  nAudioBitrate;			//音频码率(k为单位)		2008.12.10 CYG
    unsigned long   nAudioSamples;
}AV_PAR,*PAV_PAR;

typedef struct _ACCEPT_HEAD
{
	//comm head
	unsigned long	nFlag;
	unsigned long	nSockType;
	unsigned long   nMisc;
	char			szUserName[USER_NAME_LENGTH+1];
	char			szPassword[USER_PSW_LENGTH+1];
	char			szDeviceName[60];
	unsigned long	pcID;
}ACCEPT_HEAD,*PACCEPT_HEAD;

typedef struct _COMM_HEAD
{
	unsigned long	nFlag;
	GFNET_CMD_TYPE	nCommand;
	int				nLogonID;
	int				nPriority;
	int				nMisc;
	unsigned long	nErrorCode;
	unsigned long	nBufSize;
}COMM_HEAD,*PCOMM_HEAD;


typedef struct _COMM_BUFFER
{
	COMM_HEAD		commHead;
	char			commBuf[COMM_BUFFER_SIZE];
}COMM_BUFFER;


typedef struct _GF_FRAME_HEAD
{
	unsigned short	zeroFlag;
	unsigned char   oneFlag;
	unsigned char	streamFlag;
	
	unsigned long	nByteNum;
	unsigned long	nTimestamp;
}GF_FRAME_HEAD;

typedef struct _NET_PACKET_HEAD
{
	unsigned long	nNetFlag;
	//视频帧帧号
	unsigned long	nFrameNo;
	//视频单元在整个视频分组中的单元索引
	int				nPakcetNo;
	//视频帧所包含的总的单元数
	int				nPakcetCount;
	//记录分割前的分组头，分组的每个单元都一样
	GF_FRAME_HEAD	frameHead;
	//本单元的数据尺寸
	int				nBufferSize;
}NET_PACKET_HEAD;

typedef struct _GF_PIC_HEAD
{	
	unsigned char	picZero;
	unsigned char   picType;
	unsigned short  picSize;

	unsigned short	picWidth;
	unsigned short	picHeight;
	
	unsigned short	nSendPos;
	unsigned short	res;
}GF_PIC_HEAD;

typedef struct _NET_PIC_HEAD
{
	unsigned long	nNetFlag;
	unsigned long	nFrameNo;
	int				nPakcetNo;
	int				nPakcetCount;
	GF_PIC_HEAD		frameHead;
	int				nBufferSize;
}NET_PIC_HEAD;

typedef struct _NET_PACKET_VIDEO
{
	NET_PACKET_HEAD	packetHead;
	unsigned char	packetData[NET_PACKET_VIDEO_SIZE];
}NET_PACKET_VIDEO;
typedef struct _NET_PACKET_AUDIO
{
	NET_PACKET_HEAD	packetHead;
	unsigned char	packetData[NET_PACKET_AUDIO_SIZE];
}NET_PACKET_AUDIO;

typedef struct _NET_PACKET_PIC
{
	NET_PACKET_HEAD	packetHead;
	unsigned char	packetData[NET_PACKET_VIDEO_SIZE];
}NET_PACKET_PIC;

typedef struct _LOGON_SERVER_INFO
{
	unsigned long   version;
	char			szDevieName[128];
	unsigned long	nDeviceID;
	unsigned long	nLanguageNo;
	unsigned long	nMachineType;
	int				bPalStandard;//BOOL TO int
	unsigned long   nMutiAddr;
	unsigned long   nMutiPort;
	AV_PAR			avInfo[SER_MAX_CHANNEL];
}LOGON_SERVER_INFO;

typedef struct _LOGON_TALK_INFO
{
	unsigned long   version;
	char			szDevieName[128];
	unsigned long	nMachineType;

	unsigned long	nTalkAudioEncodeType;
    unsigned long	nTalkAudioChannels;
    unsigned long	nTalkAudioBits;
    unsigned long	nTalkAudioSamples;
}LOGON_TALK_INFO;

typedef struct _SEARCH_SER_INFO
{ 
	char				userName[USER_NAME_LENGTH+1];
	char				userPassword[USER_PSW_LENGTH+1];
	int             	nDeviceType;
	char				szDeviceName[64];
	unsigned long		ipLocal;
	unsigned char		macAddr[6];
	unsigned short		wPortWeb;
	unsigned short		wPortListen;
	unsigned long		ipSubMask;
	unsigned long		ipGateway;
	unsigned long		ipMultiAddr;
	unsigned long		ipDnsAddr;
	unsigned short		wMultiPort;
	int					nChannelNum;
}SEARCH_SER_INFO;

typedef struct _MULTI_OPEN_CHANNEL
{
	unsigned long dwServerIP;
	unsigned long nServerPort;
	unsigned long dwLocalIP;
	char szDeviceName[60];
	unsigned long pcID;
	char szUserName[USER_NAME_LENGTH+1];
	char szUserPassword[USER_PSW_LENGTH+1];
	int	 channelNo;
}MULTI_OPEN_CHANNEL;


int GF_PUB_GetMaxFrameSize();
int GF_VENC_RequestIFrame(unsigned char nCh, unsigned char nMinEnc, unsigned char nIFrameCount);

#endif




























