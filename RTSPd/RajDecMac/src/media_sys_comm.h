#ifndef _MDDIA_SYS_COMM_H_
#define _MDDIA_SYS_COMM_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef struct  _MSA_CHANNEL_INFO_ST_
{
	MSA_CHANNEL_ST stChan;
	int iVoChan;	
	int iVoShow;
	int iVdecChan;
	int iVpssChan;
	int iId;
	MSA_INPUT_MEDIA  enVdecMedia;
	int iVencChan;
	MSA_ENCODE_SET_ST stVencSet;
	int  iVencThreadFlag;
	pthread_t hVencThread;
	int iOsdLayerChan;
	int iOsdIsClear;
}MSA_CHANNEL_INFO_ST;


#ifdef __cplusplus
}
#endif

#endif 