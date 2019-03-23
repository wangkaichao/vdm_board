#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "sample_comm.h"
#include "media_sys_api.h"
#include "media_sys_comm.h"
#include "msa_os.h"


#define DEBUG
#include "msa_debug.h"


//#define HD0_HDMI (1)

#define MSA_MAX_CHANNEL_NUM (16)

typedef struct _MSA_CTRL_ST_
{
	MSA_INIT_DEV_ST stInitDev;	
	MSA_CHANNEL_INFO_ST  * pstChanInfo[MSA_MAX_CHANNEL_NUM];
	pthread_mutex_t hOpMutex;
	int iAlreadyCreateChanNum;
	int isInit;
}MSA_CTRL_ST;

static MSA_CTRL_ST gMsaCtrl;
static VO_VIDEO_LAYER_ATTR_S stOutputDevLayerAttr[MSA_MAX_OUTPUT_DEV_NUM]; 
static  VO_VIDEO_LAYER_ATTR_S stPipLayerAttr;

void MSA_OpLock()
{
	pthread_mutex_lock(&gMsaCtrl.hOpMutex);
}

void MSA_OpUnLock()
{
	pthread_mutex_unlock(&gMsaCtrl.hOpMutex);
}

int MSA_CheckIsIdAllowUse(MSA_CTRL_ST * pCtrl,int id)
{
	int i = 0;

	for( i = 0; i < MSA_MAX_CHANNEL_NUM; i++)
	{
		if(  pCtrl->pstChanInfo[i] != NULL )	
		{
			if( pCtrl->pstChanInfo[i]->iId == id  || pCtrl->pstChanInfo[i]->iVdecChan == id )
			{
				DPRINTK("id:%d is use in [%d] [%d-%d]addr\n",id,i,pCtrl->pstChanInfo[i]->iId,pCtrl->pstChanInfo[i]->iVoChan);
				return 0;		
			}
		}
	}

	return 1;
}


int MSA_GetAvailableChanId(MSA_CTRL_ST * pCtrl)
{
	int i = 0;

	if( pCtrl->iAlreadyCreateChanNum >= MSA_MAX_CHANNEL_NUM)
		return MSA_ERR_ALREADY_CREATE_MAX;

	for( i = 0; i < MSA_MAX_CHANNEL_NUM; i++)
	{
		if(  pCtrl->pstChanInfo[i] == NULL )	
		{
			if( MSA_CheckIsIdAllowUse(pCtrl,i) == 1 )
				return i;		
		}
	}

	return MSA_FAILED;
}

int MSA_ComparePos(int  dest_x,int dest_y, int  start_x, int  start_y,int width,int height )
{
	if( dest_x > start_x && dest_x < start_x+ width)
	{
		if( dest_y >= start_y  && dest_y <= start_y+ height)
			return 1;
	}

	if( dest_y > start_y  && dest_y < start_y+ height)
	{
		if( dest_x >= start_x && dest_x <= start_x+ width)			
			return 1;		
	}

	return 0;
}



int MSA_CheckChanParametersIsRight(MSA_CTRL_ST * pCtrl,MSA_CHANNEL_ST stChan)
{
	int i = 0;
	MSA_CHANNEL_ST  * pChan = NULL;
	int ret = 0;
	int x_combine = 0;
	int y_combine = 0;

	//MSA_ERR_CHAN_WINDOW_CREATE_PARAMETERS_ERR


	for( i = 0; i < MSA_MAX_CHANNEL_NUM; i++)
	{
		if(  pCtrl->pstChanInfo[i] != NULL )	
		{
			if(pCtrl->pstChanInfo[i]->stChan.enOutputDev == stChan.enOutputDev  &&
				pCtrl->pstChanInfo[i]->stChan.enShowLayer == stChan.enShowLayer )
			{
				if(  MSA_GetCombineMode(pCtrl->pstChanInfo[i]->stChan.enOutputDev) == MSA_HARDWARE_COMBINE_MODE)
				{
					pChan = &pCtrl->pstChanInfo[i]->stChan;

					ret = MSA_ComparePos(stChan.stWindow.iPosStartX,stChan.stWindow.iPosStartY,
						pChan->stWindow.iPosStartX,pChan->stWindow.iPosStartY,
						pChan->stWindow.iWidth,pChan->stWindow.iHeight);
					if(ret == 1 )
						x_combine = 1;

					ret = MSA_ComparePos(stChan.stWindow.iPosStartX + stChan.stWindow.iWidth,stChan.stWindow.iPosStartY,
						pChan->stWindow.iPosStartX,pChan->stWindow.iPosStartY,
						pChan->stWindow.iWidth,pChan->stWindow.iHeight);
					if(ret == 1 )
						x_combine = 1;

					ret = MSA_ComparePos(stChan.stWindow.iPosStartX+ stChan.stWindow.iWidth,stChan.stWindow.iPosStartY + stChan.stWindow.iHeight,
						pChan->stWindow.iPosStartX,pChan->stWindow.iPosStartY,
						pChan->stWindow.iWidth,pChan->stWindow.iHeight);
					if(ret == 1 )
						x_combine = 1;

					ret = MSA_ComparePos(stChan.stWindow.iPosStartX,stChan.stWindow.iPosStartY + stChan.stWindow.iHeight,
						pChan->stWindow.iPosStartX,pChan->stWindow.iPosStartY,
						pChan->stWindow.iWidth,pChan->stWindow.iHeight);
					if(ret == 1 )
						x_combine = 1;



					ret = MSA_ComparePos(pChan->stWindow.iPosStartX,pChan->stWindow.iPosStartY,
						stChan.stWindow.iPosStartX,stChan.stWindow.iPosStartY,
						stChan.stWindow.iWidth,stChan.stWindow.iHeight);
					if(ret == 1 )
						x_combine = 1;

					ret = MSA_ComparePos(pChan->stWindow.iPosStartX + pChan->stWindow.iWidth,pChan->stWindow.iPosStartY,
						stChan.stWindow.iPosStartX,stChan.stWindow.iPosStartY,
						stChan.stWindow.iWidth,stChan.stWindow.iHeight);
					if(ret == 1 )
						x_combine = 1;


					ret = MSA_ComparePos(pChan->stWindow.iPosStartX + pChan->stWindow.iWidth,pChan->stWindow.iPosStartY+pChan->stWindow.iHeight,
						stChan.stWindow.iPosStartX,stChan.stWindow.iPosStartY,
						stChan.stWindow.iWidth,stChan.stWindow.iHeight);
					if(ret == 1 )
						x_combine = 1;


					ret = MSA_ComparePos(pChan->stWindow.iPosStartX,pChan->stWindow.iPosStartY + pChan->stWindow.iHeight,
						stChan.stWindow.iPosStartX,stChan.stWindow.iPosStartY,
						stChan.stWindow.iWidth,stChan.stWindow.iHeight);
					if(ret == 1 )
						x_combine = 1;				

					if( x_combine )
						goto err;
					
				}
			}				
		}
	}

	return MSA_SUCCESS;
err:
	return MSA_ERR_CHAN_WINDOW_CREATE_PARAMETERS_ERR;
}


int MSA_CheckEnableOp(F_IN MSA_HANDLE hHandle)
{
	int iRetNo = MSA_SUCCESS;
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}

	if( pChan->iVencChan >= 0  ||pChan->iOsdLayerChan >= 0 )
	{
		iRetNo = MSA_ERR_ENC_OR_CHAN_OSD_IS_NOT_CLOSE_ERR;
		goto err;
	}
	
	return iRetNo;
err:
	return iRetNo;
}


int MSA_InsertChanInfo(MSA_CTRL_ST * pCtrl,int id,MSA_CHANNEL_INFO_ST * pInfo)
{
	if(  pCtrl->pstChanInfo[id] != NULL )	
	{
		DPRINTK("This id addr already have chan info\n");
		return MSA_FAILED;
	}

	pCtrl->pstChanInfo[id] = pInfo;

	return MSA_SUCCESS;
}



int MSA_DelChanInfo(MSA_CTRL_ST * pCtrl,int id,MSA_CHANNEL_INFO_ST * pInfo)
{
	if(  pCtrl->pstChanInfo[id] != pInfo )	
	{
		DPRINTK("This id addr not available\n");
		return MSA_FAILED;
	}

	pCtrl->pstChanInfo[id] = NULL;

	return MSA_SUCCESS;
}


