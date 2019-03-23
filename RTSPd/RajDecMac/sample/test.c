
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>



#include "media_sys_api.h"

#define DEBUG
#include "msa_debug.h"

#include "rtsp_lib.h"




//#define USE_RTSP_STRAEM


#define PLAY_D1_FILE_NAME "/nfs/yb/sample/common/D1chn0.h264"
#define PLAY_1080P_FILE_NAME "1080P.h264"
#define PLAY_1080P_JPG_FILE_NAME "/mnt/mtd/common/1080P.jpg"



typedef enum {

    HI_FALSE    = 0,

    HI_TRUE     = 1,

} HI_BOOL;


char play_file_name[500] = "";
int thread_run_flag = 0;
MSA_HANDLE rtsp_handle = NULL;



int GetFileData(char* file_name,char * buf,int max_len)
{
	FILE *fp=NULL;
	long fileOffset = 0;
	int rel;


	fp = fopen(file_name,"rb");
	if( fp == NULL )
	{
		DPRINTK(" open %s error!\n",file_name);
		goto get_err;
	}

	rel = fseek(fp,0L,SEEK_END);
	if( rel != 0 )
	{
		DPRINTK("fseek error!!\n");
		goto get_err;
	}

	fileOffset = ftell(fp);
	if( fileOffset == -1 )
	{
		DPRINTK(" ftell error!\n");
		goto get_err;
	}

	DPRINTK(" fileOffset = %ld\n",fileOffset);

	rewind(fp);	

	rel = 0;

	/* if minihttp alive than kill it */
	if( fileOffset > 0 )
	{	
		rel = fread(buf,1,max_len,fp);
		if( rel <= 0 )
		{
			DPRINTK(" fread Error!\n");
			goto get_err;
		}	
		
	}

	fclose(fp);

	return rel;

get_err:
	if( fp )
	   fclose(fp);

	return -1;
}



void * SAMPLE_COMM_VDEC_SendStream(void *  pArgs )

