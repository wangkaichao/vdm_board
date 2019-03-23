#ifndef __RTSP_H_
#define __RTSP_H_
/******************************************************************************
* 版权信息：
* 系统名称：
* 文件名称：rtsp.h
* 文件说明：该头文件为RTSP库的接口函数
* 库版本号：V2311

	
******************************************************************************/
//#include "Rtsp_lib.h"

typedef struct Rtsp_TcpUdp_Http_Comm{
	int  Is_Init_Rtspattr;     //rtsp/http and rtsp/tcp/udp can init g_Env  so use this variables can avoid it reinited 
	int  min_use_count[4];        //use in min media stream user count (include rtsp/http user and rtsp/tcp/udp user)
	int  Is_Enable_Min[4];        //if min media stream was enabled
	int  RtspHttp_Is_Running;  //rtsp/http running flag 
	int  rtspTcpUdp_Is_Running;//rtsp/tcp/udp running flag
	//int  Sem_Id;                 //semaphore identifier
}Rtsp_TcpUdp_Http_Comm;



extern Rtsp_TcpUdp_Http_Comm Com_Env;

typedef void	UrlAnalysis(char* url, int *bFile, int *nCh, int *bMain);

void 			handleRtspBySwithEncode(int bMin);

void			Init_Sem_For_Rtsp(void);
//typedef void	UrlAnalysis(char* url, int *bFile, int *nCh, int *bMain);
int				setAvInfor(int nCh, int bMain, int bAudioOpen, char *audioCodec, int aduioSampleRate, int audioPt,char *videoCodec,int videoPt);
void			setUrlCallback(UrlAnalysis *pf_urlAnalysis);

int 			startRtspServer(int rtspPort, int bUserAuth, int bPassive, int mtu, int maxChn);
void 			stopRtspServer();

int 			playRtsp(char *ip, unsigned port, char *file, int chn, int min,int bFile);
void 			stopRtsp(char *ip, unsigned port, char *file, int chn, int min,int bFile);
int 			getRtspStat(char *ip, unsigned port, char *file, int chn, int min,int bFile);

/******************************************************************************
* 函数名称：rtsp_unicast_multicast
* 功能描述：RTSP单播、多播
* 输入参数：nRtspSever		IN		输入：RTSP 服务，当前版本只启动一个RTSP服务，所以该值目前可忽略，输入0
			bMutlicast		IN		输入：RTSP单播、多播选择，0=单播，1=多播，单播时，后面的参数可忽略
			szMulticastIP	IN		输入：RTSP的多播地址
			nMainVideoPort	IN		输入：RTSP的多播端口，视频，主码流
			nMainAudioPort	IN		输入：RTSP的多播端口，音频，主码流
			nViceVideoPort	IN		输入：RTSP的多播端口，视频，次码流
			nViceAudioPort	IN		输入：RTSP的多播端口，音频，次码流
			nRes			IN		输入：保留参数
* 返回值：  >0		成功
			<＝0	失败
********************************************************************************/
int				rtsp_unicast_multicast(int nRtspSever,int bMutlicast,char *szMulticastIP,unsigned short nMainVideoPort,unsigned short nMainAudioPort,unsigned short nViceVideoPort,unsigned short nViceAudioPort,int nRes);



void sysnRtspHBTime(void);



void handleRtspBySwithEncodeEx(int bMin);




int GetRtspLoopState();



void		incomingConnectionHandlerClient(void * instance, int Mask);
long long	our_random64(void); 
void		our_srandom_svr(unsigned int x);
void		WriteDateHandler(void * instance, int Mask);
int			av_base64_decode(unsigned char* out, const char *in, int out_length);
void 		sysnRtspHBTime(void);

int SetRtspDescribeMultiState(int flag);


#endif