int MSA_InitSystem(MSA_INIT_DEV_ST stInitDev)
{
	HI_S32 s32Ret = HI_SUCCESS;
	VB_CONF_S stVbConf;    
	HI_U32 u32BlkSize;
	SIZE_S stSize; 

	VO_DEV VoDev;    
	VO_LAYER VoLayer;
	VO_PUB_ATTR_S stVoPubAttr, stVoPubAttrSD0; 
	SAMPLE_VO_MODE_E enVoMode;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr, stLayerAttrSD0; 
	HI_U32 u32FrameRate;    
	PAYLOAD_TYPE_E enType;   
	int iPipBindDev = 0;


	if (stInitDev.stOutputDevSetParameters.iOutputDevNum < 1 || stInitDev.stOutputDevSetParameters.iOutputDevNum > 2)
		return MSA_ERR_INPUT_PARAMETERS_ERR;
	
	if( gMsaCtrl.isInit == 1 )
	{
		return MSA_ERR_ALREADY_INIT;
	}	

	SAMPLE_COMM_SYS_Exit();

	/******************************************
	step  1: init variable 
	******************************************/
	memset(&stVbConf,0,sizeof(VB_CONF_S));     
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,\
		PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.u32MaxPoolCnt = 128;
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt  = 10;

	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_ENCODING_MODE_PAL,\
		PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
	stVbConf.astCommPool[1].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt =  10;

	/******************************************
	step 2: mpp system init. 
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		DPRINTK("system init failed with %d!\n", s32Ret);
		goto err_1;
	}

    /******************************************
     step 3: start vdec 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(VIDEO_ENCODING_MODE_PAL, PIC_HD1080, &stSize);
	if (HI_SUCCESS !=s32Ret)
	{
		DPRINTK("get picture size failed!\n");
		goto err_0;
	}
    enType = PT_H264;
    memset(&stVbConf,0,sizeof(VB_CONF_S));        
    SAMPLE_COMM_VDEC_ModCommPoolConf(&stVbConf, enType, &stSize);	
	s32Ret = SAMPLE_COMM_VDEC_InitModCommVb(&stVbConf);
	if(s32Ret != HI_SUCCESS)
	{	    	
	    SAMPLE_PRT("init mod common vb fail for %#x!\n", s32Ret);
	    goto err_1;
	}

	/******************************************
	step 3: start DHD0 	 
	******************************************/    
	/**************start Dev DHD0****************************/
	VoDev = SAMPLE_VO_DEV_DHD0;    
	stVoPubAttr.enIntfSync = stInitDev.stOutputDevSetParameters.enOutputScreenMode[0];	
	stVoPubAttr.enIntfType = stInitDev.stOutputDevSetParameters.iDevIntfType[0];	
	stVoPubAttr.u32BgColor = 0x00000000;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);    
	if (HI_SUCCESS != s32Ret)
	{
		DPRINTK("SAMPLE_COMM_VO_StartDev failed!\n");
		goto err;
	}

	/**************start Layer VHD0  for base show****************************/
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	stLayerAttr.bClusterMode = HI_FALSE;
	stLayerAttr.bDoubleFrame = HI_FALSE;
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;    
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stDispRect.u32Width, &stLayerAttr.stDispRect.u32Height, &stLayerAttr.u32DispFrmRt);
	if (s32Ret != HI_SUCCESS)
	{
		DPRINTK("failed with %#x!\n", s32Ret);
		goto  err_1;
	}

	if( stInitDev.stOutputDevSetParameters.iDevCombineMode[0] == MSA_SOFTWARE_COMBINE_MODE)
	{
		s32Ret = HI_MPI_VO_SetVideoLayerPartitionMode(VoLayer,VO_PART_MODE_SINGLE);    
		if (s32Ret != HI_SUCCESS)
		{
			DPRINTK("HI_MPI_VO_SetVideoLayerPartitionMode failed with %#x!\n", s32Ret);
			goto  err_1;
		}
	}	
	stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
	stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;
	stOutputDevLayerAttr[0] = stLayerAttr;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);    
	if (HI_SUCCESS != s32Ret)
	{
		DPRINTK("SAMPLE_COMM_VO_StartLayer failed!\n");
		goto err_1;
	} 

	{
		VO_VGA_PARAM_S VgaParam;		
		HI_U32 u32Contrast = 0x0;
		HI_U32 u32Luma = 0x0;
		HI_U32 u32Satuature = 0x0;
		HI_U32 u32Gain = 0x0;

		s32Ret =HI_MPI_VO_GetVgaParam(VoDev,&VgaParam);
		if (HI_SUCCESS != s32Ret)
		{
		   DPRINTK("HI_MPI_VO_GetVgaParam err %x!\n",s32Ret);			  
		}    			
		
		VgaParam.stCSC.u32Contrast = 20;
		VgaParam.stCSC.u32Luma = 50;
		VgaParam.stCSC.u32Satuature = 50;			

		s32Ret =HI_MPI_VO_SetVgaParam(VoDev,&VgaParam);
		if (HI_SUCCESS != s32Ret)
		{
		   DPRINTK("HI_MPI_VO_GetVgaParam err %x!\n",s32Ret);
		}
	}

	if( stInitDev.stOutputDevSetParameters.iOutputDevNum == 2 )
	{
		/******************************************
		step 3: start DHD1 	 
		******************************************/    
		/**************start Dev DHD0****************************/
		VoDev = SAMPLE_VO_DEV_DHD1;    
		stVoPubAttr.enIntfSync = stInitDev.stOutputDevSetParameters.enOutputScreenMode[1];	
		stVoPubAttr.enIntfType = stInitDev.stOutputDevSetParameters.iDevIntfType[1];		
		stVoPubAttr.u32BgColor = 0x00000000;
		s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);    
		if (HI_SUCCESS != s32Ret)
		{
			DPRINTK("SAMPLE_COMM_VO_StartDev failed!\n");
			goto err;
		}

		/**************start Layer VHD0  for base show****************************/
		VoLayer = SAMPLE_VO_LAYER_VHD1;
		stLayerAttr.bClusterMode = HI_FALSE;
		stLayerAttr.bDoubleFrame = HI_FALSE;
		stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;    
		s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
			&stLayerAttr.stDispRect.u32Width, &stLayerAttr.stDispRect.u32Height, &stLayerAttr.u32DispFrmRt);
		if (s32Ret != HI_SUCCESS)
		{
			DPRINTK("failed with %#x!\n", s32Ret);
			goto  err_1;
		}

		/**************set layer partiMode****************************/    
		if( stInitDev.stOutputDevSetParameters.iDevCombineMode[1] == MSA_SOFTWARE_COMBINE_MODE)
		{
			s32Ret = HI_MPI_VO_SetVideoLayerPartitionMode(VoLayer,VO_PART_MODE_SINGLE);    
			if (s32Ret != HI_SUCCESS)
			{
				DPRINTK("HI_MPI_VO_SetVideoLayerPartitionMode failed with %#x!\n", s32Ret);
				goto  err_1;
			}
		}
		stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
		stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

		stOutputDevLayerAttr[1] = stLayerAttr;
		s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);    
		if (HI_SUCCESS != s32Ret)
		{
			DPRINTK("SAMPLE_COMM_VO_StartLayer failed!\n");
			goto err_1;
		}
	}

	/************start Layer VPIP  for Full screen***********/
	VoLayer = SAMPLE_VO_LAYER_VPIP;
	stLayerAttr.bClusterMode = HI_TRUE;
	stLayerAttr.bDoubleFrame = HI_FALSE;
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;    

	/**************set layer partiMode****************************/    
	/*	s32Ret = HI_MPI_VO_SetVideoLayerPartitionMode(VoLayer,VO_PART_MODE_SINGLE);    
	if (s32Ret != HI_SUCCESS)
	{
	SAMPLE_PRT("HI_MPI_VO_SetVideoLayerPartitionMode failed with %#x!\n", s32Ret);
	goto  END_HDZOOMIN_7;
	} 
	*/

    //false
	iPipBindDev = stInitDev.stOutputDevSetParameters.iPipLayerBindDevId ;
	if( iPipBindDev == 1 )
	{
	
		VoDev = SAMPLE_VO_DEV_DHD0;
		VoLayer = SAMPLE_VO_LAYER_VPIP;	
		s32Ret = HI_MPI_VO_UnBindVideoLayer(VoLayer ,VoDev);
		if (s32Ret != HI_SUCCESS)
		{
			DPRINTK("Pip video layer unbind to dev 0 failed with errno %#x!\n",VoDev, s32Ret);
			goto err_1;
		}

		VoDev = SAMPLE_VO_DEV_DHD1;
		
		s32Ret = HI_MPI_VO_BindVideoLayer(VoLayer ,VoDev);
		if (s32Ret != HI_SUCCESS)
		{
			DPRINTK("Pip video layer bind to dev %d failed with errno %#x!\n",VoDev, s32Ret);
			goto err_1;
		}
	}	

	s32Ret = SAMPLE_COMM_VO_GetWH(stInitDev.stOutputDevSetParameters.enOutputScreenMode[iPipBindDev], \
		&stLayerAttr.stDispRect.u32Width, &stLayerAttr.stDispRect.u32Height, &u32FrameRate);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		goto  err_1;
	}	
	stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
	stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;
	s32Ret = HI_MPI_VO_SetVideoLayerPriority(VoLayer, SAMPLE_VO_LAYER_PRIORITY_BASE);	
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
		goto err_1;
	}  

	stPipLayerAttr = stLayerAttr;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);    
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
		goto err_1;
	}  

	if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
	{
	    SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
	    goto err_1;
	}

	/******************************************
     step 4: start DSD0 
    ******************************************/    
    /**************start Dev DSD0****************************/
    //VoDev = SAMPLE_VO_DEV_DSD0;    
    stVoPubAttrSD0.enIntfSync = VO_OUTPUT_PAL;
    stVoPubAttrSD0.enIntfType = VO_INTF_CVBS;
    stVoPubAttrSD0.u32BgColor = 0x00000000; //0x000000ff;
    s32Ret = SAMPLE_COMM_VO_StartDev(SAMPLE_VO_DEV_DSD0, &stVoPubAttrSD0);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartDev failed!\n");
        goto err;
    }
    
    /**************start Layer VSD0 ****************************/
    //VoLayer = SAMPLE_VO_LAYER_VSD0;
    stLayerAttrSD0.bClusterMode = HI_FALSE;
    stLayerAttrSD0.bDoubleFrame = HI_FALSE;
    stLayerAttrSD0.enPixFormat = SAMPLE_PIXEL_FORMAT;    
    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttrSD0.enIntfSync, \
        &stLayerAttrSD0.stDispRect.u32Width, &stLayerAttrSD0.stDispRect.u32Height, &stLayerAttrSD0.u32DispFrmRt);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  err;
    }
    stLayerAttrSD0.stImageSize.u32Width = stLayerAttrSD0.stDispRect.u32Width;
    stLayerAttrSD0.stImageSize.u32Height = stLayerAttrSD0.stDispRect.u32Height;

    s32Ret = SAMPLE_COMM_VO_StartLayer(SAMPLE_VO_LAYER_VSD0, &stLayerAttrSD0);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
        goto err;
    }
	/*------------------------------------------------------------------------------------------------------*/
	
	HI_TDE2_Open();

	s32Ret = gui_open_fb(MSA_HD0,stLayerAttr.stDispRect.u32Width,stLayerAttr.stDispRect.u32Height);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("gui_open_fb %d failed!\n",MSA_HD0);
		goto err_1;
	}
	else
		MSA_ClearOsd(0,0,stLayerAttr.stDispRect.u32Width,stLayerAttr.stDispRect.u32Height,MSA_HD0);
	
	if( stInitDev.stOutputDevSetParameters.iOutputDevNum == 2 )
	{
		s32Ret = gui_open_fb(MSA_HD1,stLayerAttr.stDispRect.u32Width,stLayerAttr.stDispRect.u32Height);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("gui_open_fb %d failed!\n",MSA_HD1);
			goto err_1;
		}
		else
			MSA_ClearOsd(0,0,stLayerAttr.stDispRect.u32Width,stLayerAttr.stDispRect.u32Height,MSA_HD1);
	}

    s32Ret = gui_open_fb(MSA_SD0, 720, 576);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("gui_open_fb %d failed!\n",MSA_SD0);
		goto err_1;
	}
	else
		MSA_ClearOsd(0,0,720,576,MSA_SD0);