{
	
	MSA_HANDLE handle = (MSA_HANDLE)pArgs;
	FILE *fpStrm=NULL;

	unsigned char  *pu8Buf = NULL;	

	HI_BOOL bFindStart, bFindEnd;

	int s32Ret,  i,  start = 0;
	

	int s32UsedBytes = 0, s32ReadLen = 0;

	U64INT u64pts = 0;
	char jpg_buf[1*1024*1024];
	int jpg_size  = 0;

	int  len;   
	int s32MinBufSize;
	static int iThreadNum = 0;
	int iThisThreadId = 0;
	int iCount = 0;

	MSA_DATA_HEAD_ST stDataHead;

	memset(&stDataHead,0x00,sizeof(MSA_DATA_HEAD_ST));

	s32MinBufSize = 1920*1080*3/2;

	jpg_size = GetFileData(PLAY_1080P_JPG_FILE_NAME,jpg_buf,1*1024*1024);

	iThisThreadId = iThreadNum;

	iThreadNum++;



	fpStrm = fopen(play_file_name, "rb");

	if(fpStrm == NULL)

	{

		printf("SAMPLE_TEST:can't open file %s in \n",play_file_name);

		return (void*)-1;

	}

	


	pu8Buf = malloc(s32MinBufSize);

	if(pu8Buf == NULL)

	{

		printf("SAMPLE_TEST:can't alloc %d \n", s32MinBufSize);

		fclose(fpStrm);

		return  (void*)-1;

	}     

	fflush(stdout);

	

	u64pts = 0;

	while (thread_run_flag)
	{
		#ifdef USE_RTSP_STRAEM
		if( iThisThreadId == 0 )
		{
			usleep(10000);
			DPRINTK("iThisThreadId=%d out\n",iThisThreadId);
			return NULL;
		}
		#endif
    

         if (1 )

        {

            bFindStart = HI_FALSE;  

            bFindEnd   = HI_FALSE;

            fseek(fpStrm, s32UsedBytes, SEEK_SET);

            s32ReadLen = fread(pu8Buf, 1, s32MinBufSize,fpStrm);

            if (s32ReadLen == 0)

            {

                if (1)

                {

                    s32UsedBytes = 0;

                    fseek(fpStrm, 0, SEEK_SET);

                    s32ReadLen = fread(pu8Buf, 1, s32MinBufSize, fpStrm);

                }

                else

                {

                    break;

                }

            }

         

            for (i=0; i<s32ReadLen-5; i++)
            {

                if (  pu8Buf[i  ] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 && 

                     ( (pu8Buf[i+3]&0x1F) == 0x5 || (pu8Buf[i+3]&0x1F) == 0x1 ) &&

                     ( (pu8Buf[i+4]&0x80) == 0x80)

                   )                 

                {

                    bFindStart = HI_TRUE;

                    i += 4;

                    break;
                }
            }



            for (; i<s32ReadLen-5; i++)
            {

                if (  pu8Buf[i  ] == 0 && pu8Buf[i+1] == 0 && pu8Buf[i+2] == 1 && 

					( ((pu8Buf[i+3]&0x1F) == 0x7) || ((pu8Buf[i+3]&0x1F) == 0x8) || ((pu8Buf[i+3]&0x1F) == 0x6)

                      || (((pu8Buf[i+3]&0x1F) == 0x5 || (pu8Buf[i+3]&0x1F) == 0x1) &&((pu8Buf[i+4]&0x80) == 0x80))

                    )

                   )

                {

                    bFindEnd = HI_TRUE;

                    break;
                }
            }



            if(i > 0) s32ReadLen = i;

            if (bFindStart == HI_FALSE)
            {
                printf("SAMPLE_TEST:  can not find start code!s32ReadLen %d, s32UsedBytes %d. \n", s32ReadLen, s32UsedBytes);
            }
            else if (bFindEnd == HI_FALSE)
            {
                s32ReadLen = i+5;
            }           

        }        



	{

		stDataHead.enDataType = MSA_VIDEO_H264;	
		//stDataHead.TimeStamp.u64PTS +=  33333;  //每秒30帧的播放速度。
		stDataHead.TimeStamp.u64PTS =  0;  //马上进行解码，不再对应播放速度
		stDataHead.iDataLen = s32ReadLen;

		

		//切换不同的解码数据
		if(( iThisThreadId == 1 ||  iThisThreadId == 3) && jpg_size > 0)	
		{
			//jpeg图像解码，一秒钟最多不能超过15帧，否则机器性能不够。
			//解码会报错。
			stDataHead.enDataType = MSA_PIC_JPEG; 			
			stDataHead.TimeStamp.u64PTS =  0;  //每秒30帧的播放速度。
			stDataHead.iDataLen = jpg_size;
			s32Ret = MSA_SendDecData(handle,stDataHead,jpg_buf);

			if( iCount > 200 )
				iCount = 0;

			usleep(30000);
		}else
		{
			//DPRINTK("send %d \n",stDataHead.iDataLen);
			s32Ret = MSA_SendDecData(handle,stDataHead,pu8Buf + start);
		}

		iCount++;

		
	}

               
        if (MSA_SUCCESS != s32Ret)
        {
		DPRINTK("s32Ret = %d\n",s32Ret);
		usleep(100);

        }
        else
        {

            s32UsedBytes = s32UsedBytes +s32ReadLen + start;			

            u64pts += 1;     
	}

	 usleep(30000);

	}   

	
	fflush(stdout);

	if (pu8Buf != NULL)

	{

        free(pu8Buf);

	}

	fclose(fpStrm);

	iThreadNum = 0;

	return (void *)0;

}

void StopApp(int signo) 
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




	
int rtsp_recv(int fd,char * recv_buf,int len,int type,struct timeval timestamp)
{	
	int ret_no = 1;
	int ret = 0;
	MSA_DATA_HEAD_ST stDataHead;
	U64INT  sec;
	U64INT usec;
	char * head_ptr = NULL;

	//前16 个字节用来填写帧头数据
	head_ptr = recv_buf;
	recv_buf = head_ptr + 16;
	
	//printf("%x fd=%d  size=%d type=%d %d.%d\n",recv_buf,fd,len,type,timestamp.tv_sec,timestamp.tv_usec);

	if( type == RL_DATA_TYPE_VIDEO && thread_run_flag==1 && rtsp_handle != NULL)
	{
		//printf("%x %x %x %x %x %x %x %x\n",recv_buf[0],recv_buf[1],recv_buf[2],recv_buf[3],
		//	recv_buf[4],recv_buf[5],recv_buf[6],recv_buf[7]);

		sec = timestamp.tv_sec;
		usec = timestamp.tv_usec;

		stDataHead.enDataType = MSA_VIDEO_H264;			
		stDataHead.TimeStamp.u64PTS =  sec *1000000 + usec; 	
		stDataHead.iDataLen = len;
	
		ret = MSA_SendDecData(rtsp_handle,stDataHead,recv_buf);
		if( ret < 0 )
		{
			DPRINTK("decode frame len=%d err\n",len);
		}
	}
	
	return ret_no;
err:
	return ret_no;
}

