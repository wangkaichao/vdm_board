
#include "global.h"

#include "rtsp_server_lib.h"

#include "GFNetApi.h"
#include "GFNetComm.h"
#include "GFNetStruct.h"

//#if GPS_XINAN_ADD
extern unsigned long       g_pts_interval[SER_MAX_CHANNEL][2]; //统计缓存中pts间隔
//#endif

#define RTP_HEAD_SIZE       12
#define TS_PACKET_SIZE      (7*188)
typedef struct RTP_HEAD_H_
{
	unsigned short  rtpHeaderInfo;
	unsigned short  seqNum;
	unsigned long   timeStamp; 
	unsigned long   ssrc;
}RTP_HEAD_H;


int IsGetRtpMainStream(void);
int IsGetRtpSubStream(void);

/*
  * 初始化流参数
  * Channel：数据流通道号
  * pStream：数据参数
  */
int InitStream(int channel, CHANNEL_STREAM *pStream)
{
	int				j;
	unsigned long	arg[4];

	memset(arg,0,sizeof(arg));
	//memset(pStream,0,sizeof(CHANNEL_STREAM));
	pStream->nChannelNo = 0;
	pStream->nPoolVideoCount = 0;
	pStream->nWriteVideoBegin = 0;
	pStream->nWriteVideoEnd = 0;
	pStream->nWriteVideoCycle = 0;	
	pStream->nLastIFrameBegin = 0;
	pStream->nLastIFrameEnd = 0;
	pStream->nLastIFrameCycle = 0;
	pStream->nLastIFrameCount = 0;
	pStream->nPoolAudioCount = 0;
	pStream->nWriteAudioBegin = 0;
	pStream->nWriteAudioEnd = 0;
	pStream->nWriteAudioCycle = 0;	
	pStream->pVideoBuffer = 0;
	pStream->pAudioBuffer = 0;
	pStream->nVideoFrameTick = 0;
	pStream->nAudioFrameTick = 0;	
	pStream->existChannel = 0;
	pStream->sync_send_num = 0;
	////////////////////////////////////////////
	pStream->nPoolVideoCountSub = 0;
	pStream->nWriteVideoBeginSub = 0;
	pStream->nWriteVideoEndSub = 0;
	pStream->nWriteVideoCycleSub = 0;	
	pStream->nLastIFrameBeginSub = 0;
	pStream->nLastIFrameEndSub = 0;
	pStream->nLastIFrameCycleSub = 0;
	pStream->nLastIFrameCountSub = 0;
	pStream->pVideoBufferSub = 0;
	pStream->nVideoFrameTickSub = 0;	
	pStream->nChannelNo = channel;
	pStream->existChannel = 0;
	pStream->nVideoFrameTick = 0;
	pStream->nVideoFrameTickSub = 0;
	pStream->nAudioFrameTick = 0;
	pStream->nVideoFrameTickSub = 0;
	pStream->nLastSendFrmNo = 0;
	pStream->nPoolVideoCount    = g_server_info.nVBufferCount;	
	pStream->nPoolVideoCountSub = g_server_info.nVBufferCount;
	if(pStream->nPoolVideoCountSub > 1024)
		pStream->nPoolVideoCountSub = 1024;

	pStream->nPoolAudioCount = g_server_info.nABufferCount;
	if(pStream->nPoolVideoCount > MAX_POOL_SIZE || pStream->nPoolAudioCount > MAX_POOL_SIZE)
		return FALSE;
/*
	if(NULL == (pStream->pAudioBuffer = (char *)malloc(sizeof(NET_PACKET_AUDIO)*pStream->nPoolAudioCount)))
	{
	   printf(" *************** MALLOC buffer failur(%d) ***************\n",__LINE__);
	   return FALSE;
	}

	memset(pStream->pAudioBuffer,0,sizeof(NET_PACKET_AUDIO)*pStream->nPoolAudioCount);
	for(j = 0;j < pStream->nPoolAudioCount;j++)
	pStream->pPoolAudioPacket[j] = (NET_PACKET_AUDIO *)((unsigned long)pStream->pAudioBuffer + sizeof(NET_PACKET_AUDIO)*j);
*/	
	pStream->pAudioBuffer = NULL;

	//main stream
	if(NULL == (pStream->pVideoBuffer = (char *)malloc(sizeof(NET_PACKET_VIDEO)*(pStream->nPoolVideoCount))))
	{
		printf(" *************** MALLOC buffer failur(%d) ***************\n",__LINE__);
		return FALSE;
	}

	memset(pStream->pVideoBuffer,0,sizeof(NET_PACKET_VIDEO)*(pStream->nPoolVideoCount));
	for(j = 0;j < pStream->nPoolVideoCount;j++)
		pStream->pPoolVideoPacket[j] = (NET_PACKET_VIDEO *)(pStream->pVideoBuffer + sizeof(NET_PACKET_VIDEO)*j);

	//sub stream
	if(NULL == (pStream->pVideoBufferSub = (char *)malloc(sizeof(NET_PACKET_VIDEO)*pStream->nPoolVideoCountSub)))
	{
		printf(" *************** MALLOC buffer failur(%d) ***************\n",__LINE__);
		return FALSE;
	}

	memset(pStream->pVideoBufferSub,0,sizeof(NET_PACKET_VIDEO)*pStream->nPoolVideoCountSub);
	for(j = 0;j < pStream->nPoolVideoCountSub;j++)
		pStream->pPoolVideoPacketSub[j] = (NET_PACKET_VIDEO *)(pStream->pVideoBufferSub + sizeof(NET_PACKET_VIDEO)*j);
	pStream->sync_send_num = 0;

	/*
	//pic
	if(NULL == (pStream->pPictureBuf = (char *) malloc(1024*1024+256)))
	{
		printf(" *************** MALLOC buffer failur(%d) ***************\n",__LINE__);
		return FALSE;
	}
	pStream->nPictureBufMax  = 1024*1024+256;
	pStream->nPictureSize    = 0;
	pStream->nPictureBufTick = 0;	
	*/
	pStream->pPictureBuf = NULL;

	//初始化Rtp流读写同步
	pthread_mutex_init(&pStream->mutexRtpWaitStreamData,NULL);
	pthread_cond_init(&pStream->RtpWaitStreamData,NULL);

	if((pStream->hWaitDelaySock = GF_Create_Sock(2)) <= 0)
	{
		printf(" *************** SOCK CREATE failur(%d) ***************\n",__LINE__);
		return FALSE;
	}
	GF_Set_Sock_Attr(pStream->hWaitDelaySock, 1, 0, 0, 0, 256000);
	return TRUE;
}