//////////////////////////////////////////////////////////////////////
	gMsaCtrl.stInitDev = stInitDev;
	gMsaCtrl.isInit = 1;
	pthread_mutex_init(&gMsaCtrl.hOpMutex,NULL);	

	DPRINTK("Init media sys success!\n");
	return MSA_SUCCESS;
	
err:

err_1:
	SAMPLE_COMM_SYS_Exit();
err_0:		
	return MSA_FAILED;
}


int MSA_DestroySystem()
{
	VO_LAYER VoLayer;
	HI_S32 s32Ret = HI_SUCCESS;
	VO_DEV VoDev;    
	int i = 0;

	if( gMsaCtrl.isInit == 1 )
	{
		HI_TDE2_Close();
		gui_close_fb(MSA_HD0);
		gui_close_fb(MSA_HD1);
		gui_close_fb(MSA_SD0);

		for( i = 0; i < MSA_MAX_CHANNEL_NUM; i++)
		{
			if(  gMsaCtrl.pstChanInfo[i] != NULL )	
			{
				MSA_DestroyChanWindow(gMsaCtrl.pstChanInfo[i]);						
			}
		}

		VoLayer = SAMPLE_VO_LAYER_VPIP;
		s32Ret = SAMPLE_COMM_VO_StopLayer(VoLayer);    
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_StopLayer failed!\n");		    
		}

		VoLayer = SAMPLE_VO_LAYER_VHD1;
		s32Ret = SAMPLE_COMM_VO_StopLayer(VoLayer);    
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_StopLayer failed!\n");		    
		}
		VoDev = SAMPLE_VO_DEV_DHD1;    
		s32Ret = SAMPLE_COMM_VO_StopDev(VoDev);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_StopLayer failed!\n");		    
		}  

		VoLayer = SAMPLE_VO_LAYER_VHD0;
		s32Ret = SAMPLE_COMM_VO_StopLayer(VoLayer);    
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_StopLayer failed!\n");		    
		}
		VoDev = SAMPLE_VO_DEV_DHD0;    
		s32Ret = SAMPLE_COMM_VO_StopDev(VoDev);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_StopLayer failed!\n");		    
		}  

		VoLayer = SAMPLE_VO_LAYER_VSD0;
		s32Ret = SAMPLE_COMM_VO_StopLayer(VoLayer);    
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_StopLayer failed!\n");		    
		}
		VoDev = SAMPLE_VO_DEV_DSD0;    
		s32Ret = SAMPLE_COMM_VO_StopDev(VoDev);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_StopLayer failed!\n");		    
		} 
		
		SAMPLE_COMM_SYS_Exit();
		pthread_mutex_destroy(&gMsaCtrl.hOpMutex);
		gMsaCtrl.isInit = 0;
	}

	return MSA_SUCCESS;
}

int MSA_GetCombineMode(int dev)
{
	return gMsaCtrl.stInitDev.stOutputDevSetParameters.iDevCombineMode[dev];
}

int MSA_GetChanLayer(MSA_CHANNEL_INFO_ST * pChan)
{
	VO_LAYER VoLayer;
	
	if( pChan->stChan.enShowLayer == MSA_BASE_LAYER )
	{				
		if( pChan->stChan.enOutputDev == gMsaCtrl.stInitDev.stOutputDevSetParameters.iPipLayerBindDevId)
			VoLayer = SAMPLE_VO_LAYER_VPIP;
		else
			VoLayer = pChan->stChan.enOutputDev;
	}
	else
		VoLayer = pChan->stChan.enOutputDev;

	return VoLayer;
}

// windows from 0 -> 3
int  MSA_CreateChanWindow(F_IN MSA_CHANNEL_ST stChan,F_OUT MSA_HANDLE * handle)
{
	MSA_CHANNEL_INFO_ST * pChan = NULL;
	int iAvailableId = -1;
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	VO_LAYER VoLayer;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

    //do nothing. ignore
	iRet  = MSA_CheckChanParametersIsRight(&gMsaCtrl,stChan);
	if( iRet < 0 )
	{
		iRetNo = iRet;
		goto err;
	}

    //id from 0 to 15
	iAvailableId = MSA_GetAvailableChanId(&gMsaCtrl);
	if( iAvailableId < 0 )
	{
		DPRINTK("Can't get available id\n");
		iRetNo = iAvailableId;
		goto err;
	}	

	pChan = (MSA_CHANNEL_INFO_ST *)malloc(sizeof(MSA_CHANNEL_INFO_ST));
	if( pChan == NULL )
	{		
		iRetNo = MSA_ERR_MEM_ALLOC;
		goto err;
	}

	pChan->iVpssChan  = -1;
	pChan->iVdecChan = -1;
	pChan->iVencChan = -1;
	pChan->iOsdLayerChan = -1;
	pChan->iVoChan = -1;

    // true
	if( MSA_GetCombineMode(stChan.enOutputDev) ==  MSA_HARDWARE_COMBINE_MODE )
	{
		//Create vpss
		iRet = StartVpss(iAvailableId);
		if( iRet < 0 )
		{
			DPRINTK("Start vpss %d err\n",iAvailableId);
			iRetNo = MSA_FAILED;
			goto err;
		}
		//VPSS GROUP id
		pChan->iVpssChan = iAvailableId;

		if( stChan.enShowLayer == MSA_BASE_LAYER )
		{
			if( stChan.enOutputDev == gMsaCtrl.stInitDev.stOutputDevSetParameters.iPipLayerBindDevId)
				VoLayer = SAMPLE_VO_LAYER_VPIP;
			else
				VoLayer = stChan.enOutputDev;
		}
		else // run here  VoLayer = MSA_HD0
			VoLayer = stChan.enOutputDev;
	}
	else
	{
		VoLayer = stChan.enOutputDev;
	}	

    //Create vo
	iRet = StartVoChn(VoLayer,iAvailableId,stChan.stWindow.iPosStartX,stChan.stWindow.iPosStartY,stChan.stWindow.iWidth,stChan.stWindow.iHeight);
	if( iRet < 0 )
	{
		DPRINTK("StartVoChn%d err\n",iAvailableId);
		iRetNo = MSA_FAILED;
		goto err1;
	}
	pChan->iVoChan = iAvailableId;
	pChan->iVoShow = MSA_WINDOW_SHOW;

    //true
	if( MSA_GetCombineMode(stChan.enOutputDev) ==  MSA_HARDWARE_COMBINE_MODE   )
	{	
        iRet = SAMPLE_COMM_VO_BindVpss(VoLayer, pChan->iVoChan, pChan->iVpssChan , 0);
		if (HI_SUCCESS != iRet)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss failed!\n");
			iRetNo = MSA_FAILED;
			goto err1;
		}
	}	
	pChan->stChan = stChan;
	pChan->iId = iAvailableId;

    /* for DSD0 */
    if (iAvailableId == 0)
    {
        iRet = StartVoChn(SAMPLE_VO_LAYER_VSD0, iAvailableId,0,0,720,576);
        if( iRet < 0 )
        {
            DPRINTK("StartVoChn%d err\n",iAvailableId);
            iRetNo = MSA_FAILED;
            goto err3;
        }

        iRet = SAMPLE_COMM_VO_BindVpss(SAMPLE_VO_LAYER_VSD0, iAvailableId, iAvailableId, 2);
        if (HI_SUCCESS != iRet)
        {
            SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss failed!\n");
            iRetNo = MSA_FAILED;
            goto err3;
        }
    }

	if( pChan->iVdecChan < 0 )
	{
		iRet = MSA_CreateDec(MSA_VIDEO_H264,pChan->iId);
		if( iRet < 0 )
		{
			DPRINTK("MSA_CreateDec iId=%d err\n",pChan->iId);
			iRetNo = iRet;
			goto err;
		}
		
		if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_HARDWARE_COMBINE_MODE )
		{
			iRet = SAMPLE_COMM_VDEC_BindVpss(pChan->iId,pChan->iVpssChan);
			if (HI_SUCCESS != iRet)
			{
				SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
				iRetNo = MSA_FAILED;
				goto err;
			}
		}
		else if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_SOFTWARE_COMBINE_MODE  )
		{
			VoLayer = pChan->stChan.enOutputDev;
			iRet = SAMPLE_COMM_VDEC_BindVo(pChan->iId,VoLayer,pChan->iVoChan);
			if (HI_SUCCESS != iRet)
			{
				SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
				iRetNo = MSA_FAILED;
				goto err;
			}
		}

		pChan->enVdecMedia = MSA_VIDEO_H264;
		pChan->iVdecChan = pChan->iId;

		DPRINTK("create dec [%d] %d\n",pChan->iVdecChan,pChan->enVdecMedia);
	}

	iRet = MSA_InsertChanInfo(&gMsaCtrl,iAvailableId,pChan);
	if( iRet < 0 )
	{
		DPRINTK("MSA_InsertChanInfo %d err\n",iAvailableId);
		iRetNo = iRet;
		goto err1;
	}
	*handle = pChan;

	MSA_OpUnLock();

	return iRetNo;

