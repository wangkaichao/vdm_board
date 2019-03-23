#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <sys/ioctl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <time.h>

#include "hi_comm_venc.h"

#include "media_sys_api.h"
#include "media_sys_comm.h"

#define DEBUG
#include "msa_debug.h"





typedef struct _THREAD_PASS_ST_
{
	int pass_data_over;
	int id;
	void * pass_data;
}THREAD_PASS_ST;


int Hisi_GetVideoEncodeStream(MSA_CHANNEL_INFO_ST * pChan)
{
	int i,j;	
	int VencFd[16] ={0};  	
	fd_set read_fds;
	int ret;
	HI_S32 s32Ret;
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
	VENC_CHN VeChn;
	struct timeval recTime;	
	struct timeval sendStartTime;
	struct timeval sendEndTime;	
	unsigned char * ptr_point = NULL;	
	struct timeval stTimeOut;
	char buffVideoFrame[MAX_VIDEO_BUF_SIZE];
	int maxfd = 0;
	unsigned char * data_ptr = NULL;
	int data_len = 0;
	MSA_H264VIDEO_HEADER stFrameHead;
	static unsigned char u8FrameIndex[10] = {0};

	stTimeOut.tv_sec = 0;
       stTimeOut.tv_usec = 10000;	

	VeChn = pChan->iVencChan;
	   
	maxfd  = HI_MPI_VENC_GetFd(VeChn);

       FD_ZERO(&read_fds);	
	FD_SET(maxfd, &read_fds);		

	ret  = select(maxfd + 1, &read_fds, NULL, NULL, &stTimeOut);	
	if (ret < 0) 
	{
		DPRINTK("select err  fd=%d\n",maxfd);		
		return HI_FAILURE;
	}
	else if (0 == ret) 
	{		
		return HI_FAILURE;
	}
	else
	{					
		
			if (FD_ISSET(maxfd, &read_fds))
			{					
				s32Ret = HI_MPI_VENC_Query(VeChn, &stStat);
				if (s32Ret != HI_SUCCESS) 
				{
					printf("HI_MPI_VENC_Query:0x%x err\n",s32Ret);
					
					return HI_FAILURE;
				}
				
				stStream.pstPack = (VENC_PACK_S*)Debug_Malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
				if (NULL == stStream.pstPack)  
				{
					printf("Debug_Malloc memory err!\n");
					return HI_FAILURE;
				}
				
				stStream.u32PackCount = stStat.u32CurPacks;				
				
				s32Ret = HI_MPI_VENC_GetStream(VeChn, &stStream, HI_FALSE);
				if (HI_SUCCESS != s32Ret) 
				{
					printf("HI_MPI_VENC_GetStream:0x%x\n",s32Ret);
					Debug_Free(stStream.pstPack);
					stStream.pstPack = NULL;
					return HI_FAILURE;
				}				

				
				memset(&stFrameHead,0x00,sizeof(stFrameHead));

				stFrameHead.datalen = 0;

         			gettimeofday(&stFrameHead.timestamp,NULL);

				for (i=0; i< stStream.u32PackCount; i++)
				{	
					stFrameHead.datalen += stStream.pstPack[i].u32Len-stStream.pstPack[i].u32Offset;																
				}

				if( stFrameHead.datalen   > MAX_VIDEO_BUF_SIZE - 40 )
				{
					DPRINTK("VeChn = %d length = %ld too big size ,drop!\n",VeChn,stFrameHead.datalen);					
					
					goto big_venc_data;
				}					


				stFrameHead.datalen = 0;
				ptr_point = buffVideoFrame;
				
				for (i=0; i< stStream.u32PackCount; i++)
				{					

					data_ptr = stStream.pstPack[i].pu8Addr+stStream.pstPack[i].u32Offset;
					data_len = stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset;
				
					memcpy(ptr_point,data_ptr,data_len);

					stFrameHead.datalen += data_len;

					ptr_point += data_len;				
					
				}		

				if( buffVideoFrame[4] == 0x67 )				
					stFrameHead.iskeyframe =1; // 1 is keyframe 0 is not keyframe 					
				else				
					stFrameHead.iskeyframe =0;	

				if( pChan->stVencSet.DataCallback  != 0 )
				{
					pChan->stVencSet.DataCallback(pChan,buffVideoFrame,stFrameHead);
				}			


			big_venc_data:				
				
				s32Ret = HI_MPI_VENC_ReleaseStream(VeChn,&stStream);
				if (s32Ret) 
				{
					printf("HI_MPI_VENC_ReleaseStream:0x%x\n",s32Ret);
					Debug_Free(stStream.pstPack);
					stStream.pstPack = NULL;
					return HI_FAILURE;
				}
				
				Debug_Free(stStream.pstPack);
				stStream.pstPack = NULL;
			}		
		
	}	     
    
	return HI_SUCCESS;

}



void Hisi_StreamWorkThread(void * value)
{	
	THREAD_PASS_ST * thread_st = (THREAD_PASS_ST *)value;	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)thread_st->pass_data;
	int iMediaId = thread_st->id;
	
	
	thread_st->pass_data_over = 1;	

	DPRINTK("iVencChan = %d int\n",pChan->iVencChan);
	while(pChan->iVencThreadFlag)
	{
		Hisi_GetVideoEncodeStream(pChan);		
	}	
	
end:
	DPRINTK("iVencChan = %d out\n",pChan->iVencChan);
	pthread_exit(2);
}



int Hisi_CreateStreamWorkThread(MSA_HANDLE hHandle)
{
	int ret_no = MSA_SUCCESS;
	int ret = 0;
	THREAD_PASS_ST thread_st;
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	memset(&thread_st,0x00,sizeof(thread_st));

	pChan->iVencThreadFlag = 1;
	thread_st.pass_data = pChan;
	thread_st.id = 0;
	
	ret = pthread_create(&pChan->hVencThread,NULL,(void*)Hisi_StreamWorkThread,(void *)&thread_st);
	if ( 0 != ret )
	{
		pChan->iVencThreadFlag = 0;	
		ret_no = MSA_FAILED;
		goto err;
	}	

	while(thread_st.pass_data_over == 0 )
	{
		usleep(10000);
	}
	
	return ret_no;
err:
	return ret_no;
}

int Hisi_DestroyStreamWorkThread(MSA_HANDLE hHandle)
{
	int ret_no = MSA_SUCCESS;
	int ret = 0;
	THREAD_PASS_ST thread_st;
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;


	if( pChan->iVencThreadFlag == 1  )
	{	
		pChan->iVencThreadFlag = 0;		
		pthread_join(pChan->hVencThread,NULL);	
		DPRINTK("pthread_join hVencThread [%d] end\n",pChan->iVencChan);			
	}
	
	return ret_no;
err:
	return ret_no;
}






#ifdef __cplusplus
}
#endif /* __cplusplus */





