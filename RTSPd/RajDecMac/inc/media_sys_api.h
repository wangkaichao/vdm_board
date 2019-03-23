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
	int width;				   //视频宽
	int height;			   //视频高
	int datalen;			   //视频数据长度
	int iskeyframe;		  //是不是I 帧  iskeyframe=1 表示此帧为I 帧， iskeyframe = 0 表示此帧为非I帧。	
	struct timeval  timestamp; //时间戳
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
	MSA_BASE_LAYER,    //只能显示一个窗口
	MSA_PIP_LAYER		//可以显示多个窗口，但窗口不能叠加
}MSA_SHOW_LAYER;


typedef enum 
{
	MSA_WINDOW_SHOW,    //窗口显示
	MSA_WINDOW_HIDE,     //窗口隐藏
}MSA_WINDOW_SHOW_E;


typedef enum 
{
	MSA_HD0,    //高清输出0
	MSA_HD1,     //高清输出1
	MSA_SD0
}MSA_OUTPUT_DEV;



typedef enum 
{
	MSA_VIDEO_H264,
	MSA_PIC_JPEG
}MSA_INPUT_MEDIA;

typedef enum 
{
	MSA_HARDWARE_COMBINE_MODE,    //使用硬件方式合并分割图像
	MSA_SOFTWARE_COMBINE_MODE,     //使用软件方式合并分割图像
	MSA_COMBINE_MODE_BUF
}MSA_COMBINE_MODE;


typedef struct _MSA_OUTPUT_DEV_INFO_ST_
{
	int iOutputDevNum;  //使用几个输入设备，范围(1-2).
	MSA_VO_INTF_SYNC_E enOutputScreenMode[MSA_MAX_OUTPUT_DEV_NUM];
	int iDevIntfType[MSA_MAX_OUTPUT_DEV_NUM]; //输出信号, MSA_VO_INTF_VGA,MSA_VO_INTF_BT1120,MSA_VO_INTF_HDMI
	MSA_COMBINE_MODE iDevCombineMode[MSA_MAX_OUTPUT_DEV_NUM];
	int iPipLayerBindDevId;  // 画中画层绑定到指定输出设备上，范围(0 -1),超出范围将不绑定PIP层。
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
	//MSA_VPSS_CROP_RATIO_COOR 相对坐标s32X,s32Y 取值范围[0, 1000]  iWidth,iHeight 取值范围[0, 1000] 
	//MSA_VPSS_CROP_ABS_COOR 绝对坐标 对图像局部进行放缩，这里的坐标都是指的图像的实际大小值，不是通道的大小值。
	int iPicStartX;   //图像放缩的起始点x坐标 ，
	int iPicStartY;   //图像放缩的起始点y坐标 ，
	int iWidth;  //局部图像的宽.
	int iHeight; //局部图像的高。
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
	U64INT  u64PTS;		 //微妙为单位
}TIME_STAMP;


typedef struct _MSA_DEC_DATA_HEAD_ST_
{
	MSA_INPUT_MEDIA enDataType;
	int iDataLen;
	//h264 帧数据的时间戳。
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
	int iWidth;		/*宽*/
	int iHeight;		/*高*/
	int iBitrate;		/*码流*/
	int iFrameRate;      /*帧率*/
	int iGop;			/*I帧间隔*/
	VENC_RC_E enRc;	/*码流控制方式*/
	ENC_DATA_GET_CALLBACK  DataCallback;
}MSA_ENCODE_SET_ST;


typedef struct _DRAW_OSD_PIC_ST_
{	
	int screen_x;			/* osd 层贴图的起始坐标x*/
	int screen_y;			/* osd 层贴图的起始坐标y*/
	int pic_width;			/*图片的宽 */
	int pic_height;			/*图片的高 */
	int pic_show_offset_x;   /* 所贴图像在图片中的起始坐标x */
	int pic_show_offset_y;  /* 所贴图像在图片中的起始坐标y */
	int pic_show_w;		/* 所贴图像的宽*/
	int pic_show_h;		/* 所贴图像的高*/
	char * pic_data_ptr;      /*存储图像数据的指针*/
}MSA_DRAW_OSD_PIC_ST;