err3:
    StopVoChn(SAMPLE_VO_LAYER_VSD0, 0);
err2:
	StopVoChn(VoLayer,iAvailableId);
err1:
	StopVpss(iAvailableId);

err:
	if( pChan )
	{
		free(pChan);
		pChan = NULL;
	}

	MSA_OpUnLock();

	return iRetNo;
}


int MSA_DestroyChanWindow(F_IN MSA_HANDLE  hHandle)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	VO_LAYER VoLayer;
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();	

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}

	iRet = MSA_CheckEnableOp(hHandle);
	if( iRet < 0 )
	{	
		iRetNo = iRet;
		goto err;
	}

	iRet = MSA_DelChanInfo(&gMsaCtrl,pChan->iId,pChan);
	if( iRet < 0 )
	{
		DPRINTK("MSA_DelChanInfo %d err\n",pChan->iId);
		iRetNo = MSA_FAILED;
		goto err;
	}
	
	if( pChan->iVdecChan != -1 )
	{		
		if(  MSA_GetCombineMode(pChan->stChan.enOutputDev) == MSA_HARDWARE_COMBINE_MODE)
		{
			iRet = SAMPLE_COMM_VDEC_UnBindVpss(pChan->iVdecChan,pChan->iVpssChan);
			if (HI_SUCCESS != iRet)
			 {
				 SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
				 iRetNo = MSA_FAILED;
				 goto err;
			 }			
		}else
		{
			VoLayer = pChan->stChan.enOutputDev;
			iRet = SAMPLE_COMM_VDEC_UnBindVo(pChan->iVdecChan,VoLayer,pChan->iVoChan);
			if (HI_SUCCESS != iRet)
			{
				SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
				iRetNo = MSA_FAILED;
				goto err;
			}
		}		

		iRet =StopVdecChn(pChan->iVdecChan);
		if( iRet < 0 )
		{
			DPRINTK("StopVdecChn %d err\n",pChan->iVdecChan);
			iRetNo = iRet;
			goto err;
		}
		
		pChan->iVdecChan = -1;
	}

	if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_HARDWARE_COMBINE_MODE)
	{
		VoLayer = MSA_GetChanLayer(pChan);		

		if( pChan->iVoShow == MSA_WINDOW_SHOW)
		{
			SAMPLE_COMM_VO_UnBindVpss(VoLayer,pChan->iVoChan,pChan->iVpssChan,1);
			StopVoChn(VoLayer,pChan->iVoChan);	
		}

		/* DSD0 unbind vpss */
        if (pChan->iVpssChan == 0)
        {
            iRet = SAMPLE_COMM_VO_UnBindVpss(SAMPLE_VO_LAYER_VSD0, 0, 0, 2);
    		if (HI_SUCCESS != iRet)
    		{
    			SAMPLE_PRT("SAMPLE_COMM_VO_UnBindVpss failed!\n");
    			iRetNo = MSA_FAILED;
    			goto err;
    		}
            StopVoChn(SAMPLE_VO_LAYER_VSD0, 0);
            
            SAMPLE_COMM_VO_UnBindVpss(VoLayer, 0, pChan->iVpssChan, 2);
			StopVoChn(VoLayer, 0);
        }
        
		StopVpss(pChan->iVpssChan);
		pChan->iVpssChan = -1;
	}else
	{
		StopVoChn(VoLayer,pChan->iVoChan);	
	}

	free(pChan);

	MSA_OpUnLock();

	return iRetNo;
err:		

	MSA_OpUnLock();
	return iRetNo;
}




int MSA_ShowChanWindow(F_IN MSA_HANDLE  hHandle,F_IN MSA_WINDOW_SHOW_E enShow)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	VO_LAYER VoLayer;
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();	

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}


	if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_HARDWARE_COMBINE_MODE)
	{
		VoLayer = MSA_GetChanLayer(pChan);	

		if( enShow == MSA_WINDOW_HIDE && pChan->iVoShow == MSA_WINDOW_SHOW)
		{
			SAMPLE_COMM_VO_UnBindVpss(VoLayer,pChan->iVoChan,pChan->iVpssChan,1);		
			StopVoChn(VoLayer,pChan->iVoChan);	
			pChan->iVoShow = MSA_WINDOW_HIDE;
		}else if( enShow == MSA_WINDOW_SHOW && pChan->iVoShow == MSA_WINDOW_HIDE)
		{			

			iRet = StartVoChn(VoLayer,pChan->iVoChan,pChan->stChan.stWindow.iPosStartX,
				pChan->stChan.stWindow.iPosStartY,pChan->stChan.stWindow.iWidth,pChan->stChan.stWindow.iHeight);
			if( iRet < 0 )
			{
				DPRINTK("StartVoChn%d err\n",pChan->iVoChan);
				iRetNo = MSA_FAILED;
				goto err;
			}			

			pChan->iVoShow = MSA_WINDOW_SHOW;

			iRet = SAMPLE_COMM_VO_BindVpss(VoLayer,pChan->iVoChan,pChan->iVpssChan,0);	
			if (HI_SUCCESS != iRet)
			 {
				 SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss failed! %x\n",iRet);	  
				 iRetNo = MSA_FAILED;
				 goto err;
			 }
		}
	}else
	{
		SAMPLE_PRT("MSA_SOFTWARE_COMBINE_MODE not support this func failed!\n");	  
		iRetNo = MSA_ERR_SOFTWARE_COMBINE_MODE_NOT_SUPPORT;
		goto err;	
	}
	

	MSA_OpUnLock();

	return iRetNo;
err:		

	MSA_OpUnLock();
	return iRetNo;
}



int MSA_CreateDec(MSA_INPUT_MEDIA Media,int iChn)
{
	VDEC_CHN_ATTR_S stVdecChnAttr;
	PAYLOAD_TYPE_E enType;

	if( Media == MSA_VIDEO_H264 )
		enType = PT_H264;
	else if( Media == MSA_PIC_JPEG )
		enType = PT_JPEG;
	else
	{
		return MSA_ERR_UNKNOW_INPUT_MEDIA;
	}

	SetVdecChnAttr(&stVdecChnAttr, enType);
	return StartVdecChn(iChn,&stVdecChnAttr);
}


