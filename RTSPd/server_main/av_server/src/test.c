/*
   stream.c
   */


#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/poll.h>

#include "global.h"
#include "rtsp_server_lib.h"


#include "media_sys_api.h"

#define DEBUG
#include "msa_debug.h"

#include "rtsp_lib.h"

#include "GFNetApi.h"

#include "init_system.h"
#include "br_keyboard.h"

#define PLAY_1080P_JPG_FILE_NAME "/mnt/mtd/common/1080P.jpg"

#define BR_MIN(a, b)   ((a) < (b) ? (a) : (b))

#define MAX_VIDEO_FRAME_SIZE        (1024 * 1024)
//static char *pVEenData = NULL;

#define STREAM_NUM          4
#define STREAM_TIMEOUT      (10) //s

typedef enum {
	HI_FALSE    = 0,
	HI_TRUE     = 1,
} HI_BOOL;

static unsigned int show_chan = STREAM_NUM;
pthread_mutex_t mutex;

typedef enum {
	S_UNINIT    = 0,
	S_UNLINK    = 1,
	S_LINKED    = 2,
	S_GETVD     = 3,
	S_WORDWELL  = 4,
	S_LINKDOWN  = 5,
} SYS_STATUS;

struct rtspc_stream {
	int idx;

	SYS_STATUS status; 
	// 0:未初始化
	// 1:连接未创建  
	// 2:已创建连接
	// 3:以获取到有效视频数据
	// 4:稳定工作中
	// 5:断开连接

	MSA_HANDLE handle;
	MSA_CHANNEL_ST stChannel;

	int rtsp_fd;
	RTSP_INFO rtsp;//rtsp client paramers
	int have_vd;

	MSA_ENCODE_SET_ST stEncodeSet;//编码参数
	int encode_create;

	int timer;

	char *pVEenData;
};

struct rtspc_stream rc_stream[STREAM_NUM];

char play_file_name[500] = "";
int thread_run_flag = 1;
MSA_HANDLE rtsp_handle = NULL;

extern void lprint(char *origin, char *new_string, char *old_string, unsigned int BitbmpPix_X, int x, int y, char *buffer);

int  GF_VENC_RequestIFrame(unsigned char nCh, unsigned char nMinEnc, unsigned char nIFrameCount)
{
	return 0;	
}

static unsigned int  get_string(char *pDes, char *pSrc, unsigned int w)
{
    unsigned int ret = w;
    int i = 0;

    for (i = 0; i < w; i++) {
        if (pSrc[i] & 0x80) {
            if (i + 1 >= w) {
                ret--;
                return ret;
            } else
            	pDes[i] = pSrc[i];
			
            i++;
            pDes[i] = pSrc[i];
        } else {
            pDes[i] = pSrc[i];
        }
    }
    return ret;
}

static int get_stream_video_width(unsigned int stream_id)
{
	if (stream_id >= STREAM_NUM)
		return 0;

    if (rc_stream[stream_id].status != S_WORDWELL)
            return 0;

	return rc_stream[stream_id].stEncodeSet.iWidth;
}

static int get_stream_video_heigt(unsigned int stream_id)
{
	if (stream_id >= STREAM_NUM)
		return 0;

	return rc_stream[stream_id].stEncodeSet.iHeight;
}

void Stop_rtspclient(int signo) 
{
	thread_run_flag = 0;
}

int ScreenText(MSA_OUTPUT_DEV enDev,int iStartX,int iStartY,char * pShowWord)
{
	int iTextLen = 0;
	int i = 0;
	int width_word = 9;
	int height_word = 10;
	int word_num = 0;
	int pos_x = 0;
	int pos_y = 0;
	int word_width = 32;
	int word_height = 32;
	int ret;

	iTextLen = strlen(pShowWord);

	for( i = 0; i < iTextLen ; i++)
	{
		word_num = pShowWord[i] - 65;
		pos_y = word_num / width_word;
		pos_x = word_num % width_word;		

		ret = MSA_GuiDrawPicUseCoordinate("25051106",enDev,iStartX+i*word_width,iStartY,pos_x*word_width,\
				pos_y*word_height,word_width,word_height);
		if( ret < 0 )
		{
			DPRINTK("MSA_GuiDrawPicUseCoordinate err %d,%d\n",pos_x,pos_y);
		}

	}
}

/**
  * 
  */
