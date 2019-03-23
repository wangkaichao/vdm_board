#ifndef _MDDIA_SYS_API_H_
#define _MDDIA_SYS_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "osd.h"



#define MSA_SYS_VER  (0x01000001)

#define MSA_SUCCESS   (0)
#define MSA_FAILED      (-1)

#define MSA_MAX_OUTPUT_DEV_NUM (2)

typedef unsigned long long  U64INT;

typedef void *	 MSA_HANDLE;

#ifndef F_OUT
#define F_OUT
#endif

#ifndef F_IN
#define F_IN
#endif


#define MSA_ERR_ALREADY_INIT 								(-10000)
#define MSA_ERR_NOT_INIT 									(-10001)
#define MSA_ERR_MEM_ALLOC		       							(-10002)
#define MSA_ERR_ALREADY_CREATE_MAX							(-10003)
#define MSA_ERR_NULL_PTR									(-10004)
#define MSA_ERR_UNKNOW_INPUT_MEDIA						(-10005)
#define MSA_ERR_DEC_ERR										(-10006)
#define MSA_ERR_INPUT_PARAMETERS_ERR	 					(-10007)
#define MSA_ERR_CHAN_WINDOW_NOT_ALL_CLOSE_ERR	 		(-10008)
#define MSA_ERR_SWAP_HD_COMBINEMODE_NOT_SAME_ERR	 	(-10009)
#define MSA_ERR_CHAN_WINDOW_CREATE_PARAMETERS_ERR	 	(-10010)
#define MSA_ERR_CHAN_WINDOW_NOT_SUPPORT_ERR	 		(-10011)
#define MSA_ERR_DEV_IS_ALREADY_CREATE_ERR	 				(-10012)
#define MSA_ERR_DEV_IS_NOT_CREATE_ERR	 					(-10013)
#define MSA_ERR_ENC_OR_CHAN_OSD_IS_NOT_CLOSE_ERR	 	(-10014)
#define MSA_ERR_SOFTWARE_COMBINE_MODE_NOT_SUPPORT	 	(-10015)
#define MSA_ERR_VO_IS_HIDE								 	(-10016)




#define MSA_VO_INTF_VGA      (0x01L<<2)
#define MSA_VO_INTF_BT1120   (0x01L<<4)
#define MSA_VO_INTF_HDMI     (0x01L<<5)


#define MAX_VIDEO_BUF_SIZE	(1*1024*1024) /* (512*1024)*/


typedef struct _H264VIDEO_HEADER_
{	
	int width;				   //��Ƶ��
	int height;			   //��Ƶ��
	int datalen;			   //��Ƶ���ݳ���
	int iskeyframe;		  //�ǲ���I ֡  iskeyframe=1 ��ʾ��֡ΪI ֡�� iskeyframe = 0 ��ʾ��֡Ϊ��I֡��	
	struct timeval  timestamp; //ʱ���
}MSA_H264VIDEO_HEADER;


typedef enum MSA_hiPIXEL_FORMAT_E
{   
    MSA_PIXEL_FORMAT_RGB_1BPP = 0,
    MSA_PIXEL_FORMAT_RGB_2BPP,
    MSA_PIXEL_FORMAT_RGB_4BPP,
    MSA_PIXEL_FORMAT_RGB_8BPP,
    MSA_PIXEL_FORMAT_RGB_444,
    MSA_PIXEL_FORMAT_RGB_4444,
    MSA_PIXEL_FORMAT_RGB_555,
    MSA_PIXEL_FORMAT_RGB_565,
    MSA_PIXEL_FORMAT_RGB_1555,

    /*  9 reserved */
    MSA_PIXEL_FORMAT_RGB_888,
    MSA_PIXEL_FORMAT_RGB_8888,
    MSA_PIXEL_FORMAT_RGB_PLANAR_888,
    MSA_PIXEL_FORMAT_RGB_BAYER_8BPP,
    MSA_PIXEL_FORMAT_RGB_BAYER_10BPP,
    MSA_PIXEL_FORMAT_RGB_BAYER_12BPP,
    MSA_PIXEL_FORMAT_RGB_BAYER_14BPP,
    MSA_PIXEL_FORMAT_RGB_BAYER,         /* 16 bpp */

    MSA_PIXEL_FORMAT_YUV_A422,
    MSA_PIXEL_FORMAT_YUV_A444,

    MSA_PIXEL_FORMAT_YUV_PLANAR_422,
    MSA_PIXEL_FORMAT_YUV_PLANAR_420,
    MSA_PIXEL_FORMAT_YUV_PLANAR_444,

    MSA_PIXEL_FORMAT_YUV_SEMIPLANAR_422,
    MSA_PIXEL_FORMAT_YUV_SEMIPLANAR_420,
    MSA_PIXEL_FORMAT_YUV_SEMIPLANAR_444,

    MSA_PIXEL_FORMAT_UYVY_PACKAGE_422,
    MSA_PIXEL_FORMAT_YUYV_PACKAGE_422,
    MSA_PIXEL_FORMAT_VYUY_PACKAGE_422,
    MSA_PIXEL_FORMAT_YCbCr_PLANAR,
    
    MSA_PIXEL_FORMAT_BUTT   
} MSA_PIXEL_FORMAT_E;


