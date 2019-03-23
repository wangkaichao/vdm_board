

#include "init_system.h"

#include "GFNetStruct.h"

/*rtsp server*/
#include "rtsp_server.h"
#include "rtsp_server_lib.h"

SERVER_INFO			g_server_info;
CLIENT_INFO_HEAD	g_user_info;
CHANNEL_STREAM		g_channel_stream[SER_MAX_CHANNEL];
unsigned long       g_pts_interval[SER_MAX_CHANNEL][2]; //统计缓存中pts间隔


static	int	g_bNetServiceStart = FALSE;

static unsigned int gNetMagicFlag = 0x4567;

unsigned long GF_GetMagicFlag()
{
	return gNetMagicFlag;
}


int GF_PUB_GetMaxFrameSize()
{
	return (1024 * 1024 * 28 / 10);		// 2.8M
}


int GF_Modify_Sys_KeepAlive()
{
	printf("Modify System KeepAlive Start....\n");
	system("echo 60 > /proc/sys/net/ipv4/tcp_keepalive_time");
	system("echo 10 > /proc/sys/net/ipv4/tcp_keepalive_intvl");
	system("echo 5 > /proc/sys/net/ipv4/tcp_keepalive_probes");
	printf("Modify System KeepAlive End....\n");

	return 0;
}


//GFSNet_Init(SER_MAX_CHANNEL, 0, 0xffffffff, 
//            nVBufferCount, 400, NULL, "123",
//            NULL, NULL);
/*
 * 初始化rtsp服务器参数
 * nChannelNum：视频通道号
 * nBasePort：rtsp服务端口
 * nMultiIP：是否多播
 * nVBufferCount：视频缓冲区通道帧数
 * nABufferCount：音频通道帧数
 * pDeviceName：设备名
 * pServerID：服务器ID
 * pNSSNotifyCallback：音视频帧完成通知函数
 */
int GFSNet_Init(int nChannelNum, int nBasePort, unsigned long nMultiIP,
		int nVBufferCount, int nABufferCount, const char *pDeviceName,
		const char *pServerID, NSSNotifyFrame  pNSSNotifyCallback)
{
	int     i = 0;

	printf("GFSNet_Init --> 1\n");

	//修改系统keepalive 值
	GF_Modify_Sys_KeepAlive();

	printf("GFSNet_Init --> 2\n");

	memset(&g_server_info  ,0,sizeof(g_server_info));
	memset(&g_user_info    ,0,sizeof(g_user_info));
	memset(g_channel_stream,0,sizeof(g_channel_stream));

	memset(g_pts_interval  ,0, sizeof(g_pts_interval));
	memset(g_server_info.szUpdateFileName, 0,
			sizeof(g_server_info.szUpdateFileName));

	g_user_info.statusMultiSendChannel = 0;

	if(pDeviceName)
	{
		//strcpy(g_server_info.szDeviceName,pDeviceName);
	}
	if(pServerID)
	{
		strcpy(g_server_info.szServerID, pServerID);
	}
	if(nVBufferCount <= 0)
		nVBufferCount = DEFAULT_POOL_NUM;
	if(nABufferCount <= 0)
		nVBufferCount = DEFAULT_AUDIO_BUF_COUNT;

	g_server_info.bExistServer      = FALSE;
	g_server_info.bStartServer      = FALSE;
	g_server_info.nChannelNum       = nChannelNum;
	g_server_info.nBasePort         = nBasePort;//5000
	g_server_info.multiAddr         = htonl(nMultiIP);
	g_server_info.nVBufferCount     = nVBufferCount;
	g_server_info.nABufferCount     = nABufferCount;
	g_server_info.pUpdateFileBuf    = NULL;
	g_server_info.currentUpdateIP   = 0;
	g_server_info.currentUpdatePort = 0;
	g_server_info.talkArrayIndex    = 0;
	g_server_info.talkLastestIndex  = 0;
	g_server_info.pTalkBuffer       = NULL;
	g_server_info.bFlagTalking      = FALSE;
	g_server_info.talkHaveData      = 0;
	pthread_mutex_init(&g_server_info.mutexMultiWait,      NULL);
	pthread_cond_init(&g_server_info.condMultiWait  ,      NULL);

	g_server_info.pNSSCallback = pNSSNotifyCallback;
	g_server_info.pNSSFrame    = NULL;
	if(pNSSNotifyCallback != NULL)
		g_server_info.pNSSFrame = malloc(GF_PUB_GetMaxFrameSize());

	printf("GFSNet_Init --> 2\n");


	return TRUE;
}

/*
 * rtsp服务初始化，调用init_stream接口完成工作
 */