/*
  * 删除流接口
  * nChannelNo：流通道号
  * pStream：数据参数
  */
void DestStream(int nChannelNo,CHANNEL_STREAM	*pStream)
{
	if(pStream->existChannel)
		return;

	pStream->existChannel = 1;

	printf("DestStream  channel :%d \n",nChannelNo);
	pthread_cond_broadcast(&pStream->qWaitStreamData);
	sleep(1);
	if(pStream->pVideoBuffer)
	{
		free((void *)pStream->pVideoBuffer);
		pStream->pVideoBuffer = NULL;
	}
	if(pStream->pVideoBufferSub)
	{
		free((void *)pStream->pVideoBufferSub);
		pStream->pVideoBufferSub = NULL;
	}
	if(pStream->pAudioBuffer)
	{
		free((void *)pStream->pAudioBuffer);
		pStream->pAudioBuffer = NULL;
	}

}

/*
  * 写音频数据到缓冲区
  * nChannelNo：流通道号
  * pStream：数据流通道参数
  * pAudioData：音频数据
  */
int  WriteAudio2Buf(unsigned int nChannelNo,CHANNEL_STREAM *pStream, const char *pAudioData)
{
	NET_PACKET_AUDIO *pNetPacket = NULL;
	GF_FRAME_HEAD	 *pAudioHead = (GF_FRAME_HEAD *)pAudioData; 
	unsigned int            frameLen   = 0;
	unsigned int            frameNum   = 0;
	unsigned int            i          = 0;
	unsigned int            writeLen   = 0;
	unsigned int            copyLen    = 0;

	if(pAudioHead == NULL || (0 != pAudioHead->zeroFlag) || (1 != pAudioHead->oneFlag))
	{
		printf("WriteAudio2Buf data error \n");
		return 0;
	}

	frameLen = sizeof(GF_FRAME_HEAD)+pAudioHead->nByteNum;
	if(frameLen > NET_PACKET_AUDIO_SIZE) //ljm 2010-04-03
	{
		printf("WriteAudio2Buf data too big %ld \n", frameLen);
		return 0;
	}

	if(g_server_info.pNSSFrame)
	{
		memcpy(g_server_info.pNSSFrame,pAudioData,sizeof(GF_FRAME_HEAD)+pAudioHead->nByteNum);
		//add for NSS network
		if(g_server_info.pNSSCallback)
			g_server_info.pNSSCallback((int)pAudioHead,(int)nChannelNo);
	}

	pthread_mutex_lock(&pStream->muAudioWriteRead);
	if(pStream->existChannel)
	{
		pthread_mutex_unlock(&pStream->muAudioWriteRead);
		return 0;
	}

	frameNum = (frameLen + NET_PACKET_AUDIO_SIZE -1)/NET_PACKET_AUDIO_SIZE;
	pStream->nWriteAudioBegin = pStream->nWriteAudioEnd;
	for (i = 0; i < frameNum; ++i)
	{
		writeLen   = (frameLen > NET_PACKET_AUDIO_SIZE) ? NET_PACKET_AUDIO_SIZE:frameLen;
		pNetPacket = pStream->pPoolAudioPacket[pStream->nWriteAudioEnd++];

		if(pStream->nWriteAudioEnd == pStream->nPoolAudioCount)
		{
			pStream->nWriteAudioEnd = 0;
			pStream->nWriteAudioCycle++;
		}

		pNetPacket->packetHead.nNetFlag     = GF_MAGIC_FLAG;
		pNetPacket->packetHead.nPakcetNo    = i;
		pNetPacket->packetHead.nPakcetCount = frameNum;
		pNetPacket->packetHead.nBufferSize  = writeLen;
		memcpy(&pNetPacket->packetHead.frameHead,pAudioHead,sizeof(GF_FRAME_HEAD));
		pNetPacket->packetHead.nFrameNo = pStream->nAudioFrameTick;
		memcpy(pNetPacket->packetData, pAudioData+copyLen,writeLen);
		copyLen  += writeLen;
		frameLen -= writeLen;
	}

	pStream->nAudioFrameTick++;

	pthread_mutex_unlock(&pStream->muAudioWriteRead);
	pthread_cond_signal(&pStream->qWaitStreamData);

	//通知RTP线程接收流
	NotifyRtpRecAudio();

	return copyLen;
}

/* 将视频数据写入网络发送缓冲区CHANNEL_STREAM
  * nChannelNo：数据流通道号
  * pStream：数据流参数
  * pVideoData：视频数据
  */