typedef struct _MSA_OSD_LAYER_INFO_
{
    int 			layer_width;      
    int          		layer_height;   
    MSA_PIXEL_FORMAT_E  enPixelFmt;  
}MSA_OSD_LAYER_INFO;



typedef enum MSA_hiVO_INTF_SYNC_E
{

    MSA_VO_OUTPUT_PAL = 0,

    MSA_VO_OUTPUT_NTSC,

    

    MSA_VO_OUTPUT_1080P24,

    MSA_VO_OUTPUT_1080P25,

    MSA_VO_OUTPUT_1080P30,

    

    MSA_VO_OUTPUT_720P50, 

    MSA_VO_OUTPUT_720P60,   

    MSA_VO_OUTPUT_1080I50,

    MSA_VO_OUTPUT_1080I60,    

    MSA_VO_OUTPUT_1080P50,

    MSA_VO_OUTPUT_1080P60,            



    MSA_VO_OUTPUT_576P50,

    MSA_VO_OUTPUT_480P60,



    MSA_VO_OUTPUT_800x600_60,            /* VESA 800 x 600 at 60 Hz (non-interlaced) */

    MSA_VO_OUTPUT_1024x768_60,           /* VESA 1024 x 768 at 60 Hz (non-interlaced) */

    MSA_VO_OUTPUT_1280x1024_60,          /* VESA 1280 x 1024 at 60 Hz (non-interlaced) */

    MSA_VO_OUTPUT_1366x768_60,           /* VESA 1366 x 768 at 60 Hz (non-interlaced) */

    MSA_VO_OUTPUT_1440x900_60,           /* VESA 1440 x 900 at 60 Hz (non-interlaced) CVT Compliant */

    MSA_VO_OUTPUT_1280x800_60,           /* 1280*800@60Hz VGA@60Hz*/

    MSA_VO_OUTPUT_1600x1200_60,          /* VESA 1600 x 1200 at 60 Hz (non-interlaced) */

    MSA_VO_OUTPUT_1680x1050_60,          /* VESA 1680 x 1050 at 60 Hz (non-interlaced) */

    MSA_VO_OUTPUT_1920x1200_60,          /* VESA 1920 x 1600 at 60 Hz (non-interlaced) CVT (Reduced Blanking)*/

    MSA_VO_OUTPUT_640x480_60,            /* VESA 640 x 480 at 60 Hz (non-interlaced) CVT */

    MSA_VO_OUTPUT_1920x2160_30,          /* 1920x2160_30 */

    MSA_VO_OUTPUT_USER,

    MSA_VO_OUTPUT_BUTT

} MSA_VO_INTF_SYNC_E;

typedef enum 
{
	MSA_BASE_LAYER,    //ֻ����ʾһ������
	MSA_PIP_LAYER		//������ʾ������ڣ������ڲ��ܵ���
}MSA_SHOW_LAYER;


typedef enum 
{
	MSA_WINDOW_SHOW,    //������ʾ
	MSA_WINDOW_HIDE,     //��������
}MSA_WINDOW_SHOW_E;


typedef enum 
{
	MSA_HD0,    //�������0
	MSA_HD1,    //�������1
	MSA_SD0     //�������0
}MSA_OUTPUT_DEV;



typedef enum 
{
	MSA_VIDEO_H264,
	MSA_PIC_JPEG
}MSA_INPUT_MEDIA;

