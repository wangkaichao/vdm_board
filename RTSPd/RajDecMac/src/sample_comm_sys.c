/******************************************************************************
  Some simple Hisilicon Hi3531 system functions.

  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#define DEBUG
#include "msa_debug.h"
#include "media_sys_api.h"
#include "osd.h"

int g_cover_vpss_chn = 1;
HI_S32 g_s32VBSource = 0;
VB_POOL g_ahVbPool[VB_MAX_POOLS] = {[0 ... (VB_MAX_POOLS-1)] = VB_INVALID_POOLID};

/******************************************************************************
* function : get picture size(w*h), according Norm and enPicSize
******************************************************************************/
HI_S32 SAMPLE_COMM_SYS_GetPicSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S *pstSize)
{
    switch (enPicSize)
    {
        case PIC_QCIF:
            pstSize->u32Width = D1_WIDTH / 4;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?144:120;
            break;
        case PIC_CIF:
            pstSize->u32Width = D1_WIDTH / 2;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?288:240;
            break;
        case PIC_D1:
            pstSize->u32Width = D1_WIDTH;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case PIC_960H:
            pstSize->u32Width = 960;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;			
        case PIC_2CIF:
            pstSize->u32Width = D1_WIDTH / 2;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case PIC_QVGA:    /* 320 * 240 */
            pstSize->u32Width = 320;
            pstSize->u32Height = 240;
            break;
        case PIC_VGA:     /* 640 * 480 */
            pstSize->u32Width = 640;
            pstSize->u32Height = 480;
            break;
        case PIC_XGA:     /* 1024 * 768 */
            pstSize->u32Width = 1024;
            pstSize->u32Height = 768;
            break;
        case PIC_SXGA:    /* 1400 * 1050 */
            pstSize->u32Width = 1400;
            pstSize->u32Height = 1050;
            break;
        case PIC_UXGA:    /* 1600 * 1200 */
            pstSize->u32Width = 1600;
            pstSize->u32Height = 1200;
            break;
        case PIC_QXGA:    /* 2048 * 1536 */
            pstSize->u32Width = 2048;
            pstSize->u32Height = 1536;
            break;
        case PIC_WVGA:    /* 854 * 480 */
            pstSize->u32Width = 854;
            pstSize->u32Height = 480;
            break;
        case PIC_WSXGA:   /* 1680 * 1050 */
            pstSize->u32Width = 1680;
            pstSize->u32Height = 1050;
            break;
        case PIC_WUXGA:   /* 1920 * 1200 */
            pstSize->u32Width = 1920;
            pstSize->u32Height = 1200;
            break;
        case PIC_WQXGA:   /* 2560 * 1600 */
            pstSize->u32Width = 2560;
            pstSize->u32Height = 1600;
            break;
        case PIC_HD720:   /* 1280 * 720 */
            pstSize->u32Width = 1280;
            pstSize->u32Height = 720;
            break;
        case PIC_HD1080:  /* 1920 * 1080 */
            pstSize->u32Width = 1920;
            pstSize->u32Height = 1080;
            break;
        default:
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/******************************************************************************
* function : calculate VB Block size of Histogram.
******************************************************************************/
HI_U32 SAMPLE_COMM_SYS_CalcHistVbBlkSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S *pstHistBlkSize, HI_U32 u32AlignWidth)
{
    HI_S32 s32Ret;
    SIZE_S stPicSize;
    
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, enPicSize, &stPicSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get picture size[%d] failed!\n", enPicSize);
            return HI_FAILURE;
    }

   SAMPLE_PRT("stPicSize.u32Width%d,pstHistBlkSize->u32Width%d\n,stPicSize.u32Height%d,pstHistBlkSize->u32Height%d\n",
   	stPicSize.u32Width,pstHistBlkSize->u32Width,
   	stPicSize.u32Height,pstHistBlkSize->u32Height );
    return (CEILING_2_POWER(44, u32AlignWidth)*CEILING_2_POWER(44, u32AlignWidth)*16*4);

    return HI_SUCCESS;
}

/******************************************************************************
* function : calculate VB Block size of picture.
******************************************************************************/
HI_U32 SAMPLE_COMM_SYS_CalcPicVbBlkSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, PIXEL_FORMAT_E enPixFmt, HI_U32 u32AlignWidth)
{
    HI_S32 s32Ret = HI_FAILURE;
    SIZE_S stSize;
    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;
    HI_U32 u32BlkSize = 0;

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get picture size[%d] failed!\n", enPicSize);
            return HI_FAILURE;
    }

    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 != enPixFmt && PIXEL_FORMAT_YUV_SEMIPLANAR_420 != enPixFmt)
    {
        SAMPLE_PRT("pixel format[%d] input failed!\n", enPixFmt);
            return HI_FAILURE;
    }

    if (16!=u32AlignWidth && 32!=u32AlignWidth && 64!=u32AlignWidth)
    {
        SAMPLE_PRT("system align width[%d] input failed!\n",\
               u32AlignWidth);
            return HI_FAILURE;
    }
    if (704 == stSize.u32Width)
    {
        stSize.u32Width = 720;
    }    
    //SAMPLE_PRT("w:%d, u32AlignWidth:%d\n", CEILING_2_POWER(stSize.u32Width,u32AlignWidth), u32AlignWidth);

    u32Width  = CEILING_2_POWER(stSize.u32Width, u32AlignWidth);
    u32Height = CEILING_2_POWER(stSize.u32Height,u32AlignWidth);
    
    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixFmt)
    {
        u32BlkSize = u32Width * u32Height * 2;
    }
    else
    {
        u32BlkSize = u32Width * u32Height * 3 / 2;
    }
    
    return u32BlkSize;
}