int MSA_SendDecData(F_IN MSA_HANDLE hHandle,F_IN MSA_DATA_HEAD_ST stDataHead,F_IN void * pData)
{
	int iAvailableId = -1;
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	VDEC_STREAM_S stStream;
	VO_LAYER VoLayer;	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( hHandle == NULL || pData == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}

	MSA_OpLock();	

retry:

	if( pChan->iVdecChan < 0 )
	{
		iRet = MSA_CreateDec(stDataHead.enDataType,pChan->iId);
		if( iRet < 0 )
		{
			DPRINTK("MSA_CreateDec iId=%d err\n",pChan->iId);
			iRetNo = iRet;
			goto err;
		}

		
		if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_HARDWARE_COMBINE_MODE )
		{
			iRet = SAMPLE_COMM_VDEC_BindVpss(pChan->iId,pChan->iVpssChan);
			if (HI_SUCCESS != iRet)
			{
				SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
				iRetNo = MSA_FAILED;
				goto err;
			}
		}else if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_SOFTWARE_COMBINE_MODE  )
		{
			VoLayer = pChan->stChan.enOutputDev;
			iRet = SAMPLE_COMM_VDEC_BindVo(pChan->iId,VoLayer,pChan->iVoChan);
			if (HI_SUCCESS != iRet)
			{
				SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
				iRetNo = MSA_FAILED;
				goto err;
			}
		}

		pChan->enVdecMedia = stDataHead.enDataType;
		pChan->iVdecChan = pChan->iId;
		DPRINTK("create dec [%d] %d\n",pChan->iVdecChan,pChan->enVdecMedia);
	}

	if( pChan->enVdecMedia != stDataHead.enDataType)
	{	
		
		if( pChan->iVdecChan != -1 )
		{
			if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_HARDWARE_COMBINE_MODE )
			{						
				iRet = SAMPLE_COMM_VDEC_UnBindVpss(pChan->iVdecChan,pChan->iVpssChan);
				if (HI_SUCCESS != iRet)
				{
					SAMPLE_PRT("SAMPLE_COMM_VDEC_UnBindVpss failed!\n");	  
					iRetNo = MSA_FAILED;
					goto err;
				}	
			}			
			else
			{
				VoLayer = pChan->stChan.enOutputDev;
				iRet = SAMPLE_COMM_VDEC_UnBindVo(pChan->iVdecChan,VoLayer,pChan->iVoChan);
				if (HI_SUCCESS != iRet)
				{
					SAMPLE_PRT("SAMPLE_COMM_VDEC_UnBindVo failed!\n");	  
					iRetNo = MSA_FAILED;
					goto err;
				}		
			}		
	
			iRet =StopVdecChn(pChan->iVdecChan);
			if( iRet < 0 )
			{
				DPRINTK("StopVdecChn %d err\n",pChan->iVdecChan);
				iRetNo = iRet;
				goto err;
			}

			pChan->iVdecChan = -1;
		}
		goto retry;
	}

	stStream.u64PTS  = stDataHead.TimeStamp.u64PTS;	
	stStream.pu8Addr = pData;
	stStream.u32Len  = stDataHead.iDataLen; 
	stStream.bEndOfFrame  = HI_TRUE;
	stStream.bEndOfStream = HI_FALSE;       

	iRet=HI_MPI_VDEC_SendStream(pChan->iVdecChan, &stStream, 0);
	if (HI_SUCCESS != iRet)
	{
		DPRINTK("HI_MPI_VDEC_SendStream %d err %x\n",pChan->iVdecChan,iRet);
		usleep(100);		
		iRetNo = MSA_ERR_DEC_ERR;
		goto err;
	}	
	
	MSA_OpUnLock(); 
	return iRetNo;
err:	
	MSA_OpUnLock();	
	return iRetNo;
}


int MAS_ChanWindowSet(F_IN MSA_HANDLE hHandle, F_IN MSA_WINDOW_ST stWindow)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	VO_LAYER VoLayer;
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}		

/*
	iRet = MSA_CheckEnableOp(hHandle);
	if( iRet < 0 )
	{	
		iRetNo = iRet;
		goto err;
	}
	*/

	if( pChan->iVoShow ==  MSA_WINDOW_HIDE )
	{
		iRetNo = MSA_ERR_VO_IS_HIDE;
		goto err;
	}


	if(  MSA_GetCombineMode(pChan->stChan.enOutputDev) == MSA_HARDWARE_COMBINE_MODE)
	{
		if( pChan->stChan.enShowLayer == MSA_BASE_LAYER )
		{			
			if( pChan->stChan.enOutputDev == gMsaCtrl.stInitDev.stOutputDevSetParameters.iPipLayerBindDevId)
				VoLayer = SAMPLE_VO_LAYER_VPIP;
			else
				VoLayer = pChan->stChan.enOutputDev;
		}
		else
			VoLayer = pChan->stChan.enOutputDev;

		iRet = SAMPLE_COMM_VO_UnBindVpss(VoLayer,pChan->iVoChan,pChan->iVpssChan,0);
		if (HI_SUCCESS != iRet)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_UnBindVpss failed!%x\n",iRet);	  
			iRetNo = MSA_FAILED;
			goto err;
		}
	}else
	{
	
		VoLayer = pChan->stChan.enOutputDev;

		if( pChan->iVdecChan != -1 )
		{
			iRet = SAMPLE_COMM_VDEC_UnBindVo(pChan->iVdecChan,VoLayer,pChan->iVoChan);
			if (HI_SUCCESS != iRet)
			{
				SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!%x\n",iRet);	  
				iRetNo = MSA_FAILED;
				goto err;
			}	
		}
	}

	StopVoChn(VoLayer,pChan->iVoChan);

	iRet = StartVoChn(VoLayer,pChan->iVoChan,stWindow.iPosStartX,stWindow.iPosStartY,stWindow.iWidth,stWindow.iHeight);
	if( iRet < 0 )
	{
		DPRINTK("StartVoChn%d err\n",pChan->iVoChan);
		iRetNo = MSA_FAILED;
		goto err;
	}


	if(  MSA_GetCombineMode(pChan->stChan.enOutputDev) == MSA_HARDWARE_COMBINE_MODE)
	{
		if( pChan->stChan.enShowLayer == MSA_BASE_LAYER )
		{		
			if( pChan->stChan.enOutputDev == gMsaCtrl.stInitDev.stOutputDevSetParameters.iPipLayerBindDevId)
				VoLayer = SAMPLE_VO_LAYER_VPIP;
			else
				VoLayer = pChan->stChan.enOutputDev;
		}
		else
			VoLayer = pChan->stChan.enOutputDev;

		iRet = SAMPLE_COMM_VO_BindVpss(VoLayer,pChan->iVoChan,pChan->iVpssChan,0);
		if (HI_SUCCESS != iRet)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss failed!%x\n",iRet);	  
			iRetNo = MSA_FAILED;
			goto err;
		}
	}else
	{
		VoLayer = pChan->stChan.enOutputDev;

		if( pChan->iVdecChan != -1 )
		{
			iRet = SAMPLE_COMM_VDEC_BindVo(pChan->iVdecChan,VoLayer,pChan->iVoChan);
			if (HI_SUCCESS != iRet)
			{
				SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!%x\n",iRet);	  
				iRetNo = MSA_FAILED;
				goto err;
			}	
		}
	}

	pChan->stChan.stWindow = stWindow;


	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}



int MAS_ChanWindowOpenZoomIn(F_IN MSA_HANDLE hHandle, F_IN MSA_WINDOW_ZOOM_IN_ST stZoomInWindow)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	VO_LAYER VoLayer;
	VPSS_CROP_INFO_S stVpssClip; 
	VO_ZOOM_ATTR_S stZoomAttr;    
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;


	MSA_OpLock();

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}	

	if(  MSA_GetCombineMode(pChan->stChan.enOutputDev) == MSA_HARDWARE_COMBINE_MODE)
	{
		stVpssClip.bEnable = HI_TRUE;
		stVpssClip.enCropCoordinate = stZoomInWindow.enCropCoordinate;
		stVpssClip.stCropRect.s32X = ALIGN_BACK(stZoomInWindow.iPicStartX,4);
		stVpssClip.stCropRect.s32Y = ALIGN_BACK(stZoomInWindow.iPicStartY,4);
		stVpssClip.stCropRect.u32Width = ALIGN_BACK(stZoomInWindow.iWidth,8);
		stVpssClip.stCropRect.u32Height = ALIGN_BACK(stZoomInWindow.iHeight,8); 

		iRet = HI_MPI_VPSS_SetGrpCrop(pChan->iVpssChan, &stVpssClip);    
		if (iRet != HI_SUCCESS)
		{
			SAMPLE_PRT("failed with %#x!\n", iRet);
			goto  err;
		} 

		DPRINTK("enCropCoordinate:%d x:%d y:%d widht:%d height:%d\n", stZoomInWindow.enCropCoordinate,
			stVpssClip.stCropRect.s32X,stVpssClip.stCropRect.s32Y,stVpssClip.stCropRect.u32Width,stVpssClip.stCropRect.u32Height);
	}else
	{
		VoLayer = pChan->stChan.enOutputDev;

		if( stZoomInWindow.enCropCoordinate == MSA_VPSS_CROP_RATIO_COOR)
		{
			stZoomAttr.enZoomType = VOU_ZOOM_IN_RATIO;
			stZoomAttr.stZoomRatio.u32XRatio =  ALIGN_BACK(stZoomInWindow.iPicStartX,4);
			stZoomAttr.stZoomRatio.u32YRatio = ALIGN_BACK(stZoomInWindow.iPicStartY,4);
			stZoomAttr.stZoomRatio.u32WRatio = ALIGN_BACK(stZoomInWindow.iWidth,8);
			stZoomAttr.stZoomRatio.u32HRatio = ALIGN_BACK(stZoomInWindow.iHeight,8); 
		}else
		{
			stZoomAttr.enZoomType = VOU_ZOOM_IN_RECT;
			stZoomAttr.stZoomRect.s32X =  ALIGN_BACK(stZoomInWindow.iPicStartX,4);
			stZoomAttr.stZoomRect.s32Y = ALIGN_BACK(stZoomInWindow.iPicStartY,4);
			stZoomAttr.stZoomRect.u32Width = ALIGN_BACK(stZoomInWindow.iWidth,8);
			stZoomAttr.stZoomRect.u32Height = ALIGN_BACK(stZoomInWindow.iHeight,8); 
		}

		iRet = HI_MPI_VO_SetZoomInWindow(VoLayer, pChan->iVoChan, &stZoomAttr);
		if (HI_SUCCESS != iRet)
		{
			SAMPLE_PRT("HI_MPI_VO_SetZoomInWindow failed!\n");
			goto err;
		} 

		DPRINTK("enCropCoordinate:%d x:%d y:%d widht:%d height:%d\n", stZoomInWindow.enCropCoordinate,
			stVpssClip.stCropRect.s32X,stVpssClip.stCropRect.s32Y,stVpssClip.stCropRect.u32Width,stVpssClip.stCropRect.u32Height);
	}

	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}