typedef enum 
{
	MSA_HARDWARE_COMBINE_MODE,    //ʹ��Ӳ����ʽ�ϲ��ָ�ͼ��
	MSA_SOFTWARE_COMBINE_MODE,     //ʹ�������ʽ�ϲ��ָ�ͼ��
	MSA_COMBINE_MODE_BUF
}MSA_COMBINE_MODE;


typedef struct _MSA_OUTPUT_DEV_INFO_ST_
{
	int iOutputDevNum;  //ʹ�ü��������豸����Χ(1-2).
	MSA_VO_INTF_SYNC_E enOutputScreenMode[MSA_MAX_OUTPUT_DEV_NUM];
	int iDevIntfType[MSA_MAX_OUTPUT_DEV_NUM]; //����ź�, MSA_VO_INTF_VGA,MSA_VO_INTF_BT1120,MSA_VO_INTF_HDMI
	MSA_COMBINE_MODE iDevCombineMode[MSA_MAX_OUTPUT_DEV_NUM];
	int iPipLayerBindDevId;  // ���л���󶨵�ָ������豸�ϣ���Χ(0 -1),������Χ������PIP�㡣
}MSA_OUTPUT_DEV_INFO_ST;



typedef struct _MSA_INIT_DEV_ST_
{	
	MSA_OUTPUT_DEV_INFO_ST stOutputDevSetParameters;
}MSA_INIT_DEV_ST;



typedef struct _MSA_IPCAM_ST_
{
	char strIp[20];
	char strUserName[40];
	char strUserPass[40];
}MSA_IPCAM_ST;

typedef struct _MSA_WINDOW_ST_
{
	int iPosStartX;
	int iPosStartY;
	int iWidth;
	int iHeight;
}MSA_WINDOW_ST;


typedef enum   _MSA_VPSS_CROP_COORDINATE_
{
    MSA_VPSS_CROP_RATIO_COOR = 0,   /*Ratio coordinate*/
    MSA_VPSS_CROP_ABS_COOR          /*Absolute coordinate*/
}MSA_VPSS_CROP_COORDINATE;

typedef struct _MSA_WINDOW_ZOOM_IN_ST_
{
	MSA_VPSS_CROP_COORDINATE enCropCoordinate;
	//MSA_VPSS_CROP_RATIO_COOR �������s32X,s32Y ȡֵ��Χ[0, 1000]  iWidth,iHeight ȡֵ��Χ[0, 1000] 
	//MSA_VPSS_CROP_ABS_COOR �������� ��ͼ��ֲ����з�������������궼��ָ��ͼ���ʵ�ʴ�Сֵ������ͨ���Ĵ�Сֵ��
	int iPicStartX;   //ͼ���������ʼ��x���� ��
	int iPicStartY;   //ͼ���������ʼ��y���� ��
	int iWidth;  //�ֲ�ͼ��Ŀ�.
	int iHeight; //�ֲ�ͼ��ĸߡ�
}MSA_WINDOW_ZOOM_IN_ST;

typedef struct _MSA_CHANNEL_ST_
{
	MSA_OUTPUT_DEV enOutputDev;
	MSA_SHOW_LAYER enShowLayer;  
	//MSA_INPUT_MEDIA enInputMedia;
	//MSA_IPCAM_ST stIpcamInfo;
	MSA_WINDOW_ST stWindow;
}MSA_CHANNEL_ST;


typedef struct _U64_TIME_STAMP_
{		
	U64INT  u64PTS;		 //΢��Ϊ��λ
}TIME_STAMP;


typedef struct _MSA_DEC_DATA_HEAD_ST_
{
	MSA_INPUT_MEDIA enDataType;
	int iDataLen;
	//h264 ֡���ݵ�ʱ�����
	TIME_STAMP TimeStamp;
}MSA_DATA_HEAD_ST;


typedef enum venc_rc_e
{
    VENC_RC_CBR = 0,
    VENC_RC_VBR
}VENC_RC_E;


typedef int (*ENC_DATA_GET_CALLBACK)(MSA_HANDLE  hHandle,char * pVencBuf,MSA_H264VIDEO_HEADER stHeader);