FILE * fpRec = NULL;
FILE * fpDownload = NULL;
int iDownloadFileNum = 0;
int iWriteFrameNum =100;


int enc_data_callback(MSA_HANDLE  hHandle,char * pVencBuf,MSA_H264VIDEO_HEADER stHeader)
{
	//DPRINTK("%d len=%d\n",stHeader.iskeyframe,stHeader.datalen);
	//printf("%x %x %x %x %x %x %x %x\n",pVencBuf[0],pVencBuf[1],pVencBuf[2],pVencBuf[3],
	//		pVencBuf[4],pVencBuf[5],pVencBuf[6],pVencBuf[7]);

	if( fpRec == NULL  && iWriteFrameNum > 0)
		{
			fpRec = fopen("1.h264","wb");
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
				DPRINTK("write data err\n");

				fclose(fpRec);
				fpRec = NULL;

				DPRINTK("close 1.h264\n");

				return -1;
				
			}else
			{
				//DPRINTK("write frame data %d\n",iWriteFrameNum);
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
	return 1;
}


int draw_rectangle(MSA_CHANNEL_ST stChannel)
{	
	int style = 3;
	stChannel.stWindow.iWidth = stChannel.stWindow.iWidth - style*2;
	stChannel.stWindow.iHeight = stChannel.stWindow.iHeight - style;

	DPRINTK("%d,%d  %d,%d\n",stChannel.stWindow.iPosStartX,stChannel.stWindow.iPosStartY,
		stChannel.stWindow.iPosStartX+ stChannel.stWindow.iWidth,stChannel.stWindow.iPosStartY + stChannel.stWindow.iHeight);

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


int main(int argc,char **argv )
{
	MSA_INIT_DEV_ST stInitDev;
	MSA_HANDLE handle[4];
	MSA_CHANNEL_ST stChannel;
	MSA_WINDOW_ST stWindow;
	MSA_CHANNEL_ST stChannel1;
	MSA_CHANNEL_ST stChannel3;
	MSA_WINDOW_ZOOM_IN_ST stZoomInWindow;
	MSA_ENCODE_SET_ST stEncodeSet;
	GUI_DRAW_ITEM * pGuiItem = NULL;
	MSA_DRAW_OSD_PIC_ST stDrawOsd;
	MSA_OSD_LAYER_INFO stLayerInfo;
	int ret = 0;
	int i = 0;
	pthread_t   VdecThread[4];
	RTSP_INFO rtsp;
	int rtsp_fd = -1;
	int screen_w = 0;
	int screen_h = 0;
	int pic_w = 0;
	int pic_h = 0;

	//signal(SIGINT, StopApp);
	screen_w = 1024;
	screen_h = 768;

	memset(&stInitDev,0x00,sizeof(stInitDev));
	stInitDev.stOutputDevSetParameters.iOutputDevNum = 1;
	stInitDev.stOutputDevSetParameters.enOutputScreenMode[0]  = MSA_VO_OUTPUT_1024x768_60;
	//stInitDev.stOutputDevSetParameters.enOutputScreenMode[1]  = MSA_VO_OUTPUT_1024x768_60;	
	stInitDev.stOutputDevSetParameters.iDevIntfType[0] =  MSA_VO_INTF_VGA|MSA_VO_INTF_HDMI;
	//stInitDev.stOutputDevSetParameters.iDevIntfType[1] =  MSA_VO_INTF_HDMI;
	stInitDev.stOutputDevSetParameters.iDevCombineMode[0] = MSA_HARDWARE_COMBINE_MODE;
	//stInitDev.stOutputDevSetParameters.iDevCombineMode[1] = MSA_SOFTWARE_COMBINE_MODE;
	stInitDev.stOutputDevSetParameters.iPipLayerBindDevId = MSA_HD0; //画中画层绑定到0 输出口。

	//场景1.
	//高清输出通道 0 ，采用硬件进行放缩，并且能进行画中画显示
	//高清输出通道1,采用软件进行放缩，并且能进行画中画显示
/*	stInitDev.stOutputDevSetParameters.enOutputScreenMode[0]  = MSA_VO_OUTPUT_1440x900_60;
	stInitDev.stOutputDevSetParameters.iDevIntfType[0] = MSA_VO_INTF_HDMI;
	stInitDev.stOutputDevSetParameters.iDevCombineMode[0] = MSA_HARDWARE_COMBINE_MODE;
	stInitDev.stOutputDevSetParameters.iPipLayerBindDevId = MSA_HD0;

	stInitDev.stOutputDevSetParameters.enOutputScreenMode[1]  = MSA_VO_OUTPUT_1440x900_60;	
	stInitDev.stOutputDevSetParameters.iDevIntfType[1] = MSA_VO_INTF_VGA ;	
	stInitDev.stOutputDevSetParameters.iDevCombineMode[1] = MSA_SOFTWARE_COMBINE_MODE;
	*/

	//场景2.
	//高清输出通道 0 ，采用硬件进行放缩，不进行画中画显示
	//高清输出通道1,      采用硬件进行放缩，不进行画中画显示
	//在两个输出通道上创建的窗口都不能重合交错。
/*	stInitDev.stOutputDevSetParameters.enOutputScreenMode[0]  = MSA_VO_OUTPUT_1440x900_60;
	stInitDev.stOutputDevSetParameters.iDevIntfType[0] = MSA_VO_INTF_HDMI;
	stInitDev.stOutputDevSetParameters.iDevCombineMode[0] = MSA_HARDWARE_COMBINE_MODE;
	stInitDev.stOutputDevSetParameters.iPipLayerBindDevId = MSA_HD0;

	stInitDev.stOutputDevSetParameters.enOutputScreenMode[1]  = MSA_VO_OUTPUT_1440x900_60;	
	stInitDev.stOutputDevSetParameters.iDevIntfType[1] = MSA_VO_INTF_VGA ;	
	stInitDev.stOutputDevSetParameters.iDevCombineMode[1] = MSA_HARDWARE_COMBINE_MODE;
*/

	ret = MSA_InitSystem(stInitDev);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MSA_InitSystem err=%d\n",ret);
		goto end;
	}
	
	ret = MSA_InitSystem(stInitDev);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MSA_InitSystem err=%d\n",ret);
		goto end;
	}

	MSA_DestroySystem();
	
	ret = MSA_InitSystem(stInitDev);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MSA_InitSystem err=%d\n",ret);
		goto end;
	}
	
	if( MSA_GuiOpenPicData("1.guid",MSA_HD0) < 0 )
		DPRINTK("MSA_GuiOpenPicData err\n");

	//在高清0通道上创建一个底层显示窗口。
	stChannel.enOutputDev = MSA_HD0;
	stChannel.enShowLayer = MSA_PIP_LAYER; //当 MSA_HD0 设置为MSA_HARDWARE_COMBINE_MODE时并且iPipLayerBindDevId= MSA_HD0 时，
										   // MSA_BASE_LAYER 层将使用PIP 层来显示
	stChannel.stWindow.iPosStartX = 0;
	stChannel.stWindow.iPosStartY = 0;
	stChannel.stWindow.iWidth = screen_w;
	stChannel.stWindow.iHeight = screen_h;
	ret = MSA_CreateChanWindow(stChannel,&handle[0]);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MSA_CreateChanWindow err=%d\n",ret);
		goto end;
	}

	MAS_ChanWindowSet(handle[0],stChannel.stWindow);
	//对窗口画边框
	draw_rectangle(stChannel);
	//在输出设备上显示OSD
	DPRINTK("draw text\n");
	//ScreenText(MSA_HD0,0,0,"TESTWORLD");