int  WriteVideo2Buf(unsigned int nChannelNo, CHANNEL_STREAM *pStream, const char *pVideoData)
{
	NET_PACKET_VIDEO	*pNetPacket  = NULL;
	EXT_FRAME_HEAD		*pExtFrmHead = NULL;
	GF_FRAME_HEAD	    *pVideoHead  = NULL;
	unsigned int               frameLen    = 0;
	unsigned int               frameNum    = 0;
	unsigned int               bIFrame     = 0;
	unsigned int               i           = 0;
	unsigned int               copyLen     = 0;
	unsigned int               sendLen     = 0;

	pVideoHead  = (GF_FRAME_HEAD *)pVideoData;
	if(pVideoHead == NULL || ((0 != pVideoHead->zeroFlag) || (1 != pVideoHead->oneFlag)))
	{
		printf("WriteVideo2Buf data error \n");
		return 0;
	}

	if(g_server_info.pNSSFrame) // pNSSFrame == NULL, pNSSCallback == NULL
	{
		if(g_server_info.pNSSCallback)
			g_server_info.pNSSCallback((int)pVideoData,(int)nChannelNo);
	}

	pthread_mutex_lock(&pStream->muVideoWriteRead);
	if(pStream->existChannel)
	{
		pthread_mutex_unlock(&pStream->muVideoWriteRead);          
		return 0;
	}

	pExtFrmHead = (EXT_FRAME_HEAD*)(pVideoData + sizeof(GF_FRAME_HEAD));

	//总长度
	frameLen    = sizeof(GF_FRAME_HEAD) + pVideoHead->nByteNum;

	//需要分割的存储单元数，整个包参与分割包括分组头
	frameNum    = (frameLen + NET_PACKET_VIDEO_SIZE -1)/NET_PACKET_VIDEO_SIZE;

	if(frameNum > pStream->nPoolVideoCount)
	{
		printf("WriteVideo2Buf data to more %ld \n", frameNum);
		pthread_mutex_unlock(&pStream->muVideoWriteRead);          
		return 0;
	}
	//有视频，判断I，P侦
	//是否I帧
	bIFrame = (STREAM_FLAG_VI == pVideoHead->streamFlag) ? 1 : 0;
	//开始拷贝视频数据
	copyLen = 0;
	pStream->nWriteVideoBegin = pStream->nWriteVideoEnd;
	pNetPacket                = pStream->pPoolVideoPacket[pStream->nWriteVideoBegin];

	//时间戳
	pStream->nLastestFrmPTS = pExtFrmHead->nTimestamp;
	for(i = 0;i < frameNum;i++)
	{
		//以1024为单位进行分割
		//本单元的长度
		sendLen = frameLen > NET_PACKET_VIDEO_SIZE ? NET_PACKET_VIDEO_SIZE : frameLen;

		//pNetPacket 指向一个缓冲单元
		pNetPacket = pStream->pPoolVideoPacket[pStream->nWriteVideoEnd++];

		//已写到缓冲池末尾，开始环绕
		if(pStream->nWriteVideoEnd == pStream->nPoolVideoCount)
		{
			pStream->nWriteVideoEnd = 0;
			//记录绕回写的次数
			pStream->nWriteVideoCycle++;
		}

		pNetPacket->packetHead.nNetFlag     = GF_MAGIC_FLAG;
		//视频单元在整个视频分组中的单元索引
		pNetPacket->packetHead.nPakcetNo    = i;
		//整个媒体数据分组包括的缓冲单元数
		pNetPacket->packetHead.nPakcetCount = frameNum;
		//被分割的视频分组头
		pNetPacket->packetHead.frameHead    = *pVideoHead;
		//本单元的数据尺寸
		pNetPacket->packetHead.nBufferSize  = sendLen;
		memcpy(pNetPacket->packetData, pVideoData+copyLen, sendLen);

		//未组装的剩余数据的尺寸
		frameLen -= sendLen;
		//媒体分组的下次拷贝位置
		copyLen  += sendLen;

		//填写视频帧帧号
		pNetPacket->packetHead.nFrameNo = pStream->nVideoFrameTick;
	}

	// 该流缓冲池中视频数据帧的个数
	pStream->nVideoFrameTick++;
	//记录最近接收的I帧信息
	if(bIFrame)
	{
		//I帧序列帧的起始单元号
		pStream->nLastIFrameBegin	= pStream->nWriteVideoBegin;
		//I帧序列帧的结束单元号
		pStream->nLastIFrameEnd		= pStream->nWriteVideoEnd;
		pStream->nLastIFrameCycle	= pStream->nWriteVideoCycle;
		//最近写入的 I帧帧号
		pStream->nLastIFrameCount   = frameNum;
		pStream->nRequstIFrmOK      = 1;

		pNetPacket = pStream->pPoolVideoPacket[pStream->nLastIFrameBegin];
		//printf("WriteVideo2Buf;LastIFrame:%u;frameNum:%u;\n",
		//pStream->nLastIFrameBegin,
		//pNetPacket->packetHead.nFrameNo);

		if (pNetPacket && 0!= pNetPacket->packetHead.nPakcetNo)
		{
			printf("error PacketNo:%d",pNetPacket->packetHead.nPakcetNo);
		}
	}

	pthread_mutex_unlock(&pStream->muVideoWriteRead);
	pthread_cond_signal(&pStream->qWaitStreamData);

	//通知RTP线程接收流mazhidong 2015-6-11
	NotifyRtpRecMainStream();
	return copyLen;
}

/* 将音视频写入网络发送缓冲区CHANNEL_STREAM
  * 将视频数据写入到缓冲区，调用前面两个接口完成数据写入
  * nChannelNo：数据流通道号
  * pStream：数据流参数
  * pFrameData：视频数据
  * pAudioData：音频数据
  */
unsigned long WriteFrame(unsigned long nChannelNo,CHANNEL_STREAM *pStream,const char *pFrameData,const char *pAudioData)
{
	unsigned long	copyLen     = 0;
	unsigned long	sndDelayNum = 0;

	if(pStream->existChannel ==1)
		return 0;
	//有音频,得到音频的BUFFER,拷贝音频数据
	if(pAudioData)
	{
		copyLen = WriteAudio2Buf(nChannelNo, pStream, pAudioData);
	}
	else
	{	
		//将视频数据写入网络发送缓冲区CHANNEL_STREAM
		copyLen = WriteVideo2Buf(nChannelNo, pStream, pFrameData);
	}

	if(copyLen > 0)
	{
		//	    printf("copyLen = %d\n", copyLen);
		sndDelayNum = 0;
		if((pStream->nVideoFrameTick > pStream->nLastSendFrmNo) && (0!=pStream->nLastSendFrmNo))
		{
			//未发送的帧数
			sndDelayNum = pStream->nVideoFrameTick - pStream->nLastSendFrmNo;
		}

		//发送慢，放慢写的速度
		if(sndDelayNum > 5)
		{
			sndDelayNum = sndDelayNum - 5;
			GF_Select(&pStream->hWaitDelaySock, 0x1, 0x1, sndDelayNum*50);
		}
	}

	return copyLen;
}