/******************************************************************************
* function : Set system memory location
******************************************************************************/
HI_S32 SAMPLE_COMM_SYS_MemConfig(HI_VOID)
{
    HI_S32 i = 0;
    HI_S32 s32Ret = HI_SUCCESS;

    HI_CHAR * pcMmzName = NULL;
    MPP_CHN_S stMppChnVO;
    MPP_CHN_S stMppChnVPSS;
    MPP_CHN_S stMppChnVENC;
    MPP_CHN_S stMppChnVDEC;

    /*VI,VDEC最大通道数为32*/
    for(i=0; i<VDEC_MAX_CHN_NUM; i++)
    {        
        stMppChnVDEC.enModId = HI_ID_VDEC;
        stMppChnVDEC.s32DevId = 0;
        stMppChnVDEC.s32ChnId = i;

        /*vdec*/
        s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVDEC,pcMmzName);
        if (s32Ret)
        {
            SAMPLE_PRT("HI_MPI_SYS_SetMemConf ERR !\n");
            return HI_FAILURE;
        }

    }  

    /*vpss,grp,venc最大通道为64*/
    for(i=0;i<64;i++)
    {
        stMppChnVPSS.enModId  = HI_ID_VPSS;
        stMppChnVPSS.s32DevId = i;
        stMppChnVPSS.s32ChnId = 0;

        stMppChnVENC.enModId = HI_ID_VENC;
        stMppChnVENC.s32DevId = 0;
        stMppChnVENC.s32ChnId = i;

        /*vpss*/
        s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVPSS, pcMmzName);
        if (s32Ret)
        {
            SAMPLE_PRT("HI_MPI_SYS_SetMemConf ERR !\n");
            return HI_FAILURE;
        }

        /*venc*/
        s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVENC,pcMmzName);
        if (s32Ret)
        {
            SAMPLE_PRT("HI_MPI_SYS_SetMemConf ERR !\n");
            return HI_FAILURE;
        }

    }
            
    /*配置VO内存*/
    stMppChnVO.enModId  = HI_ID_VOU;
    stMppChnVO.s32DevId = 0;
    stMppChnVO.s32ChnId = 0;
    s32Ret = HI_MPI_SYS_SetMemConf(&stMppChnVO, pcMmzName);
    if (s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_SetMemConf ERR !\n");
        return HI_FAILURE;
    } 
    
    return s32Ret;
}

