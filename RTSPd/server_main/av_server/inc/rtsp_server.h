#ifndef __RTSP_H_
#define __RTSP_H_
/******************************************************************************
* ��Ȩ��Ϣ��
* ϵͳ���ƣ�
* �ļ����ƣ�rtsp.h
* �ļ�˵������ͷ�ļ�ΪRTSP��Ľӿں���
* ��汾�ţ�V2311

	
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
* �������ƣ�rtsp_unicast_multicast
* ����������RTSP�������ಥ
* ���������nRtspSever		IN		���룺RTSP ���񣬵�ǰ�汾ֻ����һ��RTSP�������Ը�ֵĿǰ�ɺ��ԣ�����0
			bMutlicast		IN		���룺RTSP�������ಥѡ��0=������1=�ಥ������ʱ������Ĳ����ɺ���
			szMulticastIP	IN		���룺RTSP�Ķಥ��ַ
			nMainVideoPort	IN		���룺RTSP�Ķಥ�˿ڣ���Ƶ��������
			nMainAudioPort	IN		���룺RTSP�Ķಥ�˿ڣ���Ƶ��������
			nViceVideoPort	IN		���룺RTSP�Ķಥ�˿ڣ���Ƶ��������
			nViceAudioPort	IN		���룺RTSP�Ķಥ�˿ڣ���Ƶ��������
			nRes			IN		���룺��������
* ����ֵ��  >0		�ɹ�
			<��0	ʧ��
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