typedef struct _MSA_ENCODE_SET_ST_
{
	int iWidth;		/*��*/
	int iHeight;		/*��*/
	int iBitrate;		/*����*/
	int iFrameRate;      /*֡��*/
	int iGop;			/*I֡���*/
	VENC_RC_E enRc;	/*�������Ʒ�ʽ*/
	ENC_DATA_GET_CALLBACK  DataCallback;
}MSA_ENCODE_SET_ST;


typedef struct _DRAW_OSD_PIC_ST_
{	
	int screen_x;			/* osd ����ͼ����ʼ����x*/
	int screen_y;			/* osd ����ͼ����ʼ����y*/
	int pic_width;			/*ͼƬ�Ŀ� */
	int pic_height;			/*ͼƬ�ĸ� */
	int pic_show_offset_x;   /* ����ͼ����ͼƬ�е���ʼ����x */
	int pic_show_offset_y;  /* ����ͼ����ͼƬ�е���ʼ����y */
	int pic_show_w;		/* ����ͼ��Ŀ�*/
	int pic_show_h;		/* ����ͼ��ĸ�*/
	char * pic_data_ptr;      /*�洢ͼ�����ݵ�ָ��*/
}MSA_DRAW_OSD_PIC_ST;


/************************************************************************
 ������:MSA_InitSystem
 ����: ��ʼ��ϵͳ
 ����: 
 		stInitDev  ��ʼ������
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MSA_InitSystem(F_IN MSA_INIT_DEV_ST stInitDev);


/************************************************************************
 ������:MSA_DestroySystem
 ����: �ر�ϵͳ
 ����: 
 		��
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MSA_DestroySystem();


/************************************************************************
 ������:MSA_CreateChanWindow
 ����: ����ͨ������
 ����: 
 		stChan  ͨ������
 ���:
 		hHandle  ����ͨ�����ھ��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MSA_CreateChanWindow(F_IN MSA_CHANNEL_ST stChan,F_OUT MSA_HANDLE * hHandle);


/************************************************************************
 ������:MSA_DestroyChanWindow
 ����: ����ͨ������
 ����: 
 		hHandle  ͨ�����ھ��
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MSA_DestroyChanWindow(F_IN MSA_HANDLE  hHandle);



/************************************************************************
 ������:MSA_ShowChanWindow
 ����: ��ͨ������
 ����: 
 		hHandle  ͨ�����ھ��
 		enShow  ���ƴ�����ʾ�����ز������ο�MSA_WINDOW_SHOW_E
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MSA_ShowChanWindow(F_IN MSA_HANDLE  hHandle,F_IN MSA_WINDOW_SHOW_E enShow);


/************************************************************************
 ������:MAS_ChanWindowSet
 ����: ��̬�޸�ͨ�����ڴ�С
 ����: 
 		hHandle  ͨ�����ھ��
 		stWindow   ���ڴ�С�ṹ��
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MAS_ChanWindowSet(F_IN MSA_HANDLE hHandle, F_IN MSA_WINDOW_ST stWindow);




/************************************************************************
 ������:MAS_ChanWindowOpenZoomIn
 ����: �Դ���ͼ����оֲ��Ŵ�
 ����: 
 		hHandle  ͨ�����ھ��
 		stZoomInWindow   ���ڷ����ṹ��
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MAS_ChanWindowOpenZoomIn(F_IN MSA_HANDLE hHandle, F_IN MSA_WINDOW_ZOOM_IN_ST stZoomInWindow);


/************************************************************************
 ������:MAS_ChanWindowCloseZoomIn
 ����: �رմ��ھֲ��Ŵ�
 ����: 
 		hHandle  ͨ�����ھ�� 		
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MAS_ChanWindowCloseZoomIn(F_IN MSA_HANDLE hHandle);


/************************************************************************
 ������:MAS_ChanWindowSwap
 ����: ��̬���ڽ���
 ����: 
 		hHandle1   ���� ͨ�����ھ�� 	
 		hHandle2   ���� ͨ�����ھ�� 	
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
 		
************************************************************************/
int MAS_ChanWindowSwap(F_IN MSA_HANDLE hHandle1,F_IN MSA_HANDLE hHandle2);