/******************************************************************************
* function : vb init & MPI system init
******************************************************************************/
HI_S32 SAMPLE_COMM_SYS_Init(VB_CONF_S *pstVbConf)
{
    MPP_SYS_CONF_S stSysConf = {0};
    HI_S32 s32Ret = HI_FAILURE;
    HI_S32 i;

    HI_MPI_SYS_Exit();
    for(i=0;i<VB_MAX_USER;i++)
    {
         HI_MPI_VB_ExitModCommPool(i);
    }
	for(i=0; i<VB_MAX_POOLS; i++)
    {
         HI_MPI_VB_DestroyPool(i);
    }
    HI_MPI_VB_Exit();

    if (NULL == pstVbConf)
    {
        SAMPLE_PRT("input parameter is null, it is invaild!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_SetConf(pstVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_SetConf failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VB_Init failed!\n");
        return HI_FAILURE;
    }

    stSysConf.u32AlignWidth = SAMPLE_SYS_ALIGN_WIDTH;
    s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_SetConf failed\n");
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Init failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/******************************************************************************
* function : vb init & MPI system init
******************************************************************************/
HI_S32 SAMPLE_COMM_SYS_Payload2FilePostfix(PAYLOAD_TYPE_E enPayload, HI_CHAR* szFilePostfix)
{
    if (PT_H264 == enPayload)
    {
        strcpy(szFilePostfix, ".h264");
    }
    else if (PT_JPEG == enPayload)
    {
        strcpy(szFilePostfix, ".jpg");
    }
    else if (PT_MJPEG == enPayload)
    {
        strcpy(szFilePostfix, ".mjp");
    }
    else
    {
        SAMPLE_PRT("payload type err!\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/******************************************************************************
* function : vb exit & MPI system exit
******************************************************************************/
HI_VOID SAMPLE_COMM_SYS_Exit(void)
{

    HI_S32 i;

    HI_MPI_SYS_Exit();
    for(i=0;i<VB_MAX_USER;i++)
    {
         HI_MPI_VB_ExitModCommPool(i);
    }
    for(i=0; i<VB_MAX_POOLS; i++)
    {
         HI_MPI_VB_DestroyPool(i);
    }	
    HI_MPI_VB_Exit();
    return;
}

HI_S32 SAMPLE_COMM_VO_StopDev(VO_DEV VoDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_MPI_VO_Disable(VoDev);

    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
}

HI_S32 SAMPLE_COMM_VO_BindVpss(VO_LAYER VoLayer,VO_CHN VoChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
	
    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;
    stDestChn.enModId = HI_ID_VOU;
    stDestChn.s32DevId = VoLayer;
    stDestChn.s32ChnId = VoChn;
	
    DPRINTK("(%d)->(%d.%d)\n",VpssGrp,VoLayer,VoChn);
    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
}

HI_S32 SAMPLE_COMM_VO_StartLayer(VO_LAYER VoLayer,const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, pstLayerAttr);

    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
}

HI_S32 SAMPLE_COMM_VDEC_UnBindVo(VDEC_CHN VdChn, VO_LAYER VoLayer, VO_CHN VoChn)
{
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
	
    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;
    stDestChn.enModId = HI_ID_VOU;
    stDestChn.s32DevId = VoLayer;
    stDestChn.s32ChnId = VoChn;
	
    DPRINTK("(%d)->(%d.%d)\n",VdChn,VoLayer,VoChn);
    CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "HI_MPI_SYS_UnBind");
    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VO_StopLayer(VO_LAYER VoLayer)
{
    HI_S32 s32Ret = HI_SUCCESS;
	
    s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    return s32Ret;
}

HI_S32 SAMPLE_COMM_VDEC_BindVpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp)
{
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;
    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    DPRINTK("%d->(%d)\n",VdChn,VpssGrp);
    CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind");
    return HI_SUCCESS;
}


HI_S32 SAMPLE_COMM_VO_UnBindVpss(VO_LAYER VoLayer,VO_CHN VoChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;
    stDestChn.enModId = HI_ID_VOU;
    stDestChn.s32DevId = VoLayer;
    stDestChn.s32ChnId = VoChn;
	
    DPRINTK("(%d)->(%d.%d)\n",VpssGrp,VoLayer,VoChn);
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

HI_S32 SAMPLE_COMM_VO_StartDev(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    //printf("-----------------dev:%d\n", VoDev);
    s32Ret = HI_MPI_VO_SetPubAttr(VoDev, pstPubAttr);

    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_Enable(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

HI_S32 SAMPLE_COMM_VDEC_BindVo(VDEC_CHN VdChn, VO_LAYER VoLayer, VO_CHN VoChn)
{
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;
    stDestChn.enModId = HI_ID_VOU;
    stDestChn.s32DevId = VoLayer;
    stDestChn.s32ChnId = VoChn;

    DPRINTK("%d->(%d.%d)\n",VdChn,VoLayer,VoChn);
    CHECK_RET(HI_MPI_SYS_Bind(&stSrcChn, &stDestChn), "HI_MPI_SYS_Bind");
    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VO_GetWH(VO_INTF_SYNC_E enIntfSync, HI_U32 *pu32W,HI_U32 *pu32H, HI_U32 *pu32Frm)
{
    switch (enIntfSync)
    {
    case VO_OUTPUT_PAL:
        *pu32W = 720;
        *pu32H = 576;
        *pu32Frm = 25;
        break;

    case VO_OUTPUT_NTSC:
        *pu32W = 720;
        *pu32H = 480;
        *pu32Frm = 30;
        break;

    case VO_OUTPUT_576P50:
        *pu32W = 720;
        *pu32H = 576;
        *pu32Frm = 50;
        break;

    case VO_OUTPUT_480P60:
        *pu32W = 720;
        *pu32H = 480;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_800x600_60:
        *pu32W = 800;
        *pu32H = 600;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_720P50:
        *pu32W = 1280;
        *pu32H = 720;
        *pu32Frm = 50;
        break;

    case VO_OUTPUT_720P60:
        *pu32W = 1280;
        *pu32H = 720;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_1080I50:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 50;
        break;

    case VO_OUTPUT_1080I60:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_1080P24:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 24;
        break;

    case VO_OUTPUT_1080P25:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 25;
        break;

    case VO_OUTPUT_1080P30:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 30;
        break;

    case VO_OUTPUT_1080P50:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 50;
        break;

    case VO_OUTPUT_1080P60:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_1024x768_60:
        *pu32W = 1024;
        *pu32H = 768;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_1280x1024_60:
        *pu32W = 1280;
        *pu32H = 1024;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_1366x768_60:
        *pu32W = 1366;
        *pu32H = 768;
        *pu32Frm = 60;
        break;

	case VO_OUTPUT_1440x900_60:
        *pu32W = 1440;
        *pu32H = 900;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_1280x800_60:
        *pu32W = 1280;
        *pu32H = 800;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_1600x1200_60:
        *pu32W = 1600;
        *pu32H = 1200;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_1680x1050_60:
        *pu32W = 1680;
        *pu32H = 1050;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_1920x1200_60:
        *pu32W = 1920;
        *pu32H = 1200;
        *pu32Frm = 60;
        break;

    case VO_OUTPUT_USER:
        *pu32W = 720;
        *pu32H = 576;
        *pu32Frm = 25;
        break;

    default:
        SAMPLE_PRT("vo enIntfSync not support!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VDEC_UnBindVpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp)
{
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;
	
    DPRINTK("(%d)->(%d)\n",VdChn,VpssGrp);
    CHECK_RET(HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn), "HI_MPI_SYS_UnBind");
    return HI_SUCCESS;
}

static HI_VOID SAMPLE_COMM_VO_HdmiConvertSync(VO_INTF_SYNC_E enIntfSync, HI_HDMI_VIDEO_FMT_E *penVideoFmt)
{
    switch (enIntfSync)
    {
    case VO_OUTPUT_PAL:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_PAL;
        break;

    case VO_OUTPUT_NTSC:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_NTSC;
        break;
		
    case VO_OUTPUT_1080P24:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_24;
        break;

    case VO_OUTPUT_1080P25:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_25;
        break;

    case VO_OUTPUT_1080P30:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_30;
        break;

    case VO_OUTPUT_720P50:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_50;
        break;

    case VO_OUTPUT_720P60:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_60;
        break;

    case VO_OUTPUT_1080I50:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_50;
        break;

    case VO_OUTPUT_1080I60:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_60;
        break;

    case VO_OUTPUT_1080P50:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_50;
        break;

    case VO_OUTPUT_1080P60:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
        break;

    case VO_OUTPUT_576P50:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_576P_50;
        break;

    case VO_OUTPUT_480P60:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_480P_60;
        break;

    case VO_OUTPUT_800x600_60:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_800X600_60;
        break;

    case VO_OUTPUT_1024x768_60:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1024X768_60;
        break;

    case VO_OUTPUT_1280x1024_60:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X1024_60;
        break;

    case VO_OUTPUT_1366x768_60:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1366X768_60;
        break;

    case VO_OUTPUT_1440x900_60:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1440X900_60;
        break;

    case VO_OUTPUT_1280x800_60:
        *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X800_60;
        break;

    default :
        SAMPLE_PRT("Unkonw VO_INTF_SYNC_E value!\n");
        break;
    }
}

HI_S32 SAMPLE_COMM_VO_HdmiStart(VO_INTF_SYNC_E enIntfSync)
{
    HI_HDMI_INIT_PARA_S stHdmiPara;
    HI_HDMI_ATTR_S      stAttr;
    HI_HDMI_VIDEO_FMT_E enVideoFmt;

    SAMPLE_COMM_VO_HdmiConvertSync(enIntfSync, &enVideoFmt);
    stHdmiPara.enForceMode = HI_HDMI_FORCE_HDMI;
    stHdmiPara.pCallBackArgs = NULL;
    stHdmiPara.pfnHdmiEventCallback = NULL;
    HI_MPI_HDMI_Init(&stHdmiPara);
    HI_MPI_HDMI_Open(HI_HDMI_ID_0);
    HI_MPI_HDMI_GetAttr(HI_HDMI_ID_0, &stAttr);

    stAttr.bEnableHdmi = HI_TRUE;
    stAttr.bEnableVideo = HI_TRUE;
    stAttr.enVideoFmt = enVideoFmt;
    stAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
    stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
    stAttr.bxvYCCMode = HI_FALSE;
    stAttr.bEnableAudio = HI_FALSE;
    stAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
    stAttr.bIsMultiChannel = HI_FALSE;
    stAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;
    stAttr.bEnableAviInfoFrame = HI_TRUE;
    stAttr.bEnableAudInfoFrame = HI_TRUE;
    stAttr.bEnableSpdInfoFrame = HI_FALSE;
    stAttr.bEnableMpegInfoFrame = HI_FALSE;
    stAttr.bDebugFlag = HI_FALSE;
    stAttr.bHDCPEnable = HI_FALSE;
    stAttr.b3DEnable = HI_FALSE;
    HI_MPI_HDMI_SetAttr(HI_HDMI_ID_0, &stAttr);
    HI_MPI_HDMI_Start(HI_HDMI_ID_0);

    printf("HDMI start success.\n");
    return HI_SUCCESS;
}

HI_VOID	SAMPLE_COMM_VDEC_ModCommPoolConf(VB_CONF_S *pstModVbConf, PAYLOAD_TYPE_E enType, SIZE_S *pstSize)
{
    HI_S32 PicSize, PmvSize;

    memset(pstModVbConf, 0, sizeof(VB_CONF_S));
    pstModVbConf->u32MaxPoolCnt = 2;
    VB_PIC_BLK_SIZE(pstSize->u32Width, pstSize->u32Height, enType, PicSize);
    pstModVbConf->astCommPool[0].u32BlkSize = PicSize;
    pstModVbConf->astCommPool[0].u32BlkCnt  = 40;

    /* NOTICE:
       1. if the VDEC channel is H264 channel and support to decode B frame, then you should allocate PmvBuffer
       2. if the VDEC channel is MPEG4 channel, then you should allocate PmvBuffer.
    */
    if(PT_MP4VIDEO == enType)
    {
        VB_PMV_BLK_SIZE(pstSize->u32Width, pstSize->u32Height, PmvSize);
        pstModVbConf->astCommPool[1].u32BlkSize = PmvSize;
        pstModVbConf->astCommPool[1].u32BlkCnt  = 20;
    }
}

HI_S32 SAMPLE_COMM_VDEC_InitModCommVb(VB_CONF_S *pstModVbConf)
{
    HI_S32 i;
    HI_S32 s32Ret;

    HI_MPI_VB_ExitModCommPool(VB_UID_VDEC);
    if(0 == g_s32VBSource)
    {
        CHECK_RET(HI_MPI_VB_SetModPoolConf(VB_UID_VDEC, pstModVbConf), "HI_MPI_VB_SetModPoolConf");
        CHECK_RET(HI_MPI_VB_InitModCommPool(VB_UID_VDEC), "HI_MPI_VB_InitModCommPool");
    }

    else if (2 == g_s32VBSource)
    {
        if (pstModVbConf->u32MaxPoolCnt > VB_MAX_POOLS)
        {
            printf("vb pool num(%d) is larger than VB_MAX_POOLS. \n", pstModVbConf->u32MaxPoolCnt);
            return HI_FAILURE;
        }

        for (i = 0; i < pstModVbConf->u32MaxPoolCnt; i++)
        {
            if (pstModVbConf->astCommPool[i].u32BlkSize && pstModVbConf->astCommPool[i].u32BlkCnt)
            {
                g_ahVbPool[i] = HI_MPI_VB_CreatePool(pstModVbConf->astCommPool[i].u32BlkSize, pstModVbConf->astCommPool[i].u32BlkCnt, NULL);
                
                if (VB_INVALID_POOLID == g_ahVbPool[i])
                    goto fail;
            }
        }
        return HI_SUCCESS;
fail:
        for (; i>=0; i--)
        {
            if (VB_INVALID_POOLID != g_ahVbPool[i])
            {
                s32Ret = HI_MPI_VB_DestroyPool(g_ahVbPool[i]);
                HI_ASSERT(HI_SUCCESS == s32Ret);
                g_ahVbPool[i] = VB_INVALID_POOLID;
            }
        }
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

//======================================================================
/******************************************************************************

* funciton : Start venc stream mode (h264, mjpeg)

* note      : rate control parameter need adjust, according your case.

******************************************************************************/

HI_S32 SAMPLE_COMM_VENC_Start(VENC_CHN VencChn, PAYLOAD_TYPE_E enType, VIDEO_NORM_E enNorm, int width,int height, SAMPLE_RC_E enRcMode)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_H264_S stH264Attr;
    VENC_ATTR_H264_CBR_S    stH264Cbr;
    VENC_ATTR_H264_VBR_S    stH264Vbr;
    VENC_ATTR_H264_FIXQP_S  stH264FixQp;
    VENC_ATTR_MJPEG_S stMjpegAttr;
    VENC_ATTR_MJPEG_FIXQP_S stMjpegeFixQp;
    VENC_ATTR_JPEG_S stJpegAttr;

    stVencChnAttr.stVeAttr.enType = enType;
    switch(enType)	{		case PT_H264:		{
            stH264Attr.u32MaxPicWidth = width;	      stH264Attr.u32MaxPicHeight = height;
            stH264Attr.u32PicWidth = width;
            stH264Attr.u32PicHeight = height;
            stH264Attr.u32BufSize  = width * height * 2;
            stH264Attr.u32Profile  = 0;
            stH264Attr.bByFrame = HI_TRUE;
            memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));

            if(SAMPLE_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                stH264Cbr.u32Gop            = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Cbr.u32StatTime       = 1; 
                stH264Cbr.u32SrcFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Cbr.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                 stH264Cbr.u32BitRate = 1024*4;
                stH264Cbr.u32FluctuateLevel = 0; 
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
            }
            else if (SAMPLE_RC_FIXQP == enRcMode) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.u32IQp = 20;
                stH264FixQp.u32PQp = 23;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
            }
            else if (SAMPLE_RC_VBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                stH264Vbr.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.u32StatTime = 1;
                stH264Vbr.u32SrcFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.fr32DstFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.u32MinQp = 10;
                stH264Vbr.u32MaxQp = 40;
                stH264Vbr.u32MaxBitRate = 1024*4*3;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
            }
            else
            {
                return HI_FAILURE;
            }
        	}
        break;
        case PT_JPEG:
            stJpegAttr.u32PicWidth  = width;
            stJpegAttr.u32PicHeight = height;
            stJpegAttr.u32BufSize = width * height * 2;
            stJpegAttr.bByFrame = HI_TRUE;
            memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));
            break;
        default:
            return HI_ERR_VENC_NOT_SUPPORT;
    }

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                VencChn, s32Ret);
        return s32Ret;
    }
    s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)	{
        SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}


HI_S32 SAMPLE_COMM_VENC_Stop(VENC_CHN VencChn)
{
    HI_S32 s32Ret;
    s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VENC_BindVpss(VENC_CHN VeChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;
	
	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp;	
	stSrcChn.s32ChnId = VpssChn;
	
	stDestChn.enModId = HI_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = VeChn;
	s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
	
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);	
		return HI_FAILURE;
	}
	return s32Ret;	
}
	
HI_S32 SAMPLE_COMM_VENC_UnBindVpss(VENC_CHN VeChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MPP_CHN_S stSrcChn;
	MPP_CHN_S stDestChn;

	stSrcChn.enModId = HI_ID_VPSS;
	stSrcChn.s32DevId = VpssGrp;
	stSrcChn.s32ChnId = VpssChn;
	stDestChn.enModId = HI_ID_VENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = VeChn;

	s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
	return s32Ret;
}

HI_S32 SAMPLE_RGN_CreateOverlayForVpss(RGN_HANDLE Handle,int vpss_id,int width,int height)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CHN_ATTR_S stChnAttr;

    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = vpss_id;
    stChn.s32ChnId = 0;

    i = Handle;

    stRgnAttrSet.enType = OVERLAY_RGN;
    stRgnAttrSet.unAttr.stOverlay.enPixelFmt       = PIXEL_FORMAT_RGB_1555;
    stRgnAttrSet.unAttr.stOverlay.stSize.u32Width  = width;
    stRgnAttrSet.unAttr.stOverlay.stSize.u32Height = height;
    stRgnAttrSet.unAttr.stOverlay.u32BgColor       = 0x0000fc00;  

    s32Ret = HI_MPI_RGN_Create(i, &stRgnAttrSet);
    if(s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_RGN_Create failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    stChnAttr.bShow  = HI_TRUE;
    stChnAttr.enType = OVERLAY_RGN;
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 0;
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 0;
    stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 255;
    stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 0;
    stChnAttr.unChnAttr.stOverlayChn.u32Layer     = i;   

    s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        printf("HI_MPI_RGN_AttachToChn failed! s32Ret: 0x%x.\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_RGN_DestroyRegion(RGN_HANDLE Handle)
{
    HI_S32 i;
    HI_S32 s32Ret;            

     s32Ret = HI_MPI_RGN_Destroy(Handle);
     if (HI_SUCCESS != s32Ret)
     {
         printf("HI_MPI_RGN_Destroy failed! s32Ret: 0x%x.\n", s32Ret);
         return s32Ret;
     }  

    return HI_SUCCESS;
}

HI_S32 StartVpss(HI_S32 s32GrpNo)
{
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr = {0};
    VPSS_CHN_ATTR_S stChnAttr = {0};
    VPSS_GRP_PARAM_S stVpssParam = {0};
    VPSS_CHN_MODE_S stVpssChnMode;
    HI_S32 s32Ret;
    HI_S32 i, j;
    HI_S32 s32ChnCnt = 5;

    DPRINTK(" %d\n",s32GrpNo);
    /*** Set Vpss Grp Attr ***/
    //stGrpAttr.u32MaxW = pstSize->u32Width;
	//stGrpAttr.u32MaxH = pstSize->u32Height;
	stGrpAttr.u32MaxW = 1920;
	stGrpAttr.u32MaxH = 1200;
	stGrpAttr.enDieMode    = VPSS_DIE_MODE_NODIE;
	stGrpAttr.bIeEn        = HI_FALSE;
	stGrpAttr.bNrEn        = HI_FALSE;
	stGrpAttr.bHistEn      = HI_FALSE;
	stGrpAttr.bDciEn       = HI_FALSE;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
   
    {
        VpssGrp = s32GrpNo;
        /*** create vpss group ***/
        s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stGrpAttr);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
		
        /*** enable vpss chn, with frame ***/
        for(j=0; j<VPSS_MAX_CHN_NUM; j++)
        {
            VpssChn = j;
            stChnAttr.bSpEn = HI_FALSE;
            stChnAttr.bBorderEn = HI_FALSE;
            stChnAttr.stBorder.u32Color = 0xff00;
            stChnAttr.stBorder.u32LeftWidth = 2;
            stChnAttr.stBorder.u32RightWidth = 2;
            stChnAttr.stBorder.u32TopWidth = 2;
            stChnAttr.stBorder.u32BottomWidth = 2;
            s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stChnAttr);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }    

            s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
        }
        
		stVpssChnMode.bDouble 	     = HI_FALSE;
		stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_AUTO;
		stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
		stVpssChnMode.u32Width 	     = 1920;
		stVpssChnMode.u32Height 	 = 1080;
		stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
		s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, g_cover_vpss_chn-1, &stVpssChnMode);
        if (HI_SUCCESS != s32Ret)
        {
            printf("set vpss grp%d chn%d mode fail! s32Ret: 0x%x.\n", i, g_cover_vpss_chn, s32Ret);
            return s32Ret;
        }
        
        s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, g_cover_vpss_chn, &stVpssChnMode);
        if (HI_SUCCESS != s32Ret)
        {
            printf("set vpss grp%d chn%d mode fail! s32Ret: 0x%x.\n", i, g_cover_vpss_chn, s32Ret);
            return s32Ret;
        }

        if (s32GrpNo == 0) {
            /* for DSD0 */
            stVpssChnMode.bDouble 	     = HI_FALSE;
    		stVpssChnMode.enChnMode 	 = VPSS_CHN_MODE_USER;
    		stVpssChnMode.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    		stVpssChnMode.u32Width 	     = 720;
    		stVpssChnMode.u32Height 	 = 576;
    		stVpssChnMode.enCompressMode = COMPRESS_MODE_NONE;
    		s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, g_cover_vpss_chn + 1, &stVpssChnMode);
            if (HI_SUCCESS != s32Ret)
            {
                printf("set vpss grp%d chn%d mode fail! s32Ret: 0x%x.\n", i, g_cover_vpss_chn + 1, s32Ret);
                return s32Ret;
            }
        }
        
        /*** start vpss group ***/
        s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }

		{
			HI_U32 u32Depth;
			HI_U32 u32OverlayMask;
			u32Depth = 6;
	        s32Ret = HI_MPI_VPSS_SetDepth(VpssGrp, g_cover_vpss_chn, u32Depth);
	        if (HI_SUCCESS != s32Ret)
	        {
	            printf("HI_MPI_VPSS_SetDepth fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, g_cover_vpss_chn, s32Ret);
	            return s32Ret;
	        }

			s32Ret = HI_MPI_VPSS_SetDepth(VpssGrp, g_cover_vpss_chn-1, u32Depth);
	        if (HI_SUCCESS != s32Ret)
	        {
	            printf("HI_MPI_VPSS_SetDepth fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, g_cover_vpss_chn, s32Ret);
	            return s32Ret;
	        }

	        if (s32GrpNo == 0) {
                /* for DSD0 */
    	        s32Ret = HI_MPI_VPSS_SetDepth(VpssGrp, g_cover_vpss_chn+1, u32Depth);
    	        if (HI_SUCCESS != s32Ret)
    	        {
    	            printf("HI_MPI_VPSS_SetDepth fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, g_cover_vpss_chn, s32Ret);
    	            return s32Ret;
    	        }
	        }

	        u32OverlayMask = 255;
	        s32Ret = HI_MPI_VPSS_SetChnOverlay(VpssGrp, g_cover_vpss_chn, u32OverlayMask);
	        if (HI_SUCCESS != s32Ret)
	        {
	            printf("HI_MPI_VPSS_SetChnOverlay fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, g_cover_vpss_chn, s32Ret);
	            return s32Ret;
			}

			s32Ret = HI_MPI_VPSS_SetChnOverlay(VpssGrp, g_cover_vpss_chn-1, u32OverlayMask);
	        if (HI_SUCCESS != s32Ret)
	        {
	            printf("HI_MPI_VPSS_SetChnOverlay fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, g_cover_vpss_chn, s32Ret);
	            return s32Ret;
	        }

            if (s32GrpNo == 0) {
    	        s32Ret = HI_MPI_VPSS_SetChnOverlay(VpssGrp, g_cover_vpss_chn+1, u32OverlayMask);
    	        if (HI_SUCCESS != s32Ret)
    	        {
    	            printf("HI_MPI_VPSS_SetChnOverlay fail! Grp: %d, Chn: %d! s32Ret: 0x%x.\n", i, g_cover_vpss_chn, s32Ret);
    	            return s32Ret;
    	        }
	        }
		}
    }

    return HI_SUCCESS;
}

/*****************************************************************************

* function : disable vi dev

*****************************************************************************/
HI_S32 StopVpss(HI_S32 s32GrpNo)
{
    HI_S32 i, j;
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;

	if( s32GrpNo < 0 )
	{
		printf("s32GrpNo = %d,err\n",s32GrpNo);
		return HI_FAILURE;
	}

	DPRINTK(" %d\n",s32GrpNo);
    {
        VpssGrp = s32GrpNo;
        s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }

        for(j=0; j<VPSS_MAX_CHN_NUM; j++)
        {
            VpssChn = j;
            s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }
        }

        s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }
    return HI_SUCCESS;
}

HI_S32 StartVoChn(VO_LAYER VoLayer,int iChan, int iPosStartX,int iPosStartY,int iWidth,int iHeight)
{
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32WndNum = 0;
    HI_U32 u32Square = 0;
    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;
    VO_CHN_ATTR_S stChnAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;  

    s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    u32Width = stLayerAttr.stImageSize.u32Width;
    u32Height = stLayerAttr.stImageSize.u32Height;
    DPRINTK("u32Width:%d, u32Height:%d\n", u32Width, u32Height);
	stChnAttr.stRect.s32X       = ALIGN_BACK(iPosStartX, 2);
	stChnAttr.stRect.s32Y       = ALIGN_BACK(iPosStartY, 2);
	stChnAttr.stRect.u32Width   = ALIGN_BACK(iWidth, 2);
	stChnAttr.stRect.u32Height  = ALIGN_BACK(iHeight, 2);
	stChnAttr.u32Priority       = VO_MAX_CHN_NUM - 1; //0
	stChnAttr.bDeflicker        = HI_FALSE;

	DPRINTK("x:%d y:%d w:%d h:%d\n",stChnAttr.stRect.s32X,stChnAttr.stRect.s32Y,stChnAttr.stRect.u32Width,stChnAttr.stRect.u32Height);
	s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, iChan, &stChnAttr);
	if (s32Ret != HI_SUCCESS)
	{
		printf("%s(%d):failed with %#x!\n", __FUNCTION__,__LINE__,  s32Ret);
		return HI_FAILURE;
	}
	
	s32Ret = HI_MPI_VO_EnableChn(VoLayer, iChan);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
    return HI_SUCCESS;
}