int MAS_ChanWindowCloseZoomIn(F_IN MSA_HANDLE hHandle)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	VO_LAYER VoLayer;
	VPSS_CROP_INFO_S stVpssClip;  
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}	

	if( pChan->stChan.enOutputDev == MSA_HD0 )
	{
		iRet = HI_MPI_VPSS_GetGrpCrop(pChan->iVpssChan, &stVpssClip);
		if (HI_SUCCESS != iRet)
		{
			DPRINTK("HI_MPI_VPSS_GetGrpCrop failed with %#x!\n", iRet);
			return -1;
		}


		if( stVpssClip.bEnable == HI_TRUE )
		{		 
			stVpssClip.bEnable = HI_FALSE;			

			iRet = HI_MPI_VPSS_SetGrpCrop(pChan->iVpssChan, &stVpssClip);
			if (HI_SUCCESS != iRet)
		 {
			 DPRINTK("HI_MPI_VPSS_SetGrpCrop failed with %#x!\n", iRet);
			 return -1;
		 }
		}
	}else
	{
		VO_ZOOM_ATTR_S stZoomAttr; 		 
		stZoomAttr.enZoomType = VOU_ZOOM_IN_RECT;
		stZoomAttr.stZoomRect.s32X =  0;
		stZoomAttr.stZoomRect.s32Y = 0;
		stZoomAttr.stZoomRect.u32Width = 0;
		stZoomAttr.stZoomRect.u32Height = 0; 

		VoLayer = SAMPLE_VO_LAYER_VHD1;
		iRet = HI_MPI_VO_SetZoomInWindow(VoLayer, pChan->iVoChan, &stZoomAttr);
		if (HI_SUCCESS != iRet)
		{
			SAMPLE_PRT("HI_MPI_VO_SetZoomInWindow failed!\n");
			goto err;
		} 
	}

	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}


int MAS_ChanWindowSwap(F_IN MSA_HANDLE hHandle1,F_IN MSA_HANDLE hHandle2)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan;	
	VO_LAYER VoLayer1,VoLayer2;
	VO_LAYER VoLayer;
	int i = 0;
	MSA_CHANNEL_INFO_ST * pChan1 = (MSA_CHANNEL_INFO_ST *)hHandle1;
	MSA_CHANNEL_INFO_ST * pChan2 = (MSA_CHANNEL_INFO_ST *)hHandle2;
	MSA_CHANNEL_INFO_ST * pChan = NULL; 

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;
	
	MSA_OpLock();

	if( hHandle1 == NULL || hHandle2 == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}

	iRet = MSA_CheckEnableOp(hHandle1);
	if( iRet < 0 )
	{	
		iRetNo = iRet;
		goto err;
	}

	iRet = MSA_CheckEnableOp(hHandle2);
	if( iRet < 0 )
	{	
		iRetNo = iRet;
		goto err;
	}

	for( i = 0; i < 2 ; i++ )
	{
	    /* for DSD0 */
    	if (i == 0)
    	{
        	iRet = SAMPLE_COMM_VO_UnBindVpss(SAMPLE_VO_LAYER_VSD0, i, i, 2);
    		if (HI_SUCCESS != iRet)
    		{
    			SAMPLE_PRT("SAMPLE_COMM_VO_UnBindVpss failed!\n");
    			iRetNo = MSA_FAILED;
    			goto err;
    		}
            StopVoChn(SAMPLE_VO_LAYER_VSD0, i);
    	}
	
		if( i == 0 )
			pChan = pChan1;
		else
			pChan = pChan2;

		VoLayer = MSA_GetChanLayer(pChan);

		if( pChan->iVpssChan >= 0 )
		{
			iRet = SAMPLE_COMM_VO_UnBindVpss(VoLayer,pChan->iVoChan,pChan->iVpssChan,0);
			if (HI_SUCCESS != iRet)
			{
				SAMPLE_PRT("SAMPLE_COMM_VO_UnBindVpss failed! %x\n",iRet);	  
				iRetNo = MSA_FAILED;
				goto err;
			}

			if( pChan->iVdecChan >= 0 )
			{
				iRet = SAMPLE_COMM_VDEC_UnBindVpss(pChan->iVdecChan,pChan->iVpssChan);
				if (HI_SUCCESS != iRet)
				 {
					 SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
					 iRetNo = MSA_FAILED;
					 goto err;
				 }	
			}

			StopVpss(pChan->iVpssChan);
			pChan->iVpssChan = -1;
		}
		else
		{
			if( pChan->iVdecChan >= 0 )
			{
				iRet = SAMPLE_COMM_VDEC_UnBindVo(pChan->iVdecChan,VoLayer,pChan->iVoChan);
				if (HI_SUCCESS != iRet)
				{
					SAMPLE_PRT("SAMPLE_COMM_VDEC_UnBindVo failed! %x\n",iRet);	  
					iRetNo = MSA_FAILED;
					goto err;
				}	
			}
		}

		StopVoChn(VoLayer,pChan->iVoChan);

		iRet = StartVoChn(VoLayer,pChan->iVoChan,pChan->stChan.stWindow.iPosStartX,
			pChan->stChan.stWindow.iPosStartY,pChan->stChan.stWindow.iWidth,pChan->stChan.stWindow.iHeight);
		if( iRet < 0 )
		{
			DPRINTK("StartVoChn%d err\n",pChan->iVoChan);
			iRetNo = MSA_FAILED;
			goto err;
		}

        /* stop VSD0 */
        StopVoChn(SAMPLE_VO_LAYER_VSD0, 0);
        
		iRet = StartVoChn(SAMPLE_VO_LAYER_VSD0, 0, 0, 0, 720, 576);
		if( iRet < 0 )
		{
			DPRINTK("StartVoChn%d err\n", 0);
			iRetNo = MSA_FAILED;
			goto err;
		}
	}
		

	for( i = 0; i < 2 ; i++ )
	{
		if( i == 0 )
		{
			pChan1 = (MSA_CHANNEL_INFO_ST *)hHandle1;
			pChan2 = (MSA_CHANNEL_INFO_ST *)hHandle2;
		}else
		{
			pChan1 = (MSA_CHANNEL_INFO_ST *)hHandle2;
			pChan2 = (MSA_CHANNEL_INFO_ST *)hHandle1;
		}
		
		if(  MSA_GetCombineMode(pChan2->stChan.enOutputDev) == MSA_HARDWARE_COMBINE_MODE)
		{
			VoLayer = MSA_GetChanLayer(pChan2);
		
			if( pChan1->iVpssChan < 0 )
			{
				iRet = StartVpss(pChan1->iId);
				if( iRet < 0 )
				{
					DPRINTK("Start vpss %d err\n",pChan1->iId);
					iRetNo = MSA_FAILED;
					goto err;
				}
				pChan1->iVpssChan = pChan1->iId;

				iRet = SAMPLE_COMM_VO_BindVpss(VoLayer,pChan2->iVoChan,pChan1->iVpssChan,0);	
				if (HI_SUCCESS != iRet)
				 {
					 SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss failed! %x\n",iRet);	  
					 iRetNo = MSA_FAILED;
					 goto err;
				 }
			}

			/* DSD0 output */
			if (i == 0)
            {
                iRet = SAMPLE_COMM_VO_BindVpss(SAMPLE_VO_LAYER_VSD0, i, i, 2);	
    			if (HI_SUCCESS != iRet)
                {
                    SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss failed! %x\n",iRet);	  
                    iRetNo = MSA_FAILED;
                    goto err;
                }
            }

			if( pChan1->iVdecChan >= 0 )
			{
				iRet = SAMPLE_COMM_VDEC_BindVpss(pChan1->iVdecChan,pChan1->iVpssChan);
				if (HI_SUCCESS != iRet)
				{
					SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
					iRetNo = MSA_FAILED;
					goto err;
				}
			}			
		
		}else
		{
			VoLayer = MSA_GetChanLayer(pChan2);

			if( pChan1->iVdecChan >= 0 )
			{
				iRet = SAMPLE_COMM_VDEC_BindVo(pChan1->iVdecChan,VoLayer,pChan2->iVoChan);
				if (HI_SUCCESS != iRet)
				 {
					 SAMPLE_PRT("SAMPLE_COMM_VDEC_UnBindVo failed! %x\n",iRet);	  
					 iRetNo = MSA_FAILED;
					 goto err;
				 }
			}
		}
	
	}
			

	pChan1 = (MSA_CHANNEL_INFO_ST *)hHandle1;
	pChan2 = (MSA_CHANNEL_INFO_ST *)hHandle2;

	tmp = pChan1->iVoChan;
	pChan1->iVoChan = pChan2->iVoChan; 
	pChan2->iVoChan = tmp;	   

	tmpChan = pChan1->stChan;
	pChan1->stChan = pChan2->stChan;
	pChan2->stChan = tmpChan;


	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}