unsigned long WriteFrameSub(unsigned long nChannelNo,CHANNEL_STREAM *pStream,const char *pFrameData)
{
	int				i;
	unsigned long	bIFrame = 0;
	unsigned long	frameLen = 0,frameNum = 0;
	unsigned long	sendLen = 0,copyLen = 0;
	NET_PACKET_VIDEO *pNetPacket = NULL;

	GF_FRAME_HEAD	*pVideoHead = (GF_FRAME_HEAD *)pFrameData;
	EXT_FRAME_HEAD		*pExtFrmHead = NULL;

	if(pStream->existChannel ==1)
		return 0;

	//检测包标志
	if(pVideoHead == NULL || ((0 != pVideoHead->zeroFlag) || (1 != pVideoHead->oneFlag)))
	{
		printf("WriteFrameSub data error \n");
		return 0;
	}
	pExtFrmHead =(EXT_FRAME_HEAD *) (pFrameData + sizeof(GF_FRAME_HEAD));

	pthread_mutex_lock(&pStream->muVideoWriteReadSub);

	frameLen = sizeof(GF_FRAME_HEAD) + pVideoHead->nByteNum;
	frameNum = (frameLen + NET_PACKET_VIDEO_SIZE -1)/NET_PACKET_VIDEO_SIZE;
	//开始拷贝视频数据
	if(pStream->existChannel)
	{
		pthread_mutex_unlock(&pStream->muVideoWriteReadSub);          
		return 0;
	}

	if(frameNum > pStream->nPoolVideoCountSub)
	{
		printf("WriteFrameSub data to more %ld \n", frameNum);
		pthread_mutex_unlock(&pStream->muVideoWriteReadSub);          
		return 0;
	}
	//有视频，判断I，P侦
	bIFrame = (STREAM_FLAG_VI == pVideoHead->streamFlag) ? 1 : 0;
	copyLen = 0;
	pStream->nWriteVideoBeginSub = pStream->nWriteVideoEndSub;
	pStream->nLastestFrmPTSSub   = pExtFrmHead->nTimestamp;
	for(i = 0;i < frameNum;i++)
	{
		sendLen = frameLen > NET_PACKET_VIDEO_SIZE?NET_PACKET_VIDEO_SIZE:frameLen;

		pNetPacket = pStream->pPoolVideoPacketSub[pStream->nWriteVideoEndSub++];
		if(pStream->nWriteVideoEndSub == pStream->nPoolVideoCountSub)
		{
			pStream->nWriteVideoEndSub = 0;
			pStream->nWriteVideoCycleSub++;
		}
		pNetPacket->packetHead.nNetFlag     = GF_MAGIC_FLAG;
		pNetPacket->packetHead.nPakcetNo    = i;
		pNetPacket->packetHead.nPakcetCount = frameNum;
		pNetPacket->packetHead.frameHead    = *pVideoHead;
		pNetPacket->packetHead.nBufferSize  = sendLen;
		memcpy(pNetPacket->packetData,pFrameData+copyLen,sendLen);
		frameLen -= sendLen;
		copyLen += sendLen;

		//视频帧的帧号
		pNetPacket->packetHead.nFrameNo = pStream->nVideoFrameTickSub;
	}

	pStream->nVideoFrameTickSub++;
	if(bIFrame)
	{
		pStream->nLastIFrameBeginSub	= pStream->nWriteVideoBeginSub;
		pStream->nLastIFrameEndSub		= pStream->nWriteVideoEndSub;
		pStream->nLastIFrameCycleSub	= pStream->nWriteVideoCycleSub;
		pStream->nLastIFrameCountSub    = frameNum;
	}

	pthread_mutex_unlock(&pStream->muVideoWriteReadSub);
	pthread_cond_signal(&pStream->qWaitStreamDataSub);

	//通知RTP线程接收流
	NotifyRtpRecSubStream();
	return copyLen;
}


int	GetOneAudioFrame(int nChnNo,char *pPackHead,
		char *pFrameBuf,int nFrameBufSize,
		unsigned int nAudioFrmNo,unsigned int *nTrueFrmNo,
		int *nLastAudioPos,
		unsigned int gotoPts, unsigned int ptsdiffer)
{	
	int				ii = 0,nRet = 0,nBegin = 0,bFind = 0;
	unsigned long	nGetPos = 0;
	NET_PACKET_AUDIO	*pCopyAudio = NULL;
	CHANNEL_STREAM	*pStream = NULL;
	int nPackCount = 0;

	EXT_FRAME_HEAD *pExtFrameHeader = NULL;

	if(nChnNo >= g_server_info.nChannelNum || nChnNo < 0)
	{
		printf("GetOneAudioFrame chn error %d\n",  nChnNo);
		return -1;
	}

	if((pStream = &g_channel_stream[nChnNo]) == NULL ||
			pStream->existChannel == 1)
	{
		printf("GetOneAudioFrame channel exit\n");
		return -1;
	}

	if(nFrameBufSize < (int)(sizeof(NET_PACKET_AUDIO)))
	{
		printf(" Input buffer pFrameBuf too small \n");
		return -1;
	}

	if(*nLastAudioPos <= 0)
		*nLastAudioPos = pStream->nWriteAudioBegin;

	pthread_mutex_lock(&pStream->muAudioWriteRead);

	/*printf("find audio frame;nAudioFrameTick:%d;nAudioFrameTick:%d;nAudioFrmNo:%d;nPoolAudioCount:%d;gotoPts:%ul,ptsdiffer:%d\n",
	  pStream->nAudioFrameTick,
	  nAudioFrmNo,
	  pStream->nPoolAudioCount,
	  gotoPts,
	  ptsdiffer);*/

	if (nAudioFrmNo > 0 && nAudioFrmNo == pStream->nAudioFrameTick)
	{
		pthread_mutex_unlock(&pStream->muAudioWriteRead);
		return 0;
	}

	//第一次取则取最新的 
	if(gotoPts == 0 && 0 == nAudioFrmNo)	
	{
		printf("reset search position;\n");
		nGetPos = pStream->nWriteAudioBegin;
	}
	else
	{
		bFind  = 0;
		//modified by mazhidong
		nBegin = *nLastAudioPos;


		for(ii = 0; ii < pStream->nPoolAudioCount; ii++)
		{

			pCopyAudio =(NET_PACKET_AUDIO *) pStream->pPoolAudioPacket[(nBegin+ii) % pStream->nPoolAudioCount];
			/*printf("find audio frame;nAudioFrameTick:%d;nFrameNo:%d;nAudioFrmNo:%d;nPoolAudioCount:%d;gotoPts:%d,ptsdiffer:%d\n",
			  pStream->nAudioFrameTick,
			  pCopyAudio->packetHead.nFrameNo,
			  nAudioFrmNo,
			  pStream->nPoolAudioCount,
			  gotoPts,
			  ptsdiffer);*/


			//需要跳转到指定的pts附近 modify by mazhidong 2015/6/4
			if (gotoPts > 0 && ptsdiffer > 0)
			{
				int timediff = 0;
				//pExtFrmHead = (EXT_FRAME_HEAD *)((char *)pNetPacket->packetData + sizeof(GF_FRAME_HEAD));

				pExtFrameHeader = (EXT_FRAME_HEAD *)((char *)pCopyAudio->packetData + 12);


				timediff = pExtFrameHeader->nTimestamp > gotoPts ?  (pExtFrameHeader->nTimestamp - gotoPts):(gotoPts- pExtFrameHeader->nTimestamp);

				//printf("nTimestamp:%d;timediff:%d;gotoPts:%ul\n",
				//pExtFrameHeader->nTimestamp,
				//timediff,
				//gotoPts);

				if(NULL != pCopyAudio &&
						GF_MAGIC_FLAG == pCopyAudio->packetHead.nNetFlag &&
						timediff <= ptsdiffer)
				{
					printf("go to the specific audio frame;\n");
					nGetPos = ((nBegin+ii)%pStream->nPoolAudioCount);
					bFind = 1;	
					break;
				}
			}
			else
			{
				pCopyAudio = pStream->pPoolAudioPacket[(nBegin+ii)%pStream->nPoolAudioCount];		
				if(  pCopyAudio != NULL && 
						(pCopyAudio->packetHead.nFrameNo) == nAudioFrmNo && 
						(GF_MAGIC_FLAG == pCopyAudio->packetHead.nNetFlag) &&
						(pCopyAudio->packetHead.nBufferSize < nFrameBufSize))
				{	
					//printf("find to audio frame;\n");
					nGetPos = ((nBegin+ii)%pStream->nPoolAudioCount);
					bFind = 1;	
					break;
				}
			}
		}

		if(!bFind && !gotoPts) //没找到匹配的帧
		{
			pCopyAudio = pStream->pPoolAudioPacket[pStream->nWriteAudioBegin];
			if(pCopyAudio != NULL && (pCopyAudio->packetHead.nFrameNo > nAudioFrmNo) && (GF_MAGIC_FLAG == pCopyAudio->packetHead.nNetFlag) && (pCopyAudio->packetHead.nBufferSize < nFrameBufSize))
			{					
				nGetPos = pStream->nWriteAudioBegin;
				bFind = 1;
			}
			else			
			{
				pthread_mutex_unlock(&pStream->muAudioWriteRead);
				return 0;
			}
		}
		else if (!bFind && gotoPts)
		{
			pthread_mutex_unlock(&pStream->muAudioWriteRead);
			return 0;
		}
	}

	pCopyAudio = pStream->pPoolAudioPacket[nGetPos];
	if( pCopyAudio== NULL || GF_MAGIC_FLAG != pCopyAudio->packetHead.nNetFlag ||
			pCopyAudio->packetHead.nBufferSize > nFrameBufSize)
	{
		pthread_mutex_unlock(&pStream->muAudioWriteRead);
		return 0;
	}

	nRet = pCopyAudio->packetHead.nBufferSize;
	if(pPackHead)
		memcpy(pPackHead,&pCopyAudio->packetHead,sizeof(NET_PACKET_HEAD));

	if(pFrameBuf)
		memcpy(pFrameBuf,pCopyAudio->packetData,pCopyAudio->packetHead.nBufferSize);

	//修改下次音频帧的搜索位置
	nPackCount = pCopyAudio->packetHead.nPakcetCount;
	*nLastAudioPos = (nGetPos + nPackCount)%pStream->nPoolAudioCount;

	*nTrueFrmNo = pCopyAudio->packetHead.nFrameNo;
	pthread_mutex_unlock(&pStream->muAudioWriteRead);
	return nRet;	
}