HI_S32 StopVoChn(VO_LAYER VoLayer,int iChan)
{
	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;

	s32Ret = HI_MPI_VO_DisableChn(VoLayer, iChan);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
		return HI_FAILURE;
	}
    return s32Ret;
}

HI_S32 StartVdecChn(HI_S32 s32ChnId, VDEC_CHN_ATTR_S *pstAttr)
{
	HI_S32  i;
	HI_S32 s32Ret;		

    s32Ret = HI_MPI_VDEC_CreateChn(s32ChnId, pstAttr);
	if (s32Ret != HI_SUCCESS)
	{
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}   
  
	s32Ret = HI_MPI_VDEC_StartRecvStream(s32ChnId);
	if (s32Ret != HI_SUCCESS)
	{
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	} 

	s32Ret = HI_MPI_VDEC_SetDisplayMode(s32ChnId,VIDEO_DISPLAY_MODE_PREVIEW);
    if(HI_SUCCESS != s32Ret)
    {
		SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
    }
	DPRINTK("Start dec %d\n",s32ChnId);
    return HI_SUCCESS;
}

HI_S32 StopVdecChn(HI_S32 s32ChnId)
{
	HI_S32 i;	
	HI_S32 s32Ret;		

	s32Ret = HI_MPI_VDEC_StopRecvStream(s32ChnId);
	if (s32Ret != HI_SUCCESS)
	{
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}   

	s32Ret = HI_MPI_VDEC_DestroyChn(s32ChnId);
	if (s32Ret != HI_SUCCESS)
	{
	    SAMPLE_PRT("failed with %#x!\n", s32Ret);
	    return HI_FAILURE;
	}   

	DPRINTK("Stop dec %d\n",s32ChnId);
	return HI_SUCCESS;
}