int MSA_HaveChanWindow()
{
	int i = 0;
	int iHaveChanWindow = 0;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;
	
	for( i = 0; i < MSA_MAX_CHANNEL_NUM; i++)
	{
		if(  gMsaCtrl.pstChanInfo[i] != NULL )	
		{
			iHaveChanWindow = 1;
			break;
		}
	}

	return iHaveChanWindow;
}


int MSA_SetDevCombineMode(F_IN int iOutputDevNum,MSA_COMBINE_MODE enMode)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	VO_LAYER VoLayer;
	int s32Ret;
	
	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( iOutputDevNum < 0 || iOutputDevNum >= gMsaCtrl.stInitDev.stOutputDevSetParameters.iOutputDevNum )
	{
		DPRINTK("iOutputDevNum=%d  not in avalible area [0-%d]\n",iOutputDevNum,\
			gMsaCtrl.stInitDev.stOutputDevSetParameters.iOutputDevNum-1);
		 iRetNo = MSA_ERR_INPUT_PARAMETERS_ERR;
		 goto err;
	}

	if( enMode < MSA_HARDWARE_COMBINE_MODE || enMode >= MSA_COMBINE_MODE_BUF )
	{
		
		DPRINTK("enMode=%d  not in avalible area [%d-%d]\n",enMode,\
			MSA_HARDWARE_COMBINE_MODE,MSA_COMBINE_MODE_BUF-1);
		 iRetNo = MSA_ERR_INPUT_PARAMETERS_ERR;
		 goto err;
	}


	if( MSA_HaveChanWindow() == 1 )
	{
		DPRINTK("Err: Change dev combine mode need close all chan window first.\n");
		 iRetNo = MSA_ERR_CHAN_WINDOW_NOT_ALL_CLOSE_ERR;
		 goto err;
	}


	
	if( gMsaCtrl.stInitDev.stOutputDevSetParameters.iDevCombineMode[iOutputDevNum] != enMode)
	{
		if( iOutputDevNum == 0 )
			VoLayer = SAMPLE_VO_LAYER_VHD0;
		else
			VoLayer = SAMPLE_VO_LAYER_VHD1;

		DPRINTK("VoLayer=%d enMode=%d\n",VoLayer,enMode);

		//DPRINTK("HI_MPI_VO_DisableVideoLayer=%d\n",VoLayer);
		 s32Ret = SAMPLE_COMM_VO_StopLayer(VoLayer);
	    	if (s32Ret != HI_SUCCESS)
	    	{
	       	 	SAMPLE_PRT("failed with %#x!\n", s32Ret);
	        	iRetNo = MSA_FAILED;
			goto err;
	    	}

		if( enMode == MSA_HARDWARE_COMBINE_MODE)	
			s32Ret = HI_MPI_VO_SetVideoLayerPartitionMode(VoLayer,VO_PART_MODE_MULTI);    
		else
			s32Ret = HI_MPI_VO_SetVideoLayerPartitionMode(VoLayer,VO_PART_MODE_SINGLE);    
		if (s32Ret != HI_SUCCESS)
		{
			DPRINTK("HI_MPI_VO_SetVideoLayerPartitionMode failed with %#x!\n", s32Ret);
			 iRetNo = MSA_FAILED;
			goto err;
		}		

		//DPRINTK("SAMPLE_COMM_VO_StartLayer=%d\n",VoLayer);
		s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer,&stOutputDevLayerAttr[iOutputDevNum]);
		if (s32Ret != HI_SUCCESS)
		{
		    SAMPLE_PRT("failed with %#x!\n", s32Ret);
		     iRetNo = MSA_FAILED;
			goto err;
		}

		 gMsaCtrl.stInitDev.stOutputDevSetParameters.iDevCombineMode[iOutputDevNum] = enMode;
	}	

	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}

int MSA_PipLayerBindToDev(F_IN int iOutputDevNum)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	VO_LAYER VoLayer;
	int s32Ret;
	
	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( iOutputDevNum < 0 || iOutputDevNum >= gMsaCtrl.stInitDev.stOutputDevSetParameters.iOutputDevNum )
	{
		DPRINTK("iOutputDevNum=%d  not in avalible area [0-%d]\n",iOutputDevNum,\
			gMsaCtrl.stInitDev.stOutputDevSetParameters.iOutputDevNum-1);
		 iRetNo = MSA_ERR_INPUT_PARAMETERS_ERR;
		 goto err;
	}
	

	
	if( gMsaCtrl.stInitDev.stOutputDevSetParameters.iPipLayerBindDevId  != iOutputDevNum )
	{	
		 VoLayer = SAMPLE_VO_LAYER_VPIP;	
		 
		 s32Ret = SAMPLE_COMM_VO_StopLayer(VoLayer);
	    	if (s32Ret != HI_SUCCESS)
	    	{
	       	 	SAMPLE_PRT("failed with %#x!\n", s32Ret);
	        	iRetNo = MSA_FAILED;
			goto err;
	    	}

		DPRINTK("HI_MPI_VO_UnBindVideoLayer=%d\n",gMsaCtrl.stInitDev.stOutputDevSetParameters.iPipLayerBindDevId);
		s32Ret = HI_MPI_VO_UnBindVideoLayer(VoLayer ,gMsaCtrl.stInitDev.stOutputDevSetParameters.iPipLayerBindDevId);
		if (s32Ret != HI_SUCCESS)
		{
			DPRINTK("Pip video layer unbind to dev %d failed with errno %#x!\n",gMsaCtrl.stInitDev.stOutputDevSetParameters.iPipLayerBindDevId, s32Ret);
			iRetNo = MSA_FAILED;
			goto err;
		}			

		DPRINTK("HI_MPI_VO_BindVideoLayer=%d\n",iOutputDevNum);
		s32Ret = HI_MPI_VO_BindVideoLayer(VoLayer ,iOutputDevNum);
		if (s32Ret != HI_SUCCESS)
		{
			DPRINTK("Pip video layer bind to dev %d failed with errno %#x!\n",iOutputDevNum, s32Ret);
			iRetNo = MSA_FAILED;
			goto err;
		}

		s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stPipLayerAttr);    
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
			iRetNo = MSA_FAILED;
			goto err;
		}  

		gMsaCtrl.stInitDev.stOutputDevSetParameters.iPipLayerBindDevId = iOutputDevNum;
	}	
	

	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}

int MAS_ChanWindowCreateEncode(F_IN MSA_HANDLE hHandle, F_IN MSA_ENCODE_SET_ST stEncodeSet)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}	

	MSA_OpLock();

	if( pChan->iVencChan >= 0 )
	{
		iRetNo = MSA_ERR_DEV_IS_ALREADY_CREATE_ERR;
		goto err;		
	}

	if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_HARDWARE_COMBINE_MODE )
	{
		iRet =SAMPLE_COMM_VENC_Start(pChan->iId,PT_H264,VIDEO_ENCODING_MODE_PAL,stEncodeSet.iWidth,stEncodeSet.iHeight,stEncodeSet.enRc);
		if (HI_SUCCESS != iRet)
		{
			SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
			iRetNo = MSA_FAILED;
			goto err;
		}

		pChan->iVencChan = pChan->iId;
		pChan->stVencSet = stEncodeSet;		

		iRet = SetEncode(pChan->iVencChan,pChan->stVencSet.enRc,stEncodeSet.iGop,stEncodeSet.iFrameRate,stEncodeSet.iBitrate);
		if( iRet < 0 )
		{
			DPRINTK("SetEncode err\n");
		}
		
		//DPRINTK("SAMPLE_COMM_VENC_BindVpss: %d -> %d.%d\n",pChan->iVencChan, pChan->iVpssChan, 0);
		 iRet = SAMPLE_COMM_VENC_BindVpss(pChan->iVencChan, pChan->iVpssChan, 1);

		 if (HI_SUCCESS != iRet)
		 {
		     	DPRINTK("SAMPLE_COMM_VENC_BindVpss failed! iVencChan=%d  iVpssChan=%d\n",pChan->iVencChan, pChan->iVpssChan);
		     	iRetNo = MSA_FAILED;
			goto err;
		 }

		 iRet = Hisi_CreateStreamWorkThread(pChan);
		 if( iRet < 0 )
		{
			DPRINTK("Hisi_CreateStreamWorkThread err\n");
		}
		
	}else if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_SOFTWARE_COMBINE_MODE  )
	{		
		SAMPLE_PRT("MSA_ERR_CHAN_WINDOW_NOT_SUPPORT_ERR\n");	  
		iRetNo = MSA_ERR_CHAN_WINDOW_NOT_SUPPORT_ERR;
		goto err;			
	}
	
	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}