/* 检查缓冲区中下一帧帧号
  * pStream：数据流参数
  * nVideoFrmNo：期待获取的帧号
  */
int	CheckTotalFrameNo(CHANNEL_STREAM *pStream,int nVideoFrmNo)
{
	int					ii = 0;
	NET_PACKET_VIDEO	*pPoolVideoPacket = NULL;

	if(pStream == NULL)
		return -3;

	for(ii = 0;ii < pStream->nPoolVideoCount; ii++)
	{
		if(ii >= MAX_POOL_SIZE)
			return -2;

		pPoolVideoPacket = pStream->pPoolVideoPacket[ii];

		if(pPoolVideoPacket == NULL)
			return -1;

		if(pPoolVideoPacket->packetHead.nFrameNo == nVideoFrmNo)
		{
			return 1;
		}
	}

	return 0;
}

/* 检验整帧数据，主要检验数据头
  * pStream：流通道参数
  * nBegin：检验起始块
  * bSub：是否检验子码流，字符叠加器无子码流
  */
int	CheckTotalFrame(CHANNEL_STREAM *pStream,unsigned long nBegin,int bSub)
{
	unsigned	long	nFrameNo;
	unsigned	long	nEnd;
	NET_PACKET_VIDEO	*pNetPacket = NULL;

	if(bSub)
		pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacketSub[nBegin];
	else
		pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacket[nBegin];

	if((GF_MAGIC_FLAG != pNetPacket->packetHead.nNetFlag) ||
			(0 != pNetPacket->packetHead.nPakcetNo))
	{
		printf("CheckTotalFrame Beg Pack Flag  Err %ld %d\n", 
				pNetPacket->packetHead.nNetFlag,
				pNetPacket->packetHead.nPakcetNo);
		return 0;
	}

	if(bSub)
		nEnd = (nBegin + pNetPacket->packetHead.nPakcetCount - 1)%pStream->nPoolVideoCountSub;
	else
		nEnd = (nBegin + pNetPacket->packetHead.nPakcetCount - 1)%pStream->nPoolVideoCount;
	nFrameNo = pNetPacket->packetHead.nFrameNo;

	if(bSub)
		pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacketSub[nEnd];	
	else
		pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacket[nEnd];	

	//最后一个单元号进行校验
	if((GF_MAGIC_FLAG != pNetPacket->packetHead.nNetFlag) || 
			(pNetPacket->packetHead.nPakcetCount != (pNetPacket->packetHead.nPakcetNo+1)))
	{
		printf("CheckTotalFrame End Pack Flag  Err %ld %d %d\n", 
				pNetPacket->packetHead.nNetFlag,
				pNetPacket->packetHead.nPakcetCount,
				pNetPacket->packetHead.nPakcetNo+1);
		return 0;
	}

	//分组号校验
	if(nFrameNo != pNetPacket->packetHead.nFrameNo)
	{
		printf("CheckTotalFrame End Pack FrameNo  Err %ld %ld\n", 
				nFrameNo, pNetPacket->packetHead.nFrameNo);
		return 0;
	}
	return 1;
}

/* 整帧数据拷贝
  * pStream：流通道参数
  * pFrameBuf：拷贝缓冲区
  * nBegin：开始拷贝块
  * bSub：是否子码流
  * nLastVideoPos：下一个视频位置
  */