HI_VOID	SetVdecChnAttr(VDEC_CHN_ATTR_S *pstVdecChnAttr, PAYLOAD_TYPE_E enType)
{
    HI_S32 i;
	SIZE_S stSize;    
	stSize.u32Width = 1920;
	stSize.u32Height = 1080;
	pstVdecChnAttr->enType       = enType;
	pstVdecChnAttr->u32BufSize   = stSize.u32Width * stSize.u32Height * 3/2;
	pstVdecChnAttr->u32Priority  = 5;
	pstVdecChnAttr->u32PicWidth  = stSize.u32Width;
	pstVdecChnAttr->u32PicHeight = stSize.u32Height;

	if (PT_H264 == enType || PT_MP4VIDEO == enType)
	{
	    pstVdecChnAttr->stVdecVideoAttr.enMode=VIDEO_MODE_FRAME;
	    pstVdecChnAttr->stVdecVideoAttr.u32RefFrameNum = 2;
	    pstVdecChnAttr->stVdecVideoAttr.s32SupportBFrame=0;
	}
	else if (PT_JPEG == enType || PT_MJPEG == enType)
	{
		pstVdecChnAttr->stVdecJpegAttr.enMode = VIDEO_MODE_FRAME;
	}
}

int SetEncode(int chan,VENC_RC_E enRc,int gop,int frame_rate,int bitstream)
{
	 VENC_CHN VencChn;
	 VENC_CHN_ATTR_S stVencChnAttr;
	 HI_S32 s32Ret;
	 int sub_stream_is_640x480 = 0;
	 int vbr_qulity = 0;
	 
	 
	 s32Ret = HI_MPI_VENC_GetChnAttr(chan, &stVencChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
                   chan, s32Ret);
            return -1;
        }		


	
	 if(enRc == VENC_RC_MODE_H264CBR)
	{				
       	 stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = gop;	 					stVencChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRate 	 = frame_rate;/* input (vi) frame rate */				 stVencChnAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate = frame_rate;/* target frame rate */
	 	 stVencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = bitstream;
	}else
	{
		int quality  = 0;		
		int new_bitstream = 0;
		//gop = frame_rate *2;
		stVencChnAttr.stRcAttr.stAttrH264Vbr.u32Gop = gop;
			stVencChnAttr.stRcAttr.stAttrH264Vbr.u32SrcFrmRate 	 = frame_rate;/* input (vi) frame rate */				 stVencChnAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate = frame_rate;/* target frame rate */		 
	  	stVencChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate = bitstream;

		//DPRINTK("chan=%d new_bitstream=%d  %d,%d\n",chan,stVencChnAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate,stVencChnAttr.stRcAttr.stAttrH264Vbr.u32MinQp,
		//	stVencChnAttr.stRcAttr.stAttrH264Vbr.u32MaxQp);	      
	}


	  s32Ret = HI_MPI_VENC_SetChnAttr(chan, &stVencChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VENC_SetChnAttr chn[%d] failed with %#x!\n", \
                   chan, s32Ret);
            return -1;
        }

	 DPRINTK("chan=%d gop=%d frame_rate=%d bitstream=%d\n",chan,gop,frame_rate, bitstream);

	  return 1;
}