int rtsp_recv(int fd, char* recv_buf, int len, int type, struct timeval timestamp)
{	
	int ret_no = 1;
	int ret = 0;
	MSA_DATA_HEAD_ST stDataHead;
	U64INT  sec;
	U64INT usec;
	char * head_ptr = NULL;

	int idx = 0;
	for (idx = 0; idx < STREAM_NUM; idx++) {
		if (rc_stream[idx].rtsp_fd == fd)
			break;
	}
	if (idx == STREAM_NUM) {
		DPRINTK("not found stream !!!\n");
		return 0;
	}

	//前16 个字节用来填写帧头数据
	head_ptr = recv_buf;
	recv_buf = head_ptr + 16;

	if ((type == RL_DATA_TYPE_VIDEO) 
			&& (thread_run_flag == 1) 
			&& (rc_stream[idx].handle != NULL)) {
		sec = timestamp.tv_sec;
		usec = timestamp.tv_usec;

		stDataHead.enDataType = MSA_VIDEO_H264;
		stDataHead.TimeStamp.u64PTS =  sec *1000000 + usec;
		stDataHead.iDataLen = len;

		ret = MSA_SendDecData(rc_stream[idx].handle, stDataHead, recv_buf);
		if( ret < 0 )
		{
			DPRINTK("decode frame len=%d err\n",len);
		}

		rc_stream[idx].have_vd = 1;
		if (rc_stream[idx].timer > 0)
			rc_stream[idx].timer--;
	}

	return ret_no;
}

FILE * fpRec = NULL;
FILE * fpDownload = NULL;
int iDownloadFileNum = 0;
int iWriteFrameNum =300;

/*
  * 视频编码器完成后的回调函数，返回为视频编码后的数据
  */
int enc_data_callback(MSA_HANDLE hHandle, char * pVencBuf, MSA_H264VIDEO_HEADER stHeader)
{    
	static int i = 0;
	int idx = 0;
	long nInterv = 0;
	int nStep = 0;
	static struct timeval ppre_time;
	static int TimeStamp = 0;

	//char log[256];

	for (idx = 0; idx < STREAM_NUM; idx++) {
		if (hHandle == rc_stream[idx].handle)
			break;
	}
	//sprintf(log, "echo \"idx: %d, entry data callback !!\" >> log.t", idx);
	//system(log);
	if (idx == STREAM_NUM) {
		DPRINTK("not found stream !!!\n");
		return 0;
	}

	if (ppre_time.tv_sec != 0) {
		nInterv = 0;
	} else {
		nInterv = (stHeader.timestamp.tv_sec - ppre_time.tv_sec - 1) * 1000000
			+ 1000000 + ppre_time.tv_usec - stHeader.timestamp.tv_usec;
	}
	/* 1s ->30frame, 33 ms/frame,  */
	nStep = nInterv / 33;
	if (nStep < 1)
		nStep = 1;
	if (nStep > 50)
		nStep = 50;

	TimeStamp += nStep;

	int pos = 12 + sizeof(EXT_FRAME_HEAD);
	memcpy(rc_stream[idx].pVEenData + pos, pVencBuf, stHeader.datalen);

	
	//sprintf(log, "echo \"idx: %d, write date to rtsp buffer !!\" >> log.t", idx);
	//system(log);
	//printf("enc_data_callback --> data len: %d\n", stHeader.datalen);

	FillFrameHeader(idx, 
			(rc_stream[idx].stEncodeSet.iWidth == 1920), 
			rc_stream[idx].pVEenData, 
			(stHeader.iskeyframe == 1) ? FRAME_FLAG_VI : FRAME_FLAG_VP,
			TimeStamp,
			stHeader.datalen + sizeof(EXT_FRAME_HEAD),
			stHeader.timestamp.tv_sec * 1000 + stHeader.timestamp.tv_usec / 1000);

	GFSNet_WriteFrame(idx, rc_stream[idx].pVEenData, NULL);

	ppre_time.tv_sec = stHeader.timestamp.tv_sec;
	ppre_time.tv_usec = stHeader.timestamp.tv_usec;

#if 0
	if( fpRec == NULL  && iWriteFrameNum > 0)
	{
		fpRec = fopen("/nfs/1.h264","wb");
		if( fpRec )
			DPRINTK("create 1.h264\n");
	}

	if( fpRec  && iWriteFrameNum > 0 )
	{
		int ret = 0;
		int iFrameLen =0;

		ret = fwrite(pVencBuf,1,stHeader.datalen,fpRec);
		if( ret != stHeader.datalen )
		{
			//				DPRINTK("write data err\n");
		}else
		{
			//				DPRINTK("write frame data %d\n",iWriteFrameNum);
		}

		iWriteFrameNum--;

		if( iWriteFrameNum <= 0 )
		{
			fclose(fpRec);
			fpRec = NULL;

			DPRINTK("close 1.h264\n");
			//exit(0);
		}
	}
#endif

	return 1;
}