int MAS_ChanWindowDestroyEncode(F_IN MSA_HANDLE hHandle)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}	

	MSA_OpLock();

	if( pChan->iVencChan < 0 )
	{
		iRetNo = MSA_ERR_DEV_IS_NOT_CREATE_ERR;
		goto err;		
	}


	if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_HARDWARE_COMBINE_MODE )
	{
		 iRet = Hisi_DestroyStreamWorkThread(pChan);
		 if( iRet < 0 )
		{
			DPRINTK("Hisi_DestroyStreamWorkThread err\n");
		}
		 
		  iRet = SAMPLE_COMM_VENC_UnBindVpss(pChan->iVencChan, pChan->iVpssChan, 0);
		 if (HI_SUCCESS != iRet)
		 {
		     	DPRINTK("SAMPLE_COMM_VENC_UnBindVpss failed! iVencChan=%d  iVpssChan=%d\n",pChan->iVencChan, pChan->iVpssChan);
		     	iRetNo = MSA_FAILED;
			goto err;
		 }

		 iRet =SAMPLE_COMM_VENC_Stop(pChan->iVencChan);
		if (HI_SUCCESS != iRet)
		{
			SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
			iRetNo = MSA_FAILED;
			goto err;
		}

		pChan->iVencChan = -1;
		
	}else if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_SOFTWARE_COMBINE_MODE  )
	{		
		SAMPLE_PRT("MSA_ERR_CHAN_WINDOW_NOT_SUPPORT_ERR\n");	  
		iRetNo = MSA_ERR_CHAN_WINDOW_NOT_SUPPORT_ERR;
		goto err;			
	}

	
	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}


int MAS_GetEncodeInfo(F_IN MSA_HANDLE hHandle, F_OUT MSA_ENCODE_SET_ST * pstEncodeSet)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}	

	MSA_OpLock();

	iRet = GetEncode(pChan->iVencChan,pChan->stVencSet.enRc,&pChan->stVencSet.iGop,&pChan->stVencSet.iFrameRate,&pChan->stVencSet.iBitrate);
	if( iRet < 0 )
	{
		iRetNo = MSA_FAILED;
		goto err;
	}

	*pstEncodeSet = pChan->stVencSet;		

	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}

int MAS_SetEncodeInfo(F_IN MSA_HANDLE hHandle, F_IN MSA_ENCODE_SET_ST stEncodeSet)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}		

	iRet = SetEncode(pChan->iVencChan,pChan->stVencSet.enRc,stEncodeSet.iGop,stEncodeSet.iFrameRate,stEncodeSet.iBitrate);
	if( iRet < 0 )
	{
		iRetNo = MSA_FAILED;
		goto err;
	}
	
	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}


int MAS_ChanWindowCreateOSDLayer(F_IN MSA_HANDLE hHandle)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}	
	

	if( pChan->iOsdLayerChan >= 0 )
	{
		iRetNo = MSA_ERR_DEV_IS_ALREADY_CREATE_ERR;
		goto err;		
	}

	if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_HARDWARE_COMBINE_MODE )
	{

		iRet = SAMPLE_RGN_CreateOverlayForVpss(pChan->iId,pChan->iVpssChan,1920,1080);	
		if (HI_SUCCESS != iRet)
		{
			SAMPLE_PRT("SAMPLE_COMM_VDEC_BindVpss failed!\n");	  
			iRetNo = MSA_FAILED;
			goto err;
		}

		pChan->iOsdLayerChan = pChan->iId;

		
	}else if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_SOFTWARE_COMBINE_MODE  )
	{		
		SAMPLE_PRT("MSA_ERR_CHAN_WINDOW_NOT_SUPPORT_ERR\n");	  
		iRetNo = MSA_ERR_CHAN_WINDOW_NOT_SUPPORT_ERR;
		goto err;			
	}
	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}


int MAS_ChanWindowDestroyOSDLayer(F_IN MSA_HANDLE hHandle)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}		

	if( pChan->iOsdLayerChan < 0 )
	{
		iRetNo = MSA_ERR_DEV_IS_NOT_CREATE_ERR;
		goto err;		
	}


	if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_HARDWARE_COMBINE_MODE )
	{		
		iRet = SAMPLE_RGN_DestroyRegion(pChan->iOsdLayerChan);
	        if (HI_SUCCESS != iRet)
	        {
	            	DPRINTK("HI_MPI_RGN_Destroy failed! s32Ret: 0x%x.\n", iRet);
	         	iRetNo = MSA_FAILED;
			goto err;
	        }
		
		pChan->iOsdLayerChan = -1;
		
	}else if( MSA_GetCombineMode(pChan->stChan.enOutputDev)  == MSA_SOFTWARE_COMBINE_MODE  )
	{		
		SAMPLE_PRT("MSA_ERR_CHAN_WINDOW_NOT_SUPPORT_ERR\n");	  
		iRetNo = MSA_ERR_CHAN_WINDOW_NOT_SUPPORT_ERR;
		goto err;			
	}

	
	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}


int MAS_GetOSDLayerInfo(F_IN MSA_HANDLE hHandle, F_OUT MSA_OSD_LAYER_INFO * pstOsdLayerInfo)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}		

	iRet = GetOSDLayerInfo(pChan->iOsdLayerChan,&pstOsdLayerInfo->layer_width,&pstOsdLayerInfo->layer_height,&pstOsdLayerInfo->enPixelFmt);
   	if (iRet < 0)
	 {
	     	DPRINTK("GetOSDLayerInfo failed! s32Ret: 0x%x.\n", iRet);
	  	iRetNo = MSA_FAILED;
		goto err;
	 }
	
	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}

int MSA_OSDLayerDrawClear(MSA_HANDLE hHandle)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}	

	
	OSDLayerDrawClear(pChan->iOsdLayerChan);

	
	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}
	


int MSA_OSDLayerDrawPic(MSA_HANDLE hHandle, MSA_DRAW_OSD_PIC_ST stDrawOsd)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}	

	if( pChan->iOsdIsClear == 0 )
	{
		OSDLayerDrawClear(pChan->iOsdLayerChan);
		pChan->iOsdIsClear = 1;
	}

	iRet = OSDLayerDrawPicUseCoordinate(pChan->iOsdLayerChan,stDrawOsd.screen_x,stDrawOsd.screen_y,\
		stDrawOsd.pic_show_offset_x,stDrawOsd.pic_show_offset_y,stDrawOsd.pic_show_w,stDrawOsd.pic_show_h,\
		stDrawOsd.pic_width,stDrawOsd.pic_height,stDrawOsd.pic_data_ptr);
	if (iRet < 0)
	 {
	     	DPRINTK("OSDLayerDrawPicUseCoordinate failed! s32Ret: 0x%x.\n", iRet);
	  	iRetNo = MSA_FAILED;
		goto err;
	 }

	
	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}



int MSA_GetDecodePicSize(MSA_HANDLE hHandle,int * pic_width,int * pic_height)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	MSA_CHANNEL_ST tmpChan; 	
	MSA_CHANNEL_INFO_ST * pChan = (MSA_CHANNEL_INFO_ST *)hHandle;

	if( gMsaCtrl.isInit == 0 )	
		return MSA_ERR_NOT_INIT;

	MSA_OpLock();

	if( hHandle == NULL )
	{
		iRetNo = MSA_ERR_NULL_PTR;
		goto err;
	}	

	if( pChan->iVdecChan < 0 )
	{
		iRetNo = MSA_ERR_DEC_ERR;
		goto err;
	}


	{
		FILE *procpt;
		char line[256];
		char strArray[10][56];
		int num[10];
		int find_data = 0;
		
		procpt = fopen("/proc/umap/vdec", "r");
		if (procpt == NULL) {
			fprintf(stderr, ("cannot open %s\n"), "/proc/umap/vdec");
			return -1;
		}	
	
		while (fgets(line, sizeof(line), procpt)) 
		{
			if (sscanf (line, " %d  %s %d %d %d %d %d %s %s %s ",
					&num[0], strArray[1], &num[2], &num[3],&num[4], &num[5], &num[6], &strArray[7],&strArray[8], &strArray[9]) != 10)
				continue;			

			if( pChan->iVdecChan == num[0] )
			{
				//DPRINTK("Get chan=%d w=%d h=%d\n",num[0], num[5], num[6]);
				*pic_width = num[5];
				*pic_height = num[6];
				find_data = 1;
				break;
			}
		}
	
		fclose(procpt);

		if( find_data == 0 )
		{
			iRetNo = MSA_ERR_DEC_ERR;
			goto err;
		}
	
	}
	
	MSA_OpUnLock();
	return iRetNo;
err:	
	MSA_OpUnLock();
	return iRetNo;
}



#ifdef __cplusplus
}
#endif /* __cplusplus */