/************************************************************************
 函数名:MSA_InitSystem
 功能: 初始化系统
 输入: 
 		stInitDev  初始化参数
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MSA_InitSystem(F_IN MSA_INIT_DEV_ST stInitDev);


/************************************************************************
 函数名:MSA_DestroySystem
 功能: 关闭系统
 输入: 
 		无
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MSA_DestroySystem();


/************************************************************************
 函数名:MSA_CreateChanWindow
 功能: 创建通道窗口
 输入: 
 		stChan  通道参数
 输出:
 		hHandle  返回通道窗口句柄
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MSA_CreateChanWindow(F_IN MSA_CHANNEL_ST stChan,F_OUT MSA_HANDLE * hHandle);


/************************************************************************
 函数名:MSA_DestroyChanWindow
 功能: 创建通道窗口
 输入: 
 		hHandle  通道窗口句柄
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MSA_DestroyChanWindow(F_IN MSA_HANDLE  hHandle);



/************************************************************************
 函数名:MSA_ShowChanWindow
 功能: 让通道隐藏
 输入: 
 		hHandle  通道窗口句柄
 		enShow  控制窗口显示或隐藏参数，参考MSA_WINDOW_SHOW_E
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MSA_ShowChanWindow(F_IN MSA_HANDLE  hHandle,F_IN MSA_WINDOW_SHOW_E enShow);


/************************************************************************
 函数名:MAS_ChanWindowSet
 功能: 动态修改通道窗口大小
 输入: 
 		hHandle  通道窗口句柄
 		stWindow   窗口大小结构体
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MAS_ChanWindowSet(F_IN MSA_HANDLE hHandle, F_IN MSA_WINDOW_ST stWindow);




/************************************************************************
 函数名:MAS_ChanWindowOpenZoomIn
 功能: 对窗口图像进行局部放大
 输入: 
 		hHandle  通道窗口句柄
 		stZoomInWindow   窗口放缩结构体
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MAS_ChanWindowOpenZoomIn(F_IN MSA_HANDLE hHandle, F_IN MSA_WINDOW_ZOOM_IN_ST stZoomInWindow);


/************************************************************************
 函数名:MAS_ChanWindowCloseZoomIn
 功能: 关闭窗口局部放大
 输入: 
 		hHandle  通道窗口句柄 		
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MAS_ChanWindowCloseZoomIn(F_IN MSA_HANDLE hHandle);


/************************************************************************
 函数名:MAS_ChanWindowSwap
 功能: 动态窗口交换
 输入: 
 		hHandle1   交换 通道窗口句柄 	
 		hHandle2   交换 通道窗口句柄 	
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
 		
************************************************************************/
int MAS_ChanWindowSwap(F_IN MSA_HANDLE hHandle1,F_IN MSA_HANDLE hHandle2);


/************************************************************************
 函数名:MSA_SendDecData
 功能: 向窗口发送视频数据
 输入: 
 		hHandle          通道窗口句柄 	
 		stDataHead    视频数据结构体 	
 		pData	       视频数据指针
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MSA_SendDecData(F_IN MSA_HANDLE hHandle,F_IN MSA_DATA_HEAD_ST stDataHead,F_IN void * pData);

/************************************************************************
 函数名:MSA_SetDevCombineMode
 功能: 切换高清输出设备的拼图模式
 输入: 
 		iOutputDevNum         高清输出设备号 	
 		enMode   			拼图模式结构体 		
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
 	     必须将所有通道窗口关闭后，才能使用此函数。
************************************************************************/
int MSA_SetDevCombineMode(F_IN int iOutputDevNum,MSA_COMBINE_MODE enMode);


/************************************************************************
 函数名:MSA_PipLayerBindToDev
 功能: 将画中画层绑定到指定高清输出设备
 输入: 
 		iOutputDevNum         高清输出设备号 	 			
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
 	     必须将所有通道窗口关闭后，才能使用此函数。
************************************************************************/
int MSA_PipLayerBindToDev(F_IN int iOutputDevNum);


//OSD

/************************************************************************
 函数名:MSA_GuiOpenPicData
 功能:读取图像数据到指定的高清输出设备
 输入: 
 		data_name     图像数据完整路径名	
 		dev			高清输出设备              
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明: 	     
************************************************************************/
int MSA_GuiOpenPicData(char * data_name,MSA_OUTPUT_DEV dev);



/************************************************************************
 函数名:MSA_GuiDrawLine
 功能:  画线函数
 输入:  		
 		dev			高清输出设备  
 		start_x		 起始点 x 
 		start_y		起始点y
 		end_x		结束点x
 		end_y              结束点y
 		lineStyle          画线的粗细值范围(1-4)
 		color                画线的颜色，像素点格式为RGBA1555,最高位一定要为0
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明: 	     
 		目前这个函数只能画横线和竖线，不能画斜线
************************************************************************/
int MSA_GuiDrawLine(MSA_OUTPUT_DEV dev,int start_x,int start_y,int end_x,int end_y,int lineStyle,unsigned short color);


/************************************************************************
 函数名:MSA_GuiDrawPicUseCoordinate
 功能:  指定图片中的局部图像进行 贴图
 输入: 
 		name     图像名字
 		dev	      高清输出设备            
 		screen_x   高清设备上贴图起始点x
 		screen_y    高清设备上贴图起始点y
 		pic_x      局部图像在原图像中的起始点x
 		pic_y	局部图像在原图像中的起始点y
 		pic_w	局部图像的宽
 		pic_h	局部图像的高
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明: 	     
 		局部图像取图范围不能超出原图像范围。
************************************************************************/
int MSA_GuiDrawPicUseCoordinate(char * name,MSA_OUTPUT_DEV dev,int screen_x,int screen_y,
	int pic_x,int pic_y,int pic_w,int pic_h);