int draw_rectangle(MSA_CHANNEL_ST stChannel)
{	
	int style = 3;
	stChannel.stWindow.iWidth = stChannel.stWindow.iWidth - style;
	stChannel.stWindow.iHeight = stChannel.stWindow.iHeight - style;

	MSA_GuiDrawLine(stChannel.enOutputDev,stChannel.stWindow.iPosStartX,stChannel.stWindow.iPosStartY, 
			stChannel.stWindow.iPosStartX + stChannel.stWindow.iWidth,stChannel.stWindow.iPosStartY,style,0x0cf0);

	MSA_GuiDrawLine(stChannel.enOutputDev,stChannel.stWindow.iPosStartX+ stChannel.stWindow.iWidth,stChannel.stWindow.iPosStartY, 
			stChannel.stWindow.iPosStartX + stChannel.stWindow.iWidth,stChannel.stWindow.iPosStartY + stChannel.stWindow.iHeight + style,style,0x0cf0);

	MSA_GuiDrawLine(stChannel.enOutputDev,stChannel.stWindow.iPosStartX + stChannel.stWindow.iWidth,stChannel.stWindow.iPosStartY + stChannel.stWindow.iHeight, 
			stChannel.stWindow.iPosStartX ,stChannel.stWindow.iPosStartY + stChannel.stWindow.iHeight,style,0x0cf0);

	MSA_GuiDrawLine(stChannel.enOutputDev,stChannel.stWindow.iPosStartX ,stChannel.stWindow.iPosStartY + stChannel.stWindow.iHeight, 
			stChannel.stWindow.iPosStartX,stChannel.stWindow.iPosStartY,style,0x0cf0);

	return 1;
}

int clear_rectangle(MSA_CHANNEL_ST stChannel)
{
	MSA_ClearOsd(stChannel.stWindow.iPosStartX,stChannel.stWindow.iPosStartY,
			stChannel.stWindow.iWidth,stChannel.stWindow.iHeight,stChannel.enOutputDev);
	return 1;
}

#define HIGH_BASE   50
#define HIGH_STEP   8
#define HIGH_MAX    19
void processOSD(MSA_HANDLE hd, char *buf)
{
	int idx = 0;
	MSA_HANDLE handle;
	MSA_DRAW_OSD_PIC_ST stDrawOsd;

	//    FILE *fd;

	unsigned int x, y;
	unsigned int w, h;
	char *p;
	int ret;
	int i = 0;

	x = *(unsigned int *)buf;
	y = *((unsigned int *)buf + 1);
	w = *((unsigned int *)buf + 2);
	h = *((unsigned int *)buf + 3);
	p = (char *)((unsigned int *)buf + 4);

//	printf("7. x: %d, y: %d, w: %d, h: %d\n", x, y, w, h);
	/*
	   fd = fopen("/nfs/s.bit", "wb");
	   if (fd != NULL) {
	   fwrite(buf, 1, 16 + w * h * 2, fd);
	   fclose(fd);
	   }
	   */
	idx = x >> 16;
	x &= 0xffff;
//    printf("idx = %d\n", idx);
	if ((idx >= STREAM_NUM) || (rc_stream[idx].encode_create == 0))
		return;
	handle = rc_stream[idx].handle;

	y &= 0xffff;
	if (y > HIGH_MAX)
		y = 1;
    if (y > 0)
        y--;

	stDrawOsd.pic_data_ptr = p;
	stDrawOsd.pic_width = w;
	stDrawOsd.pic_height = h;
	stDrawOsd.pic_show_offset_x = 0;
	stDrawOsd.pic_show_offset_y = 0;
	stDrawOsd.pic_show_w = w;
	stDrawOsd.pic_show_h  = h;
	/*
	   memset(buf, 1, w * h);
	   memset(buf + w * h, 0, w * h);
	   */
//	for (i = 0; i < 1; i++) {

		stDrawOsd.screen_x = x;
		stDrawOsd.screen_y = y *(h + HIGH_STEP) + HIGH_BASE;// + i * (h + 5);

//        if (stDrawOsd.screen_y > get_stream_video_heigt(idx) - h)
//                return;

//	printf("8. x: %d, y: %d, w: %d, h: %d\n", x, y, w, h);
		if (w == 0 || h == 0) {
			ret = MSA_OSDLayerDrawClear(handle);
			if (ret != MSA_SUCCESS)
				DPRINTK("MSA_OSDLayerDrawPic err=%d\n",ret);
		} else {
			ret = MSA_OSDLayerDrawPic(handle,stDrawOsd);
			if (ret != MSA_SUCCESS)
				DPRINTK("MSA_OSDLayerDrawPic err=%d\n",ret);
		}
		//	sleep(2);
//	}

	return;
}