int GetEncode(int chan,VENC_RC_E enRc,int * gop,int * frame_rate,int * bitstream)
{
    VENC_CHN VencChn;
    VENC_CHN_ATTR_S stVencChnAttr;
    HI_S32 s32Ret;
    int sub_stream_is_640x480 = 0;
    int vbr_qulity = 0;

    s32Ret = HI_MPI_VENC_GetChnAttr(chan, &stVencChnAttr);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
               chan, s32Ret);
        return -1;
    }

	if(enRc == VENC_RC_MODE_H264CBR)
	{				
       	 gop = stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop;
	 	 frame_rate = stVencChnAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate;
	 	 bitstream = stVencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate;
	}
	else
	{
		 gop = stVencChnAttr.stRcAttr.stAttrH264Cbr.u32Gop;
	 	 frame_rate = stVencChnAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate;
	 	 bitstream = stVencChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate;    
	}

	// DPRINTK("chan=%d gop=%d frame_rate=%d bitstream=%d\n",chan,gop,frame_rate, bitstream);
	return 1;
}



int GetOSDLayerInfo(int Handle,int * layer_width, int * layer_height,int * enPixelFmt)
{
	int iRet = 0;
	int iRetNo = MSA_SUCCESS;
	int tmp;
	RGN_CANVAS_INFO_S stCanvasInfo;
	iRet = HI_MPI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
	if(HI_SUCCESS != iRet)
	{
	    printf("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", iRet);
	    return -1;
	}

	iRet = HI_MPI_RGN_UpdateCanvas(Handle);
	if(HI_SUCCESS != iRet)
	{
	    printf("HI_MPI_RGN_UpdateCanvas failed! s32Ret: 0x%x.\n", iRet);
	     return -1;
	}

	*layer_width =  stCanvasInfo.stSize.u32Width;
	*layer_height = stCanvasInfo.stSize.u32Height;
	*enPixelFmt = stCanvasInfo.enPixelFmt;
	return 1;
}  int OSDLayerDrawClear(int Handle)  {  	int x,y;
	short * screen_buf;
	int iRet;  	RGN_CANVAS_INFO_S stCanvasInfo;
	iRet = HI_MPI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
	if(HI_SUCCESS != iRet)
	{
	    printf("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", iRet);
	    return -1;
	}		memset(stCanvasInfo.u32VirtAddr,FB_BK_COLOR,stCanvasInfo.u32Stride*stCanvasInfo.stSize.u32Height );	iRet = HI_MPI_RGN_UpdateCanvas(Handle);    	if (HI_SUCCESS != iRet)    	{        	DPRINTK("HI_MPI_RGN_UpdateCanvas fail! s32Ret: 0x%x.\n", iRet);        	return -1;    	}	return 1;  }