/************************************************************************
 ������:MSA_SendDecData
 ����: �򴰿ڷ�����Ƶ����
 ����: 
 		hHandle          ͨ�����ھ�� 	
 		stDataHead    ��Ƶ���ݽṹ�� 	
 		pData	       ��Ƶ����ָ��
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MSA_SendDecData(F_IN MSA_HANDLE hHandle,F_IN MSA_DATA_HEAD_ST stDataHead,F_IN void * pData);

/************************************************************************
 ������:MSA_SetDevCombineMode
 ����: �л���������豸��ƴͼģʽ
 ����: 
 		iOutputDevNum         ��������豸�� 	
 		enMode   			ƴͼģʽ�ṹ�� 		
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
 	     ���뽫����ͨ�����ڹرպ󣬲���ʹ�ô˺�����
************************************************************************/
int MSA_SetDevCombineMode(F_IN int iOutputDevNum,MSA_COMBINE_MODE enMode);


/************************************************************************
 ������:MSA_PipLayerBindToDev
 ����: �����л���󶨵�ָ����������豸
 ����: 
 		iOutputDevNum         ��������豸�� 	 			
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
 	     ���뽫����ͨ�����ڹرպ󣬲���ʹ�ô˺�����
************************************************************************/
int MSA_PipLayerBindToDev(F_IN int iOutputDevNum);


//OSD

/************************************************************************
 ������:MSA_GuiOpenPicData
 ����:��ȡͼ�����ݵ�ָ���ĸ�������豸
 ����: 
 		data_name     ͼ����������·����	
 		dev			��������豸              
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��: 	     
************************************************************************/
int MSA_GuiOpenPicData(char * data_name,MSA_OUTPUT_DEV dev);



/************************************************************************
 ������:MSA_GuiDrawLine
 ����:  ���ߺ���
 ����:  		
 		dev			��������豸  
 		start_x		 ��ʼ�� x 
 		start_y		��ʼ��y
 		end_x		������x
 		end_y              ������y
 		lineStyle          ���ߵĴ�ϸֵ��Χ(1-4)
 		color                ���ߵ���ɫ�����ص��ʽΪRGBA1555,���λһ��ҪΪ0
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��: 	     
 		Ŀǰ�������ֻ�ܻ����ߺ����ߣ����ܻ�б��
************************************************************************/
int MSA_GuiDrawLine(MSA_OUTPUT_DEV dev,int start_x,int start_y,int end_x,int end_y,int lineStyle,unsigned short color);


/************************************************************************
 ������:MSA_GuiDrawPicUseCoordinate
 ����:  ָ��ͼƬ�еľֲ�ͼ����� ��ͼ
 ����: 
 		name     ͼ������
 		dev	      ��������豸            
 		screen_x   �����豸����ͼ��ʼ��x
 		screen_y    �����豸����ͼ��ʼ��y
 		pic_x      �ֲ�ͼ����ԭͼ���е���ʼ��x
 		pic_y	�ֲ�ͼ����ԭͼ���е���ʼ��y
 		pic_w	�ֲ�ͼ��Ŀ�
 		pic_h	�ֲ�ͼ��ĸ�
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��: 	     
 		�ֲ�ͼ��ȡͼ��Χ���ܳ���ԭͼ��Χ��
************************************************************************/
int MSA_GuiDrawPicUseCoordinate(char * name,MSA_OUTPUT_DEV dev,int screen_x,int screen_y,
	int pic_x,int pic_y,int pic_w,int pic_h);



/************************************************************************
 ������:MSA_GetPicDataByName
 ����:  �����ֻ�ȡͼƬ����
 ����: 
 		name ͼƬ��
 		layer  OSD��
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��: ʹ�ô˺���ǰ��Ҫ���� MSA_GuiOpenPicData ��������OSD ͼƬ����
************************************************************************/
GUI_DRAW_ITEM * MSA_GetPicDataByName(char * name,MSA_OUTPUT_DEV layer);


/************************************************************************
 ������:MSA_ClearOsd
 ����: ��������
 ����: 
 		x   �����豸������������ʼ��x
 		y   �����豸������������ʼ��y
 		w  �����豸�������������
 		h   �����豸�������������
 		dev			��������豸              
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��: 	     
************************************************************************/
void MSA_ClearOsd(int x,int y,int w,int h,MSA_OUTPUT_DEV dev);