#define SVR_PORT        20051

#define ZF_W_NOW 		24
#define ZF_H_NOW 		48
#define ZIFU_H			ZF_H_NOW
#define ZIFU_W			ZF_W_NOW
#define ZIFU_N			80
#define OSD_MAX_LIST_FD		33
#define PRE_BYTE_ADD 		16
#define MAX_PIC_LEN     (2 * (ZIFU_W) * (ZIFU_H) * (ZIFU_N) + PRE_BYTE_ADD)
#define MAX_CHAR_NUM	(ZIFU_N * 2 + PRE_BYTE_ADD)

// 1080p：18行，一行 80字符/40汉字
// ascii: len = 40 * 2 * 18。为了有缓冲区间，len * 2 + 40
static char osd_buf[40 * 2 * 18 * 2 + 40];
static char osd_buf_tmp[40 * 2 * 18 * 2 + 40];
static int start_i = 0;
static int end_i = 0;

void *osd_pic_receive(void *parg)
{
	int i;
	int server_fd, client_fd;
	int max_socket_fd = -1;
	int arrClient_fd[OSD_MAX_LIST_FD];
	struct sockaddr_un client_addr;
	int readlen = -1;
	int ret = -1;

	int debug_cnt1 = 0;
	int debug_cnt2 = 0;
	static int s_index = 0;

	char *pMsg_buf = NULL;
	fd_set	readfset;
	int len = 0;
	char *string_char = NULL;
	char *char_mod_buf = NULL;

//	printf("osd_pic --> 2\n");
	pMsg_buf = (char *)malloc(MAX_PIC_LEN * sizeof(char));
	if (NULL == pMsg_buf)
		return;

	char_mod_buf = (char *)malloc(MAX_PIC_LEN * sizeof(char));
	if (NULL == char_mod_buf) {
		free(pMsg_buf);
		return;
	}

	string_char = (char *)malloc(MAX_CHAR_NUM * sizeof(char));
	if (NULL == string_char) {
		free(pMsg_buf);
		free(char_mod_buf);
		return;
	}

//	printf("osd_pic --> 3\n");
	server_fd = create_ipc_svr(SVR_PORT, OSD_MAX_LIST_FD - 1);
	if (server_fd < 0) {
		printf("osd_pic_receive: error create server_fd.\n");

		free(pMsg_buf);
		free(string_char);
		free(char_mod_buf);
		return;
	}

//	printf("osd_pic --> 4\n");
	for(i = 0; i < OSD_MAX_LIST_FD; i++)
		arrClient_fd[i] = 0;

	arrClient_fd[0] = server_fd;

	printf("start...\n");
	while (thread_run_flag) {
		FD_ZERO(&readfset);
		FD_SET(arrClient_fd[0], &readfset);
		max_socket_fd = arrClient_fd[0];
		for (i = 1; i < OSD_MAX_LIST_FD; i++) {
			if (arrClient_fd[i] > 0) {
				FD_SET(arrClient_fd[i], &readfset);
				max_socket_fd = (max_socket_fd < arrClient_fd[i]) ? (arrClient_fd[i]):(max_socket_fd);
			}
		}

		ret = select(max_socket_fd + 1, &readfset, NULL, NULL, NULL);
		if (ret <= 0)
			continue;
			
        printf("osd select out!\n");
        
		if (FD_ISSET(server_fd, &readfset)) {
			len = sizeof(client_addr);
			client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
			if (client_fd < 0)
				continue;

			for (i = 1; i < OSD_MAX_LIST_FD; i++) {
				if (arrClient_fd[i] <= 0){
					arrClient_fd[i] = client_fd;
					break;
				}
			}
			
			if (i == OSD_MAX_LIST_FD) {
				++s_index;
				s_index %= OSD_MAX_LIST_FD;
				s_index = s_index ? s_index : 1;
					
				FD_CLR(arrClient_fd[s_index], &readfset);
				close(arrClient_fd[s_index]);
				arrClient_fd[s_index] = client_fd;
				printf("too much client...\n");
				//continue;
			}

			if(--ret == 0)
				continue;
		}
		
		for (i = 1; i < OSD_MAX_LIST_FD; i++) {
			if (FD_ISSET(arrClient_fd[i], &readfset)) {
				int w, h, x, y, stream_id;
				
				client_fd = arrClient_fd[i];
				readlen = recv(client_fd, (void *)osd_buf_tmp, sizeof(osd_buf_tmp), MSG_DONTWAIT);
				printf("readlen = %d\n", readlen);
				if (readlen <= 0) {
					FD_CLR(client_fd, &readfset);
					printf("close osd socket.\n");
					close(client_fd);
					arrClient_fd[i] = 0;
					continue;
				}
				
				int j;
				for (j = 0; j < readlen; j++) {
					osd_buf[end_i++] = osd_buf_tmp[j];
					end_i %= sizeof(osd_buf);
					if (end_i == start_i) {
						printf("[%s]-%d: err osd buf over_1.\n", __FUNCTION__, __LINE__);
						break;
					}
				}
				
				while (1) {
					int pos = 0;
					unsigned int head = 0;

					for (j = 0; j < 4; j++) {
						int tmp_pos = (start_i + pos + j) % sizeof(osd_buf);

						if (tmp_pos == end_i) {
						    //printf("[%s]-%d err osd buf over_2.\n", __FUNCTION__, __LINE__);
						    break;
						}
						head <<= 8;
						head |= (osd_buf[tmp_pos] & 0xFF);
					}

					if (j != 4) {
					    //printf("[%s]-%d: err_1!\n", __FUNCTION__, __LINE__);
					    break;
					}

					pos += 4;

					if (head != 0xFFFFFFFF) {
						start_i = (start_i + 1) % sizeof(osd_buf);
						printf("[%s]-%d: err_2!\n", __FUNCTION__, __LINE__);
						continue;
					}
					
					for (j = 0; j < 16; j++) {
						int tmp_pos = (start_i + pos + j) % sizeof(osd_buf);
						
						if (tmp_pos == end_i) {
						    printf("[%s]-%d err osd buf over_3.\n", __FUNCTION__, __LINE__);
						    break;
						}
						pMsg_buf[j] = osd_buf[tmp_pos];
					}
					if (j != 16) {
					    printf("[%s]-%d: err_3!\n", __FUNCTION__, __LINE__);
					    break;
					}
					pos += 16;

					x = *((int *)pMsg_buf + 0);
					y = *((int *)pMsg_buf + 1);
					w = *((int *)pMsg_buf + 2);
					h = *((int *)pMsg_buf + 3);
					stream_id = x >> 16;
					x &= 0xffff; 
					y &= 0xffff; 
					
					for (j = 0; j < w; j++) {
						int tmp_pos = (start_i + pos + j) % sizeof(osd_buf);
						
                        if (tmp_pos == end_i)
                            break;
						
                        pMsg_buf[16 + j] = osd_buf[tmp_pos];
					}

					if (j != w) {
					    printf("[%s]-%d: err_4!\n", __FUNCTION__, __LINE__);
					    break;
					}

					pos += w;
					start_i = (start_i + pos) % sizeof(osd_buf);
					//prdintf("s=%d,e=%d\n", start_i, end_i);
					
					if (get_stream_video_width(stream_id) == 0) {
					    printf("[%s]-%d: err_5!\n", __FUNCTION__, __LINE__);
					    break;
					}
					
					w = BR_MIN((get_stream_video_width(stream_id) - x) / ZIFU_W, w); 
					memset(string_char, 0, MAX_CHAR_NUM);
					
					w = get_string(string_char, (char *)(pMsg_buf + PRE_BYTE_ADD), w);				
					*((int *)pMsg_buf + 2) = w * ZIFU_W;
					*((int *)pMsg_buf + 3) = ZIFU_H;

                    y = (y - 1) *(ZIFU_H + HIGH_STEP) + HIGH_BASE;
                    if (y > get_stream_video_heigt(stream_id) - ZIFU_H) {
                        printf("[%s]-%d: err_6!\n", __FUNCTION__, __LINE__);
                        break;
                    }

					lprint((char *)(pMsg_buf + PRE_BYTE_ADD), string_char, NULL, w * ZIFU_W, 0, 0, char_mod_buf);
					processOSD(NULL, pMsg_buf);	
				}
			}
		}
	}

error_avserver:
	if(pMsg_buf != NULL)
		free(pMsg_buf);
		
	free(string_char);
	free(char_mod_buf);

	if(server_fd > 0)
		close(server_fd);

	return;
}