int OSDLayerDrawPicUseCoordinate(int Handle,int start_x,int start_y,int pic_x,int pic_y,int pic_show_w,int pic_show_h,int pic_width,int pic_height,char * pic_data)
{
	int x,y;
	short * screen_buf;
	int iRet;
	RGN_CANVAS_INFO_S stCanvasInfo;
	iRet = HI_MPI_RGN_GetCanvasInfo(Handle, &stCanvasInfo);
	if(HI_SUCCESS != iRet)
	{
	    printf("HI_MPI_RGN_GetCanvasInfo failed! s32Ret: 0x%x.\n", iRet);
	    return -1;
	}
	if( (start_x + pic_show_w)  > stCanvasInfo.stSize.u32Width  ||
		(start_y + pic_show_h) >  stCanvasInfo.stSize.u32Height  )
	{
		DPRINTK("[%d] screen(%d,%d,%d,%d) > (%d,%d) error!\n",Handle,pic_x,pic_y,
			pic_show_w, pic_show_h,stCanvasInfo.stSize.u32Width,stCanvasInfo.stSize.u32Height );
		return -1;
	}		//memset(stCanvasInfo.u32VirtAddr,FB_BK_COLOR,stCanvasInfo.u32Stride*stCanvasInfo.stSize.u32Height );
	{
		short * pic_buf;
		int pic_line = 0;
		screen_buf = (short *)stCanvasInfo.u32VirtAddr;
		pic_buf = (short *)pic_data;
		pic_line = pic_y;		//printf("first pixel is %x\n",pic_buf[0]);
		for( y = start_y; y < start_y + pic_show_h; y++ )
		{
			memcpy(&screen_buf[y* stCanvasInfo.u32Stride /2 + start_x],
				&pic_buf[pic_line*pic_width + pic_x],pic_show_w*2);
			pic_line++;
		}
	}
	//DPRINTK("HI_MPI_RGN_UpdateCanvas 1\n");	iRet = HI_MPI_RGN_UpdateCanvas(Handle);    	if (HI_SUCCESS != iRet)    	{        	DPRINTK("HI_MPI_RGN_UpdateCanvas fail! s32Ret: 0x%x.\n", iRet);        	return -1;    	}	//DPRINTK("HI_MPI_RGN_UpdateCanvas 2\n");
	return 1;

}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