int GFSNet_Start()
{
	int			  i = 0;
	unsigned long j = 0;	
	pthread_t	 hKeepTask;

	if(g_bNetServiceStart)
		return FALSE;

	//类型为SOCK_STREAM套接字已不在连接时，进程写该套接字产生此信号
	signal(SIGPIPE,SIG_IGN);
	for(i = 0;i < g_server_info.nChannelNum; i++)
	{
		if(!InitStream(i,&g_channel_stream[i]))
		{
			printf(" ******   InitStream  failure *****\n");
			return FALSE;
		}
	}

	for(i = 0;i < SERVER_MAX_LOGON_NUM;i++)
	{
		g_server_info.bMsgClientUsed[i] = FALSE;
		g_server_info.logonAddress[i] = 0;
	}

	for(i = 0;i < ALARM_MSG_BUF;i++)
	{
		for(j = 0;j < SERVER_MAX_LOGON_NUM;j++)
			g_server_info.alarmStatusSend[i][j] = FALSE;
	}

	g_server_info.alarmStatusIndex = 0;
	g_server_info.alarmCommIndex   = 0;
	g_server_info.alarmCapInex     = 0;
	g_server_info.gpsStatusIndex   = 0;

	g_bNetServiceStart         = TRUE;
	g_server_info.bStartServer = TRUE;
	sleep(1);
	return TRUE;
}

/* 停止服务流
  *
  */
void GFSNet_Stop()
{
	int i = 0;
	if(!g_bNetServiceStart)
		return;
	g_server_info.bExistServer = TRUE;
	g_server_info.bStartServer = FALSE;

	sleep(1);

	for(i = 0;i < g_server_info.nChannelNum;i++)
	{
		DestStream(i,&g_channel_stream[i]);
	}

	sleep(1);

	g_bNetServiceStart = FALSE;
}

//获取视频帧数据
//nChnNo：主通道号
//bSubChn：次通道号
//pFrameBuf：视频帧缓冲区
//nFrameBufSize：帧空间大小
//bKeyFrame：是否I帧
//bLostLastestKey：最后一个I帧号
//nVideoFrmNo：视频帧号
//nRealFrmNo：实时帧号
//interMilliSecond：间隔时间戳
//preFrmTime：上一帧时间
//nowFrmTime：需要获取的帧时间戳
//nLastVideoPos：下一帧位置
int	GFSNet_GetVideoFrame(int nChnNo, int bSubChn, char *pFrameBuf,
		int nFrameBufSize, int bKeyFrame, int bLostLastestKey,
		unsigned long nVideoFrmNo, unsigned long *nRealFrmNo,
		unsigned long interMilliSecond, unsigned long preFrmTime,
		unsigned long *nowFrmTime,int *nLastVideoPos)
{
	if(!g_bNetServiceStart)
		return -1;

	return GetOneVideoFrame(nChnNo, bSubChn, pFrameBuf, nFrameBufSize,
			bKeyFrame, bLostLastestKey, nVideoFrmNo,
			nRealFrmNo, interMilliSecond, preFrmTime,
			nowFrmTime, nLastVideoPos);	
}

int GFSNET_GetAudioFrame(int nChnNo,char *pFrameBuf,
		int nFrameBufSize,unsigned long nAudioFrmNo,unsigned long *nTrueFrmNo,
		int *nLastAudioPos,unsigned int gotoPts, unsigned int ptsdiffer)
{
	if(!g_bNetServiceStart)
		return -1;

	return GetOneAudioFrame(nChnNo,NULL,pFrameBuf,nFrameBufSize,
			nAudioFrmNo,nTrueFrmNo,nLastAudioPos,
			gotoPts, ptsdiffer);
}

//填数据到缓冲区
//nCh：主通道号
//nMinEnc：次通道号
//pFrameHeader：帧头缓冲区
//nFrameFlag：是否I帧
//nTimeStamp：时间戳
//nFrameLength：视频帧数据大小
//nMillSecond：间隔时间
int FillFrameHeader(int nCh, int nMinEnc, char *pFrameHeader, 
		unsigned char nFrameFlag, unsigned int nTimeStamp, 
		unsigned int nFrameLength, unsigned int nMillSecond)
{
	EXT_FRAME_HEAD *pExtFrameHeader = (EXT_FRAME_HEAD *)(pFrameHeader + 12);

	/*当前只支持H264*/
	int nEncType = GF_VENC_H264;

	pFrameHeader[0] = 0x00;
	pFrameHeader[1] = 0x00;
	pFrameHeader[2] = 0x01;

	pFrameHeader[3] = nFrameFlag;

	pFrameHeader[4] = (nFrameLength & 0x000000ff);
	pFrameHeader[5] = (nFrameLength & 0x0000ff00)>>8;
	pFrameHeader[6] = (nFrameLength & 0x00ff0000)>>16;
	pFrameHeader[7] = (nFrameLength & 0xff000000)>>24;

	pFrameHeader[8] = (nTimeStamp   & 0x000000ff);
	pFrameHeader[9] = (nTimeStamp   & 0x0000ff00)>>8;
	pFrameHeader[10]= (nTimeStamp   & 0x00ff0000)>>16;
	pFrameHeader[11]= (nTimeStamp   & 0xff000000)>>24;

	pExtFrameHeader->nStartFlag = GF_EXT_HEAD_FLAG;	
	pExtFrameHeader->nVer		= 0x10;	
	pExtFrameHeader->nLength    = sizeof(EXT_FRAME_HEAD);	

	pExtFrameHeader->szFrameInfo.szFrameVideo.nVideoEncodeType = nEncType;
	/*当前写死视频宽高*/
	pExtFrameHeader->szFrameInfo.szFrameVideo.nVideoWidth      = (nMinEnc == 0) ? 1280 : 1920;
	pExtFrameHeader->szFrameInfo.szFrameVideo.nVideoHeight     = (nMinEnc == 0) ? 720 : 1080;
	pExtFrameHeader->szFrameInfo.szFrameVideo.nPal             = 1;/*0: PAL 1: NTSC*/
	pExtFrameHeader->szFrameInfo.szFrameVideo.bTwoFeild        = 0;
	pExtFrameHeader->szFrameInfo.szFrameVideo.bMinStream       = 0;

	pExtFrameHeader->szFrameInfo.szFrameVideo.nFrameRate = 30;

	pExtFrameHeader->nTimestamp = nMillSecond;

	pExtFrameHeader->nEndFlag   = GF_EXT_TAIL_FLAG;	

	return 0;    
}