#define RTSPC_STRING_FILE       "/mnt/mtd/iptable.txt"
unsigned int get_rtspc_string(int idx, char *addr, char *user, char *password)
{
	int i = 0;
	FILE *fp = NULL;
	char string_ling[512] = {0};

	//char log[256] = {0};

	if (idx >= SYS_CHANNEL)
		return 1;

	memset(string_ling, 0, 512);

	fp = fopen(RTSPC_STRING_FILE, "r");
	if (fp == NULL) {
		DPRINTK("read iptable file fail\n");
		return 1;
	}

	for (i = 0; i < idx; i++) {
		fgets(string_ling, 512, fp);
	}

	fgets(string_ling, 512, fp);
	sscanf(string_ling, "%s %s %s", addr, user, password);

	fclose(fp);

	if (addr[0] != 'r' || addr[1] != 't' || addr[2] != 's' || addr[3] != 'p' ||
			addr[4] != ':' || addr[5] != '/' || addr[6] != '/') {
		DPRINTK("rtsp addr wrong !!!\n");

		//sprintf(log, "echo \"idx: %d, rtsp addr wrong !!\" >> log.t", idx);
		//system(log);

		return 1;
	}

	//sprintf(log, "echo \"idx: %d, rtsp addr ok, addr: %s !!\" >> log.t", idx, addr);
	//system(log);

	return 0;
}