int	CopyTotalFrame(CHANNEL_STREAM *pStream,char *pFrameBuf,unsigned long nBegin,int bSub,int *nLastVideoPos)
{
	int					ii = 0,nPackCount = 0;
	NET_PACKET_VIDEO	*pNetPacket = NULL;

	if(bSub)
		pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacketSub[nBegin];
	else
		pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacket[nBegin];

	nPackCount = pNetPacket->packetHead.nPakcetCount;
	for(ii = 0;ii < nPackCount;ii++)
	{
		if(bSub)		
			pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacketSub[(nBegin+ii)%pStream->nPoolVideoCountSub];			
		else
			pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacket[(nBegin+ii)%pStream->nPoolVideoCount];			

		memcpy(pFrameBuf+ii*NET_PACKET_VIDEO_SIZE,pNetPacket->packetData,pNetPacket->packetHead.nBufferSize);		
	}

	//下一次开始查找的帧的起始单元号
	if(bSub)
		*nLastVideoPos = (nBegin+nPackCount)%pStream->nPoolVideoCountSub;
	else
		*nLastVideoPos = (nBegin+nPackCount)%pStream->nPoolVideoCount;

	return 1;
}

/* 检查是否有下一帧到来
  * pStream：数据流参数
  * bSub：是否为子码流
  * pPoolCount：未用
  * pLastestPts：最后一帧时间戳
  * pLastestFrmNo：最后一帧帧号
  * nReqTick：请求帧间隔
  */
int CheckExitsNewFrame(CHANNEL_STREAM *pStream, int bSub, unsigned int *pPoolCount, 
		unsigned int *pLastestPts, unsigned int *pLastestFrmNo, unsigned int nReqTick)
{
	if(bSub)
		*pPoolCount = pStream->nPoolVideoCountSub;
	else
		*pPoolCount = pStream->nPoolVideoCount;

	if(bSub)
		*pLastestPts = pStream->nLastestFrmPTSSub;
	else
		*pLastestPts = pStream->nLastestFrmPTS;

	if(bSub)
		*pLastestFrmNo = pStream->nVideoFrameTickSub;
	else
		*pLastestFrmNo = pStream->nVideoFrameTick;

	//首先检查请求的PTS和最新的PTS
	if(*pLastestFrmNo <= nReqTick)//BUFFER 中没有更新的 PTS
		return 0;  

	return 1;
}