//写帧数据
unsigned long GFSNet_WriteFrame(int nChannelNo,const char *pFrameData,const char *pAudioData)
{
	if(!g_bNetServiceStart)
		return 0;

	return WriteFrame(nChannelNo,&g_channel_stream[nChannelNo], pFrameData, pAudioData);
}

//设置主次码流参数
//主次码流号
int SetRTSPAttr(int nCh, int bMain)
{
	char szAudioType[32]={0};
	char szVideoType[32]={0};

	strcpy(szVideoType,"H264");

	//    int setAvInfor(int nCh, int bMain, int bAudioOpen, char *audioCodec, int aduioSampleRate, 
	//            int audioPt,char *videoCodec,int videoPt)
	/*
	函数功能：设置rtsp音视频参数
	参数说明：参数意义: http://www.ietf.org/rfc/rfc3551.txt
			nCh：主通道号
			bMain：次通道号
			bAudioOpen：是否开启音频
			audioCodec：音频编码类型
			aduioSampleRate：音频采样率
			audioPt：音频payload值
			videoCodec：视频编码类型
			videoPt：视频payload值
	*/
	setAvInfor(nCh, bMain, 0, szAudioType, 0, 97, szVideoType, 96);

	return 0;
}

static int get_rtsp_ch(const char *urlSuffix)
{
	int i = 0;
	FILE *fp = NULL;
	char string_ling[512] = {0};

	memset(string_ling, 0, sizeof(string_ling));

	fp = fopen("/mnt/mtd/iptable.txt", "r");
	if (fp == NULL) {
		printf("read iptable file fail\n");
		return 0;
	}

	//"/mnt/mtd/iptable.txt"只有前两行有效
	for (i = 0; i < 2; i++) {
		fgets(string_ling, 512, fp);
		if (strstr(string_ling, urlSuffix) != NULL) {
			break;
		}
	}

	i = (i >= 2 ? 0 : i);
	fclose(fp);

	return i;
}

//解析主次码流地址
//urlSuffix：码流地址
//bFile：源数据是否为文件
//nCh：主通道号
//bMain：次通道号
void Url_Analysis(char* urlSuffix, int *bFile, int *nCh, int *bMain)
{
	*bFile = 0;
	*nCh = 0;
	*bMain = 0;
	if(sscanf(urlSuffix,"av%d_%d", nCh, bMain) == 2)
	{
		*bFile = 0;
		if(*nCh > PORTNUM -1 || *nCh < 0 || *bMain > 1 || *bMain < 0)
		{
			*bFile = 0;
			*nCh = 0;
			*bMain = 0;
		}
	}
	else if(strstr(urlSuffix,".h264") != NULL || strstr(urlSuffix,".MP6") != NULL)
	{
		*bFile = 1;
	}
	else
	{
		*bFile = 0;
		*nCh = get_rtsp_ch(urlSuffix);
		*bMain = 0;
	}

	printf("url:%s bfile:%d nch:%d bmain:%d\n",urlSuffix, *bFile, *nCh, *bMain);
	return;
}

int _bRtspStartted = 0;

//启动rtsp服务器任务
int StartRTSPTask()
{
	int chn = 0;
	char szmultip[32]={0};

	if( !_bRtspStartted )
	{
		startRtspServer(554, 0, 1, 1460, PORTNUM);
		_bRtspStartted = 1;
	}

	if (Com_Env.Is_Init_Rtspattr == 0)
	{
		for (chn = 0; chn < PORTNUM; ++chn)
		{
			//设置主码流音视频属性
			if(SetRTSPAttr(chn, 0) != 0)
				return -1;
		}
		//设置服务器地址解析回调函数
		setUrlCallback(Url_Analysis);
		Com_Env.Is_Init_Rtspattr = 1;
	}

	return 0;
}