#ifdef USE_RTSP_STRAEM
	memset(&rtsp,0x00,sizeof(rtsp));
	strcpy(rtsp.user,"admin");
	strcpy(rtsp.pass,"admin");	
	strcpy(rtsp.rtsp_addr,"rtsp://192.168.2.55/0");
	
	rtsp_handle = handle[0];
	DPRINTK("start rtsp %s\n",rtsp.rtsp_addr);
	rtsp_fd = rl_open_rtsp(rtsp,rtsp_recv);	
#endif

	thread_run_flag = 1;
	for(i=0; i<1; i++)
	{		
		strcpy(play_file_name,PLAY_1080P_FILE_NAME);		
	    pthread_create(&VdecThread[i], 0, SAMPLE_COMM_VDEC_SendStream, (void *)handle[i]);
		usleep(50000);
	}

	sleep(1);

	//必须解码出至少一副图像后，调用此函数才能获得正确的值。
	ret = MSA_GetDecodePicSize(handle[0],&pic_w,&pic_h);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MSA_GetDecodePicSize err=%d\n",ret);
		goto end;
	}

	DPRINTK("get decode pic size (%d.%d)\n",pic_w,pic_h);

	// 创建窗口压缩引擎
	stEncodeSet.enRc = VENC_RC_CBR;
	stEncodeSet.iBitrate = 4096;
	stEncodeSet.iFrameRate = 30;
	stEncodeSet.iGop = 30;
	stEncodeSet.iWidth = 1920;
	stEncodeSet.iHeight = 1072;
	stEncodeSet.DataCallback = enc_data_callback;

	ret = MAS_ChanWindowCreateEncode(handle[0],stEncodeSet);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MAS_ChanWindowCreateEncode err=%d\n",ret);
		goto end;
	}


	//创建窗口OSD 层
	ret = MAS_ChanWindowCreateOSDLayer(handle[0]);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MAS_ChanWindowCreateOSDLayer err=%d\n",ret);
		goto end;
	}	


	//获取图片数据
	pGuiItem = MSA_GetPicDataByName("25051106",MSA_HD0);
	if(pGuiItem == NULL )
	{
		DPRINTK("MSA_GetPicDataByName err no this pic\n");
		return -1;	
	}	

	MAS_GetOSDLayerInfo(handle[0],&stLayerInfo);
	DPRINTK("layer: %d,%d %d\n",stLayerInfo.layer_width,stLayerInfo.layer_height,stLayerInfo.enPixelFmt);

	if( pGuiItem->draw_mode == GUI_PIC_DATA)
	{
		stDrawOsd.pic_data_ptr = pGuiItem->data_buf;
		stDrawOsd.pic_width = pGuiItem->w;
		stDrawOsd.pic_height = pGuiItem->h;
		stDrawOsd.screen_x = 100;
		stDrawOsd.screen_y = 100;
		stDrawOsd.pic_show_offset_x = 0;
		stDrawOsd.pic_show_offset_y = 0;
		stDrawOsd.pic_show_w = 200;
		stDrawOsd.pic_show_h  = 100;

		memset(stDrawOsd.pic_data_ptr,0x7f,stDrawOsd.pic_width*2*50);

		DPRINTK("draw pic\n");

		//对窗口OSD 层进行贴图操作
		//图像数据是A1555格式，像素点最高为0时显示，为1时不显示。
		ret = MSA_OSDLayerDrawPic(handle[0],stDrawOsd);
		if( ret != MSA_SUCCESS )
		{
			DPRINTK("MSA_OSDLayerDrawPic err=%d\n",ret);
			goto end;
		}

	}else
	{
		DPRINTK("Picture mode is wrong!\n");
	}
	
		
	//while(thread_run_flag)
	{
		sleep(10);		
		DPRINTK("hide [0] window\n");
		ret = MSA_ShowChanWindow(handle[0],MSA_WINDOW_HIDE);
		if( ret != MSA_SUCCESS )
		{
			DPRINTK("MSA_ShowChanWindow err=%d\n",ret);		
		}
		sleep(5);
		ret = MSA_ShowChanWindow(handle[0],MSA_WINDOW_SHOW);
		if( ret != MSA_SUCCESS )
		{
			DPRINTK("MSA_ShowChanWindow err=%d\n",ret);		
		}
		
		sleep(20);

		MSA_OSDLayerDrawClear(handle[0]);

		while(1)
		    sleep(5);
	}	

end:
	ret = MAS_ChanWindowDestroyOSDLayer(handle[0]);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MAS_ChanWindowDestroyOSDLayer err=%d\n",ret);		
	}

	ret = MAS_ChanWindowDestroyEncode(handle[0]);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MAS_ChanWindowDestroyEncode err=%d\n",ret);		
	}	
	
	thread_run_flag = 0;
	sleep(1);	

	ret = MSA_DestroyChanWindow(handle[0]);
	if( ret != MSA_SUCCESS )
	{
		DPRINTK("MSA_DestroyChanWindow err=%d\n",ret);		
	}
	
	#ifdef USE_RTSP_STRAEM
	if( rtsp_fd >= 0 )
	{
		rl_close_rtsp(rtsp_fd);
		rtsp_fd = -1;
	}
	#endif
	MSA_DestroySystem();	
	return ret;
}