void get_win_pos(unsigned int idx, int *x, int *y, int *w, int *h)
{
	switch (idx) {
		default:
		case 0:
			*x = 0, *y = 0;
			*w = 512, *h = 384;
			break;
		case 1:
			*x = 512, *y = 0;
			*w = 512, *h = 384;
			break;
		case 2:
			*x = 0, *y = 384;
			*w = 512, *h = 384;
			break;
		case 3:
			*x = 512, *y = 384;
			*w = 512, *h = 384;
			break;
	}
	return;
}

int rtspc_init(int idx, int screen_w, int screen_h)
{
	int i;
	int ret = 0;
	int x,y,w,h;
	unsigned short cmd;

	if (idx >= STREAM_NUM) 
		return;

	cmd = 0x0100 | idx;
    rc_stream[idx].idx = idx;
	rc_stream[idx].status = S_UNINIT;
	rc_stream[idx].have_vd = 0;
	rc_stream[idx].rtsp_fd = -1;
	memset(rc_stream[idx].rtsp.rtsp_addr, 0, sizeof(rc_stream[idx].rtsp.rtsp_addr));
	memset(rc_stream[idx].rtsp.pass, 0, sizeof(rc_stream[idx].rtsp.pass));
	memset(rc_stream[idx].rtsp.user, 0, sizeof(rc_stream[idx].rtsp.user));
	if (get_rtspc_string(idx, rc_stream[idx].rtsp.rtsp_addr, rc_stream[idx].rtsp.user, rc_stream[idx].rtsp.pass)) {
		DPRINTK("get rtspc string failed\n");
		return 0;
	}

	rc_stream[idx].encode_create = 0;
	rc_stream[idx].stEncodeSet.enRc = VENC_RC_CBR;
	rc_stream[idx].stEncodeSet.iBitrate = 4096;
	rc_stream[idx].stEncodeSet.iFrameRate = 30;
	rc_stream[idx].stEncodeSet.iGop = 30;
	rc_stream[idx].stEncodeSet.iWidth = 0;
	rc_stream[idx].stEncodeSet.iHeight = 0;
	
	rc_stream[idx].stEncodeSet.DataCallback = enc_data_callback;

	rc_stream[idx].pVEenData = (char *)malloc(MAX_VIDEO_FRAME_SIZE);
	if (rc_stream[idx].pVEenData == NULL) {
		DPRINTK("malloc vendata buf fail\n");
		return -1;
	}

	get_win_pos(idx, &x, &y, &w, &h);

	//对窗口画边框
	rc_stream[idx].stChannel.stWindow.iPosStartX = x;
	rc_stream[idx].stChannel.stWindow.iPosStartY = y;
	rc_stream[idx].stChannel.stWindow.iWidth = w;
	rc_stream[idx].stChannel.stWindow.iHeight = h;
	rc_stream[idx].stChannel.enOutputDev = MSA_HD0;
	rc_stream[idx].stChannel.enShowLayer = MSA_PIP_LAYER;

	ret = MSA_CreateChanWindow(rc_stream[idx].stChannel, &rc_stream[idx].handle);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MSA_CreateChanWindow err=%d\n",ret);
		free(rc_stream[idx].pVEenData);
		return -1;
	}

	show_chan = STREAM_NUM;
	rc_stream[idx].status = S_UNLINK;
	rc_stream[idx].timer = 0;

	return 0;
}