int  GetNewIFrame(CHANNEL_STREAM *pStream, int nLastestPts, int nPrePts, 
		int nPreNo, int bSub, unsigned int *pPos)
{
	NET_PACKET_VIDEO *pNetPacket = NULL;

	return 0;
}
//函数说明:根据参数输入取得适当的帧位置
//CHANNEL_STREAM *pStream 通道,unsigned long nReqTick 请求的时间,unsigned long nBegin 开始的位置,
//unsigned long *nGetPos  得到的位置,int *bMath 是否完全匹配,unsigned long interMilliSecond 最短时间差,
//unsigned long preFrmTime前一帧时间戳 ,int bSub 是否是子码流
//返回: =0 没有更新的包可读    >0 取得了新包的位置
//nReqTick ---期望得到的帧号
int	FindByTimeTick(CHANNEL_STREAM *pStream,unsigned int nReqTick,unsigned int nBegin,
		unsigned int *nGetPos,  int *bMatch, unsigned int interMilliSecond,
		unsigned int preFrmTime,int bSub)
{
	int					nFindOk = 0;
	int					ii      = 0; //,bFindIFrm = 0;
	unsigned long		nMinInterval = 0xffffffff;
	NET_PACKET_VIDEO	*pNetPacket = NULL;
	EXT_FRAME_HEAD		*pExtFrmHead = NULL;
	unsigned int	nPoolCnt = 0, nLastestPts = 0, nLastestNo = 0;	

	*bMatch  = 0;
	*nGetPos = 0;

	//nLastestPts---最近接收的视频帧的时间戳
	if (0 == CheckExitsNewFrame(pStream, bSub, &nPoolCnt, &nLastestPts,
				&nLastestNo, nReqTick))
	{
		printf("no new frame in buffer;\n");
		return 0; //没有更新的包可读
	}

	//缓存太多 返回最新的I帧
	/*if (GetNewIFrame(pStream, nLastestPts, preFrmTime, nReqTick, bSub, nGetPos))
	  {
	 *bMatch     = 1;
	 printf("too much ret newest frame;\n");
	 return 1;
	 }*/

	//从当前的缓冲中查找要取得的帧号
	for(ii = 0; ii < nPoolCnt; ii++)
	{
		//从上次读的最后一位开始找,正常情况应该一次找到
		if(bSub)
			pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacketSub[(nBegin+ii)%nPoolCnt];
		else
			pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacket[(nBegin+ii)%nPoolCnt];
		//找到合适的帧了
		if((GF_MAGIC_FLAG == pNetPacket->packetHead.nNetFlag) && 
				(0             == pNetPacket->packetHead.nPakcetNo) &&//是分组的起始单元号
				(nReqTick      == pNetPacket->packetHead.nFrameNo))
		{
			*nGetPos    = ((nBegin+ii)%pStream->nPoolVideoCount);
			nFindOk    = 1;
			*bMatch     = 1;
			//printf("find proper fram;\n");
			break; 
		}
	}
	//没找到匹配的帧 返回最新的IFrame
	if(nFindOk == 0)
	{

		if(bSub)
			pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacketSub[pStream->nLastIFrameBeginSub];
		else
			pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacket[pStream->nLastIFrameBegin];

		if(bSub)
			*nGetPos = pStream->nLastIFrameBeginSub;
		else
			*nGetPos = pStream->nLastIFrameBegin;

		//printf("ret newrest Vedio frame;LastIFrameBegin:%u;\n",
		//*nGetPos);

		*bMatch     = 1;
		return 1;
	}

	//发送的太慢
	//返回满足时间差最近的I 帧
	if (0)
		//else if(interMilliSecond && (nLastestPts > preFrmTime) && ((nLastestPts - preFrmTime)>interMilliSecond))
	{	//匹配了要找的帧 在缓冲内寻找该时间间隔内的I帧
		//该情形是针对用户的实时性的调节,用户要求两帧的时间差不能大于interMilliSecond时,
		//会找一个时间差小于interMilliSecond的最早的I帧

		nMinInterval = 0xffffffff;
		for(ii = 0;ii < nPoolCnt;ii++)
		{
			if(bSub)
				pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacketSub[(nBegin+ii)%nPoolCnt];
			else
				pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacket[(nBegin+ii)%nPoolCnt];

			pExtFrmHead = (EXT_FRAME_HEAD *)((char *)pNetPacket->packetData + sizeof(GF_FRAME_HEAD));

			if( (pNetPacket->packetHead.nFrameNo > nReqTick) && //帧号条件满足
					(GF_MAGIC_FLAG == pNetPacket->packetHead.nNetFlag) && //Flag ok
					(0             == pNetPacket->packetHead.nPakcetNo)&& //是一帧的第一包
					(pNetPacket->packetHead.frameHead.streamFlag==FRAME_FLAG_VI) && //I帧
					(nLastestPts  >= pExtFrmHead->nTimestamp) && 
					(nLastestPts  -  pExtFrmHead->nTimestamp < interMilliSecond) &&
					(nMinInterval > (pNetPacket->packetHead.nFrameNo-nReqTick)))
			{
				pExtFrmHead  = (EXT_FRAME_HEAD *)((char *)pNetPacket->packetData + 
						sizeof(GF_FRAME_HEAD));
				nMinInterval = pNetPacket->packetHead.nFrameNo - nReqTick;
				*nGetPos     = ((nBegin+ii)%pStream->nPoolVideoCount);
				nFindOk      = 1;
				printf("skip frame number %d\n", (int)(pNetPacket->packetHead.nFrameNo
							-nReqTick));
				printf("skip frame numbers;\n");
				*bMatch     = 1;
				break;

			}	
		}
	}

	return nFindOk;
}
//interMilliSecond---1000 preFrmTime--上一帧的时间戳
int   GetOneVideoPos(CHANNEL_STREAM	*pStream, int nVideoFrmNo, int bKeyFrame, 
		int bSubChn, int *pLastVideoPos,  int interMilliSecond, 
		int preFrmTime, int bLostLastestKey)
{
	int bMath      = 0;
	unsigned int nGetPos = 0;

	//    printf("=====================================GetOneVideoPos -->\n");

	////第一次读取,取得缓冲的最后一个I帧
	if(0 == nVideoFrmNo || bKeyFrame)//要求读取I帧
	{
		bKeyFrame = 1;
		if(bSubChn)
		{
			//最近的I帧序列帧起始单元号
			nGetPos = pStream->nLastIFrameBeginSub;	
		}
		else
			nGetPos = pStream->nLastIFrameBegin;

//		printf("first read;nGetPos:%u;\n",nGetPos);
	}
	else
	{	//如果当前要取的帧号比缓冲中最大的帧号还大,返回0
		// 对期望要取得帧号的有效性进行校验
		if(bSubChn)		
		{
			if(nVideoFrmNo >= pStream->nVideoFrameTickSub)
			{
				//printf("error parameter;nVideoFrmNo:%d;nVideoFrameTick:%d;\n",
				//nVideoFrmNo,
				//pStream->nVideoFrameTick);
				return -2;
			}
		}
		else
		{
			if(nVideoFrmNo >= pStream->nVideoFrameTick)
			{
				//printf("error parameter;nVideoFrmNo:%d;nVideoFrameTick:%d;\n",
				//nVideoFrmNo,
				//pStream->nVideoFrameTick);
				return -2;
			}
		}

		//对 *nLastVideoPos的值进行有效性判断,确保在缓冲之内
		if(bSubChn && (*pLastVideoPos < 0 || *pLastVideoPos >= pStream->nPoolVideoCountSub))
			*pLastVideoPos = pStream->nWriteVideoBeginSub;
		else if(*pLastVideoPos < 0 || *pLastVideoPos >= pStream->nPoolVideoCount)
			*pLastVideoPos = pStream->nWriteVideoBegin;
		//根据输入条件,取得一帧和位置	
		//pLastVideoPos--起始搜寻位置
		if(FindByTimeTick(pStream, nVideoFrmNo, *pLastVideoPos, &nGetPos,
					&bMath, interMilliSecond, preFrmTime, bSubChn) == 0)
		{//没有 取到更新的包
			printf("fail find proper frame;\n");
			return -4;
		}
		//没有匹配的帧，返回最近接收的I帧位置
		if(!bMath && bLostLastestKey) 
		{
			NET_PACKET_VIDEO	*pNetPacket = NULL;

			if(bSubChn)
			{
				nGetPos = pStream->nLastIFrameBeginSub;
				pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacketSub[nGetPos];
			}
			else
			{
				nGetPos = pStream->nLastIFrameBegin;
				pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacket[nGetPos];
			}

			printf("can't find proper video frame;get last I Frame position;nVideoFrmNo:%d;\n",nVideoFrmNo);
			if (pNetPacket)
			{
				if (0!= pNetPacket->packetHead.nPakcetNo)
				{
					printf("error PacketNo:%d",pNetPacket->packetHead.nPakcetNo);
				}
			}
			bKeyFrame = 1;
		}
	}
	return nGetPos;
}

//函数说明:取得一个视频帧
//int nChnNo  通道号,	int bSubChn	子码流,	char *pFrameBuf	缓存,	int nFrameBufSize	缓存大小,
//int bKeyFrame					  是否要关键帧,i nt bLostLastestKey		    视频丢失时直接取最后一关键帧
//unsigned long nVideoFrmNo		  希望取得的帧号,unsigned long *nRealFrmNo  实际取得的帧号,
//unsigned long interMilliSecond  时间差        ,unsigned long preFrmTime   前一帧的时间戳,
//unsigned long *nowFrmTime	      当前取得的时间戳, int *nLastVideoPos	    视频帧的位置

