#ifndef _RTSP_LIB_HEAD_H_
#define _RTSP_LIB_HEAD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RTSP_INFO_
{
	char rtsp_addr[256];
	char user[40];
	char pass[40];
}RTSP_INFO;

#define RL_DATA_TYPE_VIDEO  1
#define RL_DATA_TYPE_AUDIO 2

int rl_open_rtsp(RTSP_INFO rtsp,int (*Recv)(int ,char * ,int ,int ,struct timeval));
int rl_close_rtsp(int fd);

#ifdef __cplusplus
}
#endif

#endif 