void show_win(unsigned int chan)
{
	int i = 0;
	MSA_WINDOW_ST win_st;
	MSA_WINDOW_SHOW_E show_en;

	if (pthread_mutex_trylock(&mutex) != 0)
		return;

	if (show_chan < STREAM_NUM) {
		get_win_pos(show_chan, &win_st.iPosStartX, &win_st.iPosStartY, 
				&win_st.iWidth, &win_st.iHeight);
		MAS_ChanWindowSet(rc_stream[show_chan].handle, win_st);
	}
	show_en = MSA_WINDOW_SHOW;
	for (i = 0; i < STREAM_NUM; i++)
		MSA_ShowChanWindow(rc_stream[i].handle, show_en);

	show_chan = chan;
	if (chan >= STREAM_NUM) {
		pthread_mutex_unlock(&mutex);
		return;
	}

	for (i = 0; i < STREAM_NUM; i++) {
		if (i != chan) {
			show_en = MSA_WINDOW_HIDE;
			MSA_ShowChanWindow(rc_stream[i].handle, show_en);
		}
	}

	show_en = MSA_WINDOW_SHOW;
	win_st.iPosStartX = 0;
	win_st.iPosStartY = 0;
	win_st.iWidth = 1024;
	win_st.iHeight = 768;
	MAS_ChanWindowSet(rc_stream[show_chan].handle, win_st);

	pthread_mutex_unlock(&mutex);
	return;
}