/************************************************************************
 ������:MAS_ChanWindowCreateEncode
 ����:  �������ڵ�ѹ������
 ����: 
 		hHandle  ͨ�����ھ��
 		stEncodeSet   ѹ����������ýṹ��
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MAS_ChanWindowCreateEncode(F_IN MSA_HANDLE hHandle, F_IN MSA_ENCODE_SET_ST stEncodeSet);


/************************************************************************
 ������:MAS_ChanWindowDestroyEncode
 ����: ���ٴ��ڵ�ѹ������
 ����: 
 		hHandle  ͨ�����ھ�� 		
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MAS_ChanWindowDestroyEncode(F_IN MSA_HANDLE hHandle);


/************************************************************************
 ������:MAS_GetEncodeInfo
 ����: ��ȡѹ����������ò���
 ����: 
 		hHandle  ͨ�����ھ��
 		pstEncodeSet    ���ѹ����������ýṹ��
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MAS_GetEncodeInfo(F_IN MSA_HANDLE hHandle, F_OUT MSA_ENCODE_SET_ST * pstEncodeSet);


/************************************************************************
 ������:MAS_SetEncodeInfo
 ����: ����ѹ����������ò���
 ����: 
 		hHandle  ͨ�����ھ��
 		stEncodeSet   ѹ����������ýṹ��
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MAS_SetEncodeInfo(F_IN MSA_HANDLE hHandle, F_IN MSA_ENCODE_SET_ST stEncodeSet);


/************************************************************************
 ������:MAS_ChanWindowCreateOSDLayer
 ����: ��������OSD��
 ����: 
 		hHandle  ͨ�����ھ�� 		
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MAS_ChanWindowCreateOSDLayer(F_IN MSA_HANDLE hHandle);


/************************************************************************
 ������:MAS_ChanWindowDestroyOSDLayer
 ����: ���ٴ���OSD ��
 ����: 
 		hHandle  ͨ�����ھ�� 		
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MAS_ChanWindowDestroyOSDLayer(F_IN MSA_HANDLE hHandle);


/************************************************************************
 ������:MAS_GetOSDLayerInfo
 ����: ��ȡ����OSD ����
 ����: 
 		hHandle  ͨ�����ھ��
 		pstOsdLayerInfo   ���OSD ������Ϣ
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/
int MAS_GetOSDLayerInfo(F_IN MSA_HANDLE hHandle, F_OUT MSA_OSD_LAYER_INFO * pstOsdLayerInfo);



/************************************************************************
 ������:MSA_OSDLayerDrawPicUseCoordinate
 ����:  ָ��ͼƬ�еľֲ�ͼ����� ��ͼ
 ����: 
 		name     ͼ������
 		dev	      ��������豸            
 		screen_x   �����豸����ͼ��ʼ��x
 		screen_y    �����豸����ͼ��ʼ��y
 		pic_x      �ֲ�ͼ����ԭͼ���е���ʼ��x
 		pic_y	�ֲ�ͼ����ԭͼ���е���ʼ��y
 		pic_w	�ֲ�ͼ��Ŀ�
 		pic_h	�ֲ�ͼ��ĸ�
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��: 	     
 		�ֲ�ͼ��ȡͼ��Χ���ܳ���ԭͼ��Χ��
************************************************************************/
int MSA_OSDLayerDrawPic(F_IN MSA_HANDLE hHandle, MSA_DRAW_OSD_PIC_ST stDrawOsd);


/************************************************************************
 ������:MSA_OSDLayerDrawClear
 ����:  ����OSD ��������
 ����: 
 		hHandle  ͨ�����ھ��
 		
 ���:
 		��
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/

int MSA_OSDLayerDrawClear(F_IN MSA_HANDLE hHandle);


/************************************************************************
 ������:MSA_GetDecodePicSize
 ����:  ��ȡ������Ƶ�Ŀ��
 ����: 
 		hHandle  ͨ�����ھ��
 		
 ���:
 		PicWidth  ������Ƶ�Ŀ�
 		PicHeight ������Ƶ�ĸ�
 ����:�ɹ�����RJONE_SUCCESS 	     
 ˵��:
************************************************************************/

int MSA_GetDecodePicSize(F_IN MSA_HANDLE hHandle,F_OUT int * PicWidth,F_OUT int *  PicHeight);



/**********************************************************************
�����ľ���ʹ�÷�������ο�sample
**********************************************************************/

#ifdef __cplusplus
}
#endif

#endif 