/************************************************************************
 函数名:MSA_GetPicDataByName
 功能:  用名字获取图片数据
 输入: 
 		name 图片名
 		layer  OSD层
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明: 使用此函数前需要调用 MSA_GuiOpenPicData 函数载入OSD 图片数据
************************************************************************/
GUI_DRAW_ITEM * MSA_GetPicDataByName(char * name,MSA_OUTPUT_DEV layer);


/************************************************************************
 函数名:MSA_ClearOsd
 功能: 清屏函数
 输入: 
 		x   高清设备上清屏区域起始点x
 		y   高清设备上清屏区域起始点y
 		w  高清设备上清屏区域起宽
 		h   高清设备上清屏区域起高
 		dev			高清输出设备              
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明: 	     
************************************************************************/
void MSA_ClearOsd(int x,int y,int w,int h,MSA_OUTPUT_DEV dev);


/************************************************************************
 函数名:MAS_ChanWindowCreateEncode
 功能:  创建窗口的压缩引擎
 输入: 
 		hHandle  通道窗口句柄
 		stEncodeSet   压缩引擎的配置结构体
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MAS_ChanWindowCreateEncode(F_IN MSA_HANDLE hHandle, F_IN MSA_ENCODE_SET_ST stEncodeSet);


/************************************************************************
 函数名:MAS_ChanWindowDestroyEncode
 功能: 销毁窗口的压缩引擎
 输入: 
 		hHandle  通道窗口句柄 		
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MAS_ChanWindowDestroyEncode(F_IN MSA_HANDLE hHandle);


/************************************************************************
 函数名:MAS_GetEncodeInfo
 功能: 获取压缩引擎的配置参数
 输入: 
 		hHandle  通道窗口句柄
 		pstEncodeSet    存放压缩引擎的配置结构体
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MAS_GetEncodeInfo(F_IN MSA_HANDLE hHandle, F_OUT MSA_ENCODE_SET_ST * pstEncodeSet);


/************************************************************************
 函数名:MAS_SetEncodeInfo
 功能: 设置压缩引擎的配置参数
 输入: 
 		hHandle  通道窗口句柄
 		stEncodeSet   压缩引擎的配置结构体
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MAS_SetEncodeInfo(F_IN MSA_HANDLE hHandle, F_IN MSA_ENCODE_SET_ST stEncodeSet);


/************************************************************************
 函数名:MAS_ChanWindowCreateOSDLayer
 功能: 创建窗口OSD层
 输入: 
 		hHandle  通道窗口句柄 		
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MAS_ChanWindowCreateOSDLayer(F_IN MSA_HANDLE hHandle);


/************************************************************************
 函数名:MAS_ChanWindowDestroyOSDLayer
 功能: 销毁窗口OSD 层
 输入: 
 		hHandle  通道窗口句柄 		
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MAS_ChanWindowDestroyOSDLayer(F_IN MSA_HANDLE hHandle);


/************************************************************************
 函数名:MAS_GetOSDLayerInfo
 功能: 获取窗口OSD 参数
 输入: 
 		hHandle  通道窗口句柄
 		pstOsdLayerInfo   存放OSD 参数信息
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/
int MAS_GetOSDLayerInfo(F_IN MSA_HANDLE hHandle, F_OUT MSA_OSD_LAYER_INFO * pstOsdLayerInfo);



/************************************************************************
 函数名:MSA_OSDLayerDrawPicUseCoordinate
 功能:  指定图片中的局部图像进行 贴图
 输入: 
 		name     图像名字
 		dev	      高清输出设备            
 		screen_x   高清设备上贴图起始点x
 		screen_y    高清设备上贴图起始点y
 		pic_x      局部图像在原图像中的起始点x
 		pic_y	局部图像在原图像中的起始点y
 		pic_w	局部图像的宽
 		pic_h	局部图像的高
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明: 	     
 		局部图像取图范围不能超出原图像范围。
************************************************************************/
int MSA_OSDLayerDrawPic(F_IN MSA_HANDLE hHandle, MSA_DRAW_OSD_PIC_ST stDrawOsd);


/************************************************************************
 函数名:MSA_OSDLayerDrawClear
 功能:  窗口OSD 清屏函数
 输入: 
 		hHandle  通道窗口句柄
 		
 输出:
 		无
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/

int MSA_OSDLayerDrawClear(F_IN MSA_HANDLE hHandle);


/************************************************************************
 函数名:MSA_GetDecodePicSize
 功能:  获取解码视频的宽高
 输入: 
 		hHandle  通道窗口句柄
 		
 输出:
 		PicWidth  解码视频的宽
 		PicHeight 解码视频的高
 返回:成功返回RJONE_SUCCESS 	     
 说明:
************************************************************************/

int MSA_GetDecodePicSize(F_IN MSA_HANDLE hHandle,F_OUT int * PicWidth,F_OUT int *  PicHeight);



/**********************************************************************
函数的具体使用方法，请参考sample
**********************************************************************/

#ifdef __cplusplus
}
#endif

#endif 