//返回: <0 读出错   =0 无最新数据可读  >0 正确,返回该帧长度
//nVideoFrmNo---期望得到的帧号nRealFrmNo---实际返回的帧号bLostLastestKey----1
int	GetOneVideoFrame(int nChnNo,int bSubChn,char *pFrameBuf,int nFrameBufSize,
		int bKeyFrame,int bLostLastestKey, unsigned int nVideoFrmNo,
		unsigned int *nRealFrmNo,unsigned int interMilliSecond,unsigned int preFrmTime,
		unsigned int *nowFrmTime,int *nLastVideoPos)
{
	int			nRet = 0,nFrameLen = 0;
	int			nGetPos = 0;
	CHANNEL_STREAM		*pStream = NULL;
	NET_PACKET_VIDEO	*pNetPacket = NULL;
	EXT_FRAME_HEAD		*pExtHead = NULL;

	int iserror = 0;

	//ljm 2010-03-08 判断通道号不能小于0 且不能大于等于初始化时的通道数
	if(nChnNo >= g_server_info.nChannelNum || nChnNo < 0 || pFrameBuf == NULL || nRealFrmNo == NULL
			|| nowFrmTime == NULL || nLastVideoPos == NULL)
	{
		printf("GetOneVideoFrame Param error %d\n", nChnNo);
		return -1;
	}

	if((pStream = &g_channel_stream[nChnNo]) == NULL ||
			pStream->existChannel == 1)
	{
		printf("GetOneVideoFrame (%d) channel exit\n",nChnNo);
		return -1;
	}

	pNetPacket = pStream->pPoolVideoPacket[pStream->nLastIFrameBegin];
	if (0)
		printf("nLastIFrameBegin:%u;nFrameNo:%u\n",pStream->nLastIFrameBegin,
				pNetPacket->packetHead.nFrameNo);

	if(bSubChn)
		pthread_mutex_lock(&pStream->muVideoWriteReadSub);
	else
		pthread_mutex_lock(&pStream->muVideoWriteRead);	

	//获取帧的位置
	nGetPos = GetOneVideoPos(pStream, nVideoFrmNo, bKeyFrame, bSubChn, nLastVideoPos,
			interMilliSecond, preFrmTime, bLostLastestKey);

	/*printf("get video position :%d;bSubChn:%d;nVideoFrmNo:%d;bKeyFrame:%d\n",
	  nGetPos,bSubChn,
	  nVideoFrmNo,
	  bKeyFrame);*/

	if (nGetPos < 0 || nGetPos >= MAX_POOL_SIZE) //ljm 2010-03-08 add
	{
		nFrameLen = 0; //取不到数据
		//printf("fail to get video frame position;\n");
		goto GET_VIDEO_FRAME_EXIT;
	}		

	if(bSubChn)
		pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacketSub[nGetPos];
	else
		pNetPacket =(NET_PACKET_VIDEO *) pStream->pPoolVideoPacket[nGetPos];

	if (pNetPacket == NULL) //ljm 2010-03-08 add 
	{
		nFrameLen = -5;
		printf("GetOneVideoFrame pNetPacket error;\n");
		goto GET_VIDEO_FRAME_EXIT;	
	}

	pExtHead = (EXT_FRAME_HEAD *)((char *)pNetPacket->packetData + sizeof(GF_FRAME_HEAD));
	nFrameLen = pNetPacket->packetHead.frameHead.nByteNum+sizeof(GF_FRAME_HEAD);
	//检测参数
	/*printf("nNetFlag:%d;nPakcetNo:%d;bKeyFrame:%d;streamFlag:%d;nVideoFrmNo:%d;nFrameNo:%d;\n",
	  pNetPacket->packetHead.nNetFlag,
	  pNetPacket->packetHead.nPakcetNo,
	  bKeyFrame,
	  pNetPacket->packetHead.frameHead.streamFlag,
	  nVideoFrmNo,
	  pNetPacket->packetHead.nFrameNo);*/
	if((GF_MAGIC_FLAG != pNetPacket->packetHead.nNetFlag) ||
			(0             != pNetPacket->packetHead.nPakcetNo)||//必须是视频帧的起始单元号
			(nFrameLen      > nFrameBufSize) ||//获取帧缓存过小
			(nVideoFrmNo    > pNetPacket->packetHead.nFrameNo) ||//帧号比期望的帧号小
			(bKeyFrame && (STREAM_FLAG_VI != pNetPacket->packetHead.frameHead.streamFlag))
			|| !(nRet = CheckTotalFrame(pStream, nGetPos, bSubChn)))
	{
		/*GF_SYSLOG(GF_LOG_ERR,"GetOneVideoFrame check NetPacket error %ld %d %d %d %ld %ld %d %d %d\n",
		  pNetPacket->packetHead.nNetFlag, pNetPacket->packetHead.nPakcetNo,
		  nFrameLen, nFrameBufSize, nVideoFrmNo, 
		  pNetPacket->packetHead.nFrameNo, bKeyFrame, 
		  pNetPacket->packetHead.frameHead.streamFlag, nRet);*/
		/*
		   printf("GetOneVideoFrame check NetPacket error:nPakcetNo:%d;nVideoFrmNo:%d;nFrameNo:%d;bKeyFrame:%d;streamFlag:%d;\n",
		   pNetPacket->packetHead.nPakcetNo,
		   nVideoFrmNo,
		   pNetPacket->packetHead.nFrameNo,
		   bKeyFrame,
		   pNetPacket->packetHead.frameHead.streamFlag);
		   */
		if(1 == CheckTotalFrameNo(pStream,nVideoFrmNo))
		{
			//		printf("GetOneVideoFrame check NetPacket error But CheckTotalFrameNo ok!!!");
		}

		nFrameLen = 0;
		iserror = 1;
		goto GET_VIDEO_FRAME_EXIT;
	}
	//拷贝数据	
	CopyTotalFrame(pStream, pFrameBuf, nGetPos, bSubChn, nLastVideoPos);
	*nRealFrmNo = pNetPacket->packetHead.nFrameNo;
	*nowFrmTime = pExtHead->nTimestamp;

GET_VIDEO_FRAME_EXIT:	
	if(bSubChn)
		pthread_mutex_unlock(&pStream->muVideoWriteReadSub);
	else
		pthread_mutex_unlock(&pStream->muVideoWriteRead);

	//if (iserror)
	//sleep(60);

	return nFrameLen;
}

// rtp服务进程等待数据
void RtpThrdWaitMediaData(void)
{
	CHANNEL_STREAM *pStream = NULL;

	pStream = &g_channel_stream[0];

	if (pStream == 0)
		return;

	pthread_mutex_lock(&pStream->mutexRtpWaitStreamData);
	pthread_cond_wait(&pStream->RtpWaitStreamData,&pStream->mutexRtpWaitStreamData);
	pthread_mutex_unlock(&pStream->mutexRtpWaitStreamData);

}

// 通知接口，主码流
void NotifyRtpRecMainStream(void)
{
	CHANNEL_STREAM *pStream = NULL;
	pStream = &g_channel_stream[0];

	if (IsGetRtpMainStream())
	{
		//printf("NotifyRtpRecMainStream;\n");
		pthread_cond_signal(&pStream->RtpWaitStreamData);

	}

}

// 通知接口，次码流
void NotifyRtpRecSubStream(void)
{

	CHANNEL_STREAM *pStream = NULL;
	pStream = &g_channel_stream[0];

	if (IsGetRtpSubStream())
	{
		//printf("NotifyRtpRecSubStream;\n");
		pthread_cond_signal(&pStream->RtpWaitStreamData);

	}

}

void NotifyRtpRecAudio(void)
{

}