void *rtsp_client(void *pArgs)
{
	MSA_INIT_DEV_ST stInitDev;
	int ret = 0;
	int i = 0;
	pthread_t VThread;

	printf("rtsp_client -->\n");
	memset(rc_stream, 0, sizeof(rc_stream));
	memset(&stInitDev, 0x00, sizeof(stInitDev));
	
	stInitDev.stOutputDevSetParameters.iOutputDevNum = 1;
	stInitDev.stOutputDevSetParameters.enOutputScreenMode[0]  = MSA_VO_OUTPUT_1024x768_60;
	stInitDev.stOutputDevSetParameters.iDevIntfType[0] =  MSA_VO_INTF_VGA|MSA_VO_INTF_HDMI;
	stInitDev.stOutputDevSetParameters.iDevCombineMode[0] = MSA_HARDWARE_COMBINE_MODE;
	stInitDev.stOutputDevSetParameters.iPipLayerBindDevId = MSA_HD0;

	pthread_mutex_init(&mutex, NULL);
	
	/* 从/mnt/mtd/netkeyboard_id.cfg 获取硬件版本号 */
	BRLib_Keyboard_Init(0);

	ret = MSA_InitSystem(stInitDev);
	if(ret != MSA_SUCCESS)
	{
		DPRINTK("MSA_InitSystem err=%d\n", ret);
		goto end_1;
	}

	printf("rtsp_client --> 1\n");
	for (i = 0; i < STREAM_NUM; i++) {
		if (rtspc_init(i, 1024, 768) < 0) {
			goto end_2;
		}
	}

	printf("rtsp_client --> 2\n");
	pthread_create(&VThread, 0, osd_pic_receive, NULL);

	BRLib_Keyboard_Start(show_win);
	set_chn1_down();
	set_chn2_down();

	while (thread_run_flag) {
		for (i = 0; i < STREAM_NUM; i++) {
			switch (rc_stream[i].status) {
				default:
				case S_UNINIT:
					break;

				case S_UNLINK:
					printf("start rtsp %s\n",rc_stream[i].rtsp.rtsp_addr);
					rc_stream[i].rtsp_fd = rl_open_rtsp(rc_stream[i].rtsp, rtsp_recv);
					if (rc_stream[i].rtsp_fd >= 0)
						rc_stream[i].status = S_LINKED;
					break;

				case S_LINKED:
					if (rc_stream[i].timer > STREAM_TIMEOUT)
						rc_stream[i].status = S_LINKDOWN;
					else
						rc_stream[i].timer++;

					if (rc_stream[i].have_vd) {
						MSA_GetDecodePicSize(rc_stream[i].handle, 
								&rc_stream[i].stEncodeSet.iWidth, 
								&rc_stream[i].stEncodeSet.iHeight);
						
						if (rc_stream[i].stEncodeSet.iWidth != 0)
							rc_stream[i].status = S_GETVD;
					}
					break;

				case S_GETVD:
					if ((rc_stream[i].stEncodeSet.iWidth != 0) 
							&& (rc_stream[i].encode_create == 0)) {
						ret = MAS_ChanWindowCreateEncode(rc_stream[i].handle, rc_stream[i].stEncodeSet);
						if( ret != MSA_SUCCESS )
						{
							DPRINTK("MAS_ChanWindowCreateEncode err=%d\n",ret);
							goto end_4;
						}
						ret = MAS_ChanWindowCreateOSDLayer(rc_stream[i].handle);
						if( ret != MSA_SUCCESS )
						{
							DPRINTK("MAS_ChanWindowCreateOSDLayer err=%d\n",ret);
							goto end_4;
						}

						rc_stream[i].encode_create = 1;
						rc_stream[i].status = S_WORDWELL;
						set_image_hw(i, rc_stream[i].stEncodeSet.iWidth, 
								rc_stream[i].stEncodeSet.iHeight);
					}
					rc_stream[i].timer++;
					break;

				case S_WORDWELL:
					if (i == 0)
						set_chn1_up();
					if (i == 1)
						set_chn2_up();

					if (rc_stream[i].timer > STREAM_TIMEOUT)
						rc_stream[i].status = S_LINKDOWN;
					else
						rc_stream[i].timer++;
					break;

				case S_LINKDOWN:
					if (i == 0)
						set_chn1_down();
					if (i == 1)
						set_chn2_down();

					if (rc_stream[i].rtsp_fd >= 0) {
						printf("close channel %d rtsp\n", i);
						rl_close_rtsp(rc_stream[i].rtsp_fd);
						rc_stream[i].timer = 0;
						rc_stream[i].rtsp_fd = -1;
						rc_stream[i].have_vd = 0;
					}
					if (rc_stream[i].encode_create) {
						printf("Destroy channel %d rtsp\n", i);
						MAS_ChanWindowDestroyOSDLayer(rc_stream[i].handle);
						MAS_ChanWindowDestroyEncode(rc_stream[i].handle);
						rc_stream[i].stEncodeSet.iWidth = 0;
						rc_stream[i].encode_create = 0;
					}
					rc_stream[i].status = S_UNLINK;
					break;
			}
		}
		sleep(1);
	}

	thread_run_flag = 0;
	sleep(1);

end_4:
	for (i = 0; i < STREAM_NUM; i++) {
		if (rc_stream[i].rtsp_fd < 0)
			continue;
		MAS_ChanWindowDestroyOSDLayer(rc_stream[i].handle);
		MAS_ChanWindowDestroyEncode(rc_stream[i].handle);
		rl_close_rtsp(rc_stream[i].rtsp_fd);
		rc_stream[i].rtsp_fd = -1;
	}

end_3:
	i = 3;
	pthread_join(VThread, NULL);

end_2:
	while (i) {
		if (rc_stream[i].pVEenData != NULL) {
			free(rc_stream[i].pVEenData);
			rc_stream[i].pVEenData = NULL;
		}
		MSA_DestroyChanWindow(rc_stream[i].handle);
		i--;
	}

end_1:
	MSA_DestroySystem();

	BRLib_Keyboard_Exit();
	return NULL;
}



