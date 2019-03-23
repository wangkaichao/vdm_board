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

#include "hi_common.h"
#include "hi_type.h"
#include "hi_comm_vb.h"
#include "hi_comm_sys.h"
#include "hi_comm_venc.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_region.h"
#include "mpi_vb.h"
#include "mpi_sys.h"
#include "mpi_venc.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_region.h"
#include "hi_tde_type.h"

#include <linux/fb.h>
#include "hifb.h"


	//#define DEBUG
#include "msa_debug.h"
#include "media_sys_api.h"
#include "osd.h"


#define HD_FB_NUM (3)


typedef struct _GUI_ITEM_
{
	int fd;
	int layer;
	int already_open;
	int screen_stride;
	int screen_width;
	int screen_height;
	int fb_men_len;
	int pic_data_length;
	void * mem_buf;
	void * pic_buf;	
	struct fb_fix_screeninfo fixinfo;
	struct fb_var_screeninfo varinfo;
}GUI_ITEM;


GUI_ITEM hd_layer[HD_FB_NUM];
int hd_layer_open[HD_FB_NUM] = {0};
static struct fb_bitfield g_r16 = {10, 5, 0};
static struct fb_bitfield g_g16 = {5, 5, 0};
static struct fb_bitfield g_b16 = {0, 5, 0};
static struct fb_bitfield g_a16 = {15, 1, 0};



int dev_fb_init(GUI_ITEM * pLayer,int alpha,int iScreenWidth,int iScreenHeight)
{
	HI_S32 i;
	struct fb_fix_screeninfo fixinfo;
	struct fb_var_screeninfo varinfo;
	struct fb_fix_screeninfo *fix=NULL;
	struct fb_var_screeninfo *var=NULL;
	unsigned char *pShowScreen;
	HIFB_ALPHA_S stAlpha;
	HIFB_POINT_S stPoint = {0, 0};
	char file[12] = "/dev/fb0";
	HI_BOOL g_bCompress = HI_FALSE;
	HI_U8 *pDst = NULL;
	HI_BOOL bShow; 
	HIFB_COLORKEY_S stColorKey;
	HIFB_LAYER_INFO_S stLayerInfo = {0}; 
	GUI_ITEM * pstInfo = NULL;

	if(HI_NULL == pLayer)
		goto err;

	pstInfo = pLayer;

	fix=&pstInfo->fixinfo;
	var=&pstInfo->varinfo;
	switch (pstInfo->layer)
	{
	case 0 :
		strcpy(file, "/dev/fb0");
		break;
	case 1 :
		strcpy(file, "/dev/fb1");
		break;
	case 2 :
		strcpy(file, "/dev/fb2");
		break;
	default:
		strcpy(file, "/dev/fb0");
		break;
	}
	/* 1. open framebuffer device overlay 0 */
	pstInfo->fd = open(file, O_RDWR, 0);
	if(pstInfo->fd < 0)
	{
		DPRINTK("open %s failed!\n",file);
		goto err;
	} 

	/*
	if(pstInfo->layer==HIFB_LAYER_0)
	{
	if (ioctl(pstInfo->fd, FBIOPUT_COMPRESSION_HIFB, &g_bCompress) < 0)
	{
	DPRINTK("Func:%s line:%d FBIOPUT_COMPRESSION_HIFB failed!\n",__FUNCTION__, __LINE__);
	close(pstInfo->fd);
	goto err;
	}
	}
	*/

	bShow = HI_FALSE;
	if (ioctl(pstInfo->fd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
	{
		DPRINTK("FBIOPUT_SHOW_HIFB failed!\n");
		goto err;
	}


	if (ioctl(pstInfo->fd, FBIOPUT_SCREEN_ORIGIN_HIFB, &stPoint) < 0)
	{
		DPRINTK("set screen original show position failed!\n");
		close(pstInfo->fd);
		goto err;
	}
	/* 3.set alpha */
	stAlpha.bAlphaEnable = HI_TRUE;
	stAlpha.bAlphaChannel = HI_TRUE;
	stAlpha.u8Alpha0 = 0xff;
	stAlpha.u8Alpha1 = 0x00;
	stAlpha.u8GlobalAlpha = 0xff - alpha*38;
	if (ioctl(pstInfo->fd, FBIOPUT_ALPHA_HIFB,  &stAlpha) < 0)
	{
		DPRINTK("Set alpha failed!\n");
		close(pstInfo->fd);
		goto err;
	}

	stColorKey.bKeyEnable = HI_TRUE;
	stColorKey.u32Key = FB_BK_COLOR;
	if (ioctl(pstInfo->fd, FBIOPUT_COLORKEY_HIFB, &stColorKey) < 0)
	{
		DPRINTK("FBIOPUT_COLORKEY_HIFB!\n");
		close(pstInfo->fd);
		goto err;
	}	

	/* 4. get the variable screen info */
	if (ioctl(pstInfo->fd, FBIOGET_VSCREENINFO, var) < 0)
	{
		DPRINTK("Get variable screen info failed!\n");
		close(pstInfo->fd);
		goto err;
	}


	var->xres_virtual = iScreenWidth;//32;
	var->yres_virtual = iScreenHeight;//32;
	var->xres = iScreenWidth;//32;
	var->yres = iScreenHeight;//32;


	var->transp= g_a16;
	var->red = g_r16;
	var->green = g_g16;
	var->blue = g_b16;
	var->bits_per_pixel = 16;
	var->activate = FB_ACTIVATE_NOW;  
	/* 6. set the variable screeninfo */
	if (ioctl(pstInfo->fd, FBIOPUT_VSCREENINFO, var) < 0)
	{
		DPRINTK("Put variable screen info failed!\n");
		close(pstInfo->fd);
		goto err;
	}

	DPRINTK("%d-%d  %d-%d\n",var->xres_virtual ,var->yres_virtual 
		,var->xres,var->yres);
	
	/* 7. get the fix screen info */
	if (ioctl(pstInfo->fd, FBIOGET_FSCREENINFO, fix) < 0)
	{
		DPRINTK("Get fix screen info failed!\n");
		close(pstInfo->fd);
		goto err;
	}

	/* 8. map the physical video memory for user use */
	pShowScreen = mmap(HI_NULL, fix->smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, pstInfo->fd, 0);
	if(MAP_FAILED == pShowScreen)
	{
		DPRINTK("mmap framebuffer failed!\n");
		close(pstInfo->fd);
		goto err;
	}
	memset(pShowScreen, FB_BK_COLOR, fix->smem_len/2);
	// 	memset(pShowScreen, 0x00, fix->smem_len);
	/* time to paly*/
	
	bShow = HI_TRUE;
	if (ioctl(pstInfo->fd, FBIOPUT_SHOW_HIFB, &bShow) < 0)
	{
		DPRINTK("FBIOPUT_SHOW_HIFB failed!\n");
		munmap(pShowScreen, fix->smem_len);
		goto err;
	}

	
	var->xoffset = 0;
	var->yoffset = 0;
	if (ioctl(pstInfo->fd, FBIOPAN_DISPLAY, var) < 0)
	{
		printf("FBIOPAN_DISPLAY failed!\n");
		munmap(pShowScreen, fix->smem_len);
		close(pstInfo->fd);
		goto err;
	}
	

	pstInfo->screen_width = var->xres;
	pstInfo->screen_height = var->yres;
	pstInfo->screen_stride = fix->line_length;
	pstInfo->fb_men_len = fix->smem_len;   
	pstInfo->mem_buf = pShowScreen;

	DPRINTK("[%d] %d.%d  %d %d\n",pstInfo->layer,pstInfo->screen_width,pstInfo->screen_height,
		pstInfo->screen_stride,pstInfo->fb_men_len);

	return MSA_SUCCESS;
err:
	return MSA_FAILED;
}


int gui_open_fb(MSA_OUTPUT_DEV enDev,int iScreenWidth,int iScreenHeight)
{
	int ret = 0;

	if( enDev >=HD_FB_NUM  || enDev < 0 )
		return MSA_ERR_INPUT_PARAMETERS_ERR;


	if( hd_layer_open[enDev] == 1 )	
		return MSA_ERR_ALREADY_INIT;

	memset(&hd_layer[enDev],0x00,sizeof(MSA_OUTPUT_DEV));

	hd_layer[enDev].layer = enDev;

	ret = dev_fb_init(&hd_layer[enDev],0,iScreenWidth,iScreenHeight);

	if( ret == MSA_SUCCESS )	
		hd_layer_open[enDev] = 1;		

	return ret;
}

int gui_close_fb(MSA_OUTPUT_DEV enDev)
{
	if( enDev >=HD_FB_NUM  || enDev < 0 )
			return MSA_ERR_INPUT_PARAMETERS_ERR;

	
	if( hd_layer_open[enDev] == 0 )	
		return MSA_ERR_NOT_INIT;

	munmap(hd_layer[enDev].mem_buf, hd_layer[enDev].fixinfo.smem_len);

	close(hd_layer[enDev].fd);    
       	hd_layer[enDev].fd=-1; 

	hd_layer_open[enDev] = 0;
}


void MSA_ClearOsd(int x,int y,int w,int h,MSA_OUTPUT_DEV dev)
{
	TDE2_RECT_S stSrcRect;
	TDE_HANDLE s32Handle;
	HI_U32 s32Ret;
	HI_U32 u32FillData=FB_BK_COLOR;
	TDE2_SURFACE_S g_stScreen;
	MSA_OUTPUT_DEV enDev;

	enDev = dev;

	
	g_stScreen.enColorFmt = 5;
	g_stScreen.bAlphaMax255 = HI_TRUE;
	stSrcRect.s32Xpos = x;
	stSrcRect.s32Ypos = y;
	stSrcRect.u32Width = w;
	stSrcRect.u32Height = h;

	g_stScreen.u32PhyAddr = hd_layer[enDev].fixinfo.smem_start;
	g_stScreen.u32Width = hd_layer[enDev].screen_width;
	g_stScreen.u32Height = hd_layer[enDev].screen_height;
	g_stScreen.u32Stride = hd_layer[enDev].screen_stride;

	s32Handle = HI_TDE2_BeginJob();
	if(HI_ERR_TDE_INVALID_HANDLE == s32Handle)
	{
		DPRINTK("Line:%d,HI_TDE2_BeginJob failed!\n",__LINE__);
		return ;
	}
	s32Ret = HI_TDE2_QuickFill(s32Handle,&g_stScreen,&stSrcRect,u32FillData);
	if(s32Ret < 0)
	{
		DPRINTK("Line:%d,HI_TDE2_QuickFill failed,ret=0x%x!\n", __LINE__, s32Ret);
		HI_TDE2_CancelJob(s32Handle);
		return ;
	}	

	s32Ret = HI_TDE2_EndJob(s32Handle, HI_FALSE, HI_TRUE, 10);
	if(s32Ret < 0)
	{
		DBGPRINT("Line:%d,HI_TDE2_EndJob failed,ret=0x%x!\n", __LINE__, s32Ret);
		HI_TDE2_CancelJob(s32Handle);
	}
}



int MSA_GuiOpenPicData(char * data_name,MSA_OUTPUT_DEV dev)
{
	FILE * fp = NULL;
	long fileOffset = 0;	
	int rel,i,j;
	char * tmp_buf = NULL;
	unsigned short *p=NULL;	
	MSA_OUTPUT_DEV layer;

	layer = dev;
		

	if( layer >=HD_FB_NUM  || layer < 0 )
		return MSA_ERR_INPUT_PARAMETERS_ERR;



	fp = fopen(data_name,"rb");
	if( fp == NULL )
	{
		DPRINTK(" open %s!\n",data_name);
		goto read_faild;
	}



	rel = fseek(fp,0L,SEEK_END);
	if( rel != 0 )
	{
		DPRINTK("fseek error!!\n");
		goto read_faild;
	}



	fileOffset = ftell(fp);
	if( fileOffset == -1 )
	{
		DPRINTK(" ftell error!\n");
		goto read_faild;
	}



	rewind(fp);



	tmp_buf = (void*)malloc(fileOffset);

	if( tmp_buf == NULL )
	{
		DPRINTK("malloc error!\n");
		goto read_faild;
	}



	rel = fread(tmp_buf,1,fileOffset,fp);
	if( rel != fileOffset )
	{
		DPRINTK(" fread Error!\n");
		goto read_faild;
	}	



	p=(unsigned short *)tmp_buf;

	j=fileOffset/2;

	for(i=0;i<j;i++)
	{

		if(p[i]==0x7c00)
			p[i]=0xfc00;

		else if( (p[i] & 0x8000) == 1 )
		{
			p[i] = p[i] & 0x7fff;
		}
	}		



	if( fp )
		fclose(fp);

	if( hd_layer[layer].pic_buf )
	{
		DPRINTK("release pic buf!\n");
		free(hd_layer[layer].pic_buf);
		hd_layer[layer].pic_buf = NULL;
	}


	hd_layer[layer].pic_buf = tmp_buf;
	hd_layer[layer].pic_data_length = fileOffset;


	return 1;


read_faild:
	if( fp )
		fclose(fp);	

	if( tmp_buf )
		free(tmp_buf);
	
	return -1;
}

int MSA_GuiClosePicData(MSA_OUTPUT_DEV layer)
{
	if( layer >=HD_FB_NUM  || layer < 0 )
		return MSA_ERR_INPUT_PARAMETERS_ERR;

	if( hd_layer[layer].pic_buf )
	{
		DPRINTK("release %d pic buf!\n",layer);
		free(hd_layer[layer].pic_buf);
		hd_layer[layer].pic_buf = NULL;
		hd_layer[layer].pic_data_length = 0;
	}

	return 1;
}



GUI_DRAW_ITEM * gui_get_pic_data_by_name(char * name,MSA_OUTPUT_DEV layer)
{

	GUI_DRAW_ITEM * pGuiItem = NULL;

	char * data_buf=NULL;;

	int data_length = 0;

	int offset = 0;

	int show_pic = 0;



	if( hd_layer[layer].pic_buf == NULL )

	{

		DPRINTK("not load guid file! %d,%s\n",layer,name);

		goto find_error;

	}



	data_buf = hd_layer[layer].pic_buf;

	data_length = hd_layer[layer].pic_data_length;



	while(1)

	{

		pGuiItem = (GUI_DRAW_ITEM *)(data_buf + offset);



		if(strcmp(name,pGuiItem->name) == 0)

		{

			show_pic = 1;

			break;

		}



		if( pGuiItem->item_id == -1 )

			offset = offset + sizeof(GUI_DRAW_ITEM);

		else

			offset = pGuiItem->data_offset + pGuiItem->data_length;



		if(offset >= data_length )

			break;



	}



	if( show_pic == 0 )
	{
		DPRINTK("no this name pic\n");
		goto find_error;

	}

	pGuiItem->data_buf = (void*)&data_buf[pGuiItem->data_offset];
	return pGuiItem;



find_error:

	return NULL;
}

GUI_DRAW_ITEM * MSA_GetPicDataByName(char * name,MSA_OUTPUT_DEV layer)
{
	return gui_get_pic_data_by_name(name,layer);
}


GUI_DRAW_ITEM * gui_get_pic_data_by_id(int title_id,int item_id,MSA_OUTPUT_DEV layer)
{

		GUI_DRAW_ITEM * pGuiItem = NULL;

		void * data_buf=NULL;;

		int data_length = 0;

		int offset = 0;

		int show_pic = 0;



		if( hd_layer[layer].pic_buf == NULL )

		{

			DPRINTK("not load guid file!\n");

			goto find_error;

		}



		data_buf = hd_layer[layer].pic_buf;

		data_length = hd_layer[layer].pic_data_length;



		while(1)

		{

			pGuiItem = (GUI_DRAW_ITEM *)(data_buf + offset);



			if(pGuiItem->item_id == item_id && pGuiItem->title_id ==title_id  )

			{

				//			DPRINTK("find pic %d,%d\n", pGuiItem->title_id,pGuiItem->item_id);

				show_pic = 1;

				break;

			}



			if( pGuiItem->item_id == -1 )

				offset = offset + sizeof(GUI_DRAW_ITEM);

			else

				offset = pGuiItem->data_offset + pGuiItem->data_length;



			if(offset >= data_length )

				break;



		}



		if( show_pic == 0 )

		{

			DPRINTK("no this name pic\n");

			goto find_error;

		}	



		return pGuiItem;



find_error:

		return NULL;



	}





int gui_draw_pic(GUI_DRAW_ITEM * pGuiItem,MSA_OUTPUT_DEV layer,int start_x,int start_y)
{

	int x,y;

	short * screen_buf;

	struct fb_var_screeninfo var;



	if( (start_x + pGuiItem->w)  > hd_layer[layer].screen_width  ||

		(start_y + pGuiItem->h) >  hd_layer[layer].screen_height  )

	{

		DPRINTK("(%d,%d,%d,%d) > (%d,%d) error!\n",start_x,pGuiItem->w,

			start_y, pGuiItem->h,hd_layer[layer].screen_width,hd_layer[layer].screen_height );

		return -1;

	}



	if( pGuiItem->draw_mode == GUI_PIC_DATA )

	{

		if( pGuiItem->rgb_mode == 0 || pGuiItem->rgb_mode == 24 )

		{



			short * pic_buf;

			int pic_line = 0;

			screen_buf = (short *)hd_layer[layer].mem_buf;

			pic_buf = (short *)pGuiItem->data_buf;



			pic_line = 0;



			for( y = start_y; y < start_y + pGuiItem->h; y++ )

			{

				memcpy(&screen_buf[y* hd_layer[layer].screen_stride /2 + start_x],

				&pic_buf[pic_line*pGuiItem->w],pGuiItem->w*2);



				pic_line++;

			}

		}



		if( pGuiItem->rgb_mode > 0 &&  pGuiItem->rgb_mode <= 16 )

		{			

			unsigned char  * pic_buf;

			short * color_buf;

			int pic_line = 0;

			int posx,posy;

			int line_num;

			int color_num;



			screen_buf = (short *)hd_layer[layer].mem_buf;

			color_buf = (short  *)pGuiItem->data_buf;

			pic_buf = (unsigned char  *)(pGuiItem->data_buf+16*2);



			line_num = (pGuiItem->w + 1)/2;



			pic_line = 0;

			posx = 0;

			posy = 0;



			for( y = start_y; y < start_y + pGuiItem->h; y++ )

			{				

				posx = 0;					



				for( x = start_x; x < start_x + pGuiItem->w; x++ )

				{				

					if(posx %2 == 0 )

					{					

						color_num = pic_buf[posy*line_num + posx/2] & 0x0f;

					}else

					{

						color_num = (pic_buf[posy*line_num + posx/2] & 0xf0) >> 4;

					}					



					screen_buf[y* hd_layer[layer].screen_stride /2 + x] = color_buf[color_num];



					posx++;						

				}

				posy++;				



			}				



		}



	}else

	{



	}	



	var.xoffset = 0;
	var.yoffset = 0;


	if (ioctl(hd_layer[layer].fd, FBIOPAN_DISPLAY, &var) < 0)
		DPRINTK("FBIOPAN_DISPLAY failed!\n");

	

	return 1;

}





int gui_draw_pic_by_name(char * name,int use_title_pos,MSA_OUTPUT_DEV layer)

{

	GUI_DRAW_ITEM * pGuiItem = NULL;

	GUI_DRAW_ITEM * pGuiTitleItem = NULL;

	int pos_x,pos_y;



	pGuiItem = gui_get_pic_data_by_name(name,layer);

	if(pGuiItem == NULL )

	{

		DPRINTK("get pic data : %s is err!\n",name);

		return -1;

	}



	if( pGuiItem->item_id == -1 )

	{

		pos_x =  pGuiItem->x;

		pos_y =  pGuiItem->y;

	}else

	{



		if( use_title_pos == 1 )

		{

			pGuiTitleItem = gui_get_pic_data_by_id(pGuiItem->title_id,-1,layer);

			pos_x = pGuiTitleItem->x  + pGuiItem->x;

			pos_y = pGuiTitleItem->y  + pGuiItem->y;

		}else

		{

			pos_x =  pGuiItem->x;

			pos_y =  pGuiItem->y;

		}

	}	



	return gui_draw_pic(pGuiItem,layer,pos_x,pos_y);

}



int gui_draw_pic_by_coordinate(GUI_DRAW_ITEM * pGuiItem,MSA_OUTPUT_DEV layer,int start_x,int start_y,

	int pic_x,int pic_y,int pic_w,int pic_h)

{

	int x,y;

	short * screen_buf;

	if( (start_x + pic_w)  > hd_layer[layer].screen_width  ||
		(start_y + pic_h) >  hd_layer[layer].screen_height  )
	{
		DPRINTK("[%d] screen(%d,%d,%d,%d) > (%d,%d) error!\n",layer,start_x,pGuiItem->w,
			start_y, pGuiItem->h,hd_layer[layer].screen_width,hd_layer[layer].screen_height );
		return -1;
	}



	if( (pic_x + pic_w)  > pGuiItem->w  ||
		(pic_y + pic_h) >  pGuiItem->h  )
	{
		DPRINTK("pic(%d,%d,%d,%d) > (%d,%d) error!\n",pic_x,pic_w,
			pic_y, pic_h,pGuiItem->w,pGuiItem->h);
		return -1;
	}


	if( pGuiItem->draw_mode == GUI_PIC_DATA )
	{
		if( pGuiItem->rgb_mode == 0 || pGuiItem->rgb_mode == 24 )
		{
			short * pic_buf;
			int pic_line = 0;
			screen_buf = (short *)hd_layer[layer].mem_buf;
			pic_buf = (short *)pGuiItem->data_buf;
			pic_line = pic_y;
			for( y = start_y; y < start_y + pic_h; y++ )
			{
				memcpy(&screen_buf[y* hd_layer[layer].screen_stride /2 + start_x],
					&pic_buf[pic_line*pGuiItem->w + pic_x],pic_w*2);
				pic_line++;
			}
		}

		if( pGuiItem->rgb_mode > 0 &&  pGuiItem->rgb_mode <= 16 )

		{			

			unsigned char  * pic_buf;

			short * color_buf;

			int pic_line = 0;

			int posx,posy;

			int line_num;

			int color_num;



			screen_buf = (short *)hd_layer[layer].mem_buf;

			color_buf = (short  *)pGuiItem->data_buf;

			pic_buf = (unsigned char  *)(pGuiItem->data_buf+16*2);



			line_num = (pGuiItem->w + 1)/2;



			pic_line = 0;

			posx = 0;

			posy = pic_y;



			for( y = start_y; y < start_y + pic_h; y++ )
			{				

				posx = pic_x;		

				for( x = start_x; x < start_x + pic_w; x++ )

				{				

					if(posx %2 == 0 )
					{					
						color_num = pic_buf[posy*line_num + posx/2] & 0x0f;
					}else
					{
						color_num = (pic_buf[posy*line_num + posx/2] & 0xf0) >> 4;
					}	
					screen_buf[y* hd_layer[layer].screen_stride /2 + x] = color_buf[color_num];
					posx++;	
				}
				posy++;	
			}				



		}



	}else
	{

	}	

	return 1;

}



int MSA_GuiDrawPicUseCoordinate(char * name,MSA_OUTPUT_DEV dev,int screen_x,int screen_y,
	int pic_x,int pic_y,int pic_w,int pic_h)
{

	GUI_DRAW_ITEM * pGuiItem = NULL;
	MSA_OUTPUT_DEV layer;

	layer = dev;

	if( layer >=HD_FB_NUM  || layer < 0 )
		return MSA_ERR_INPUT_PARAMETERS_ERR;	
	
	pGuiItem = gui_get_pic_data_by_name(name,layer);
	if(pGuiItem == NULL )
		return -1;		
	
	return gui_draw_pic_by_coordinate(pGuiItem,layer,screen_x,screen_y,pic_x,pic_y,pic_w,pic_h);
}



int gui_draw_pic_index(char * name,MSA_OUTPUT_DEV layer,int screen_x,int screen_y,
	int pic_x,int pic_y,int pic_w,int pic_h, int rowc,int linec)
{

	GUI_DRAW_ITEM * pGuiItem = NULL;
	pGuiItem = gui_get_pic_data_by_name(name,layer);
	if(pGuiItem == NULL )
		return -1;	
	return gui_draw_pic_by_coordinate(pGuiItem,layer,screen_x,screen_y,pic_x*(pGuiItem->w/rowc),pic_y*(pGuiItem->h/linec),pic_w*pGuiItem->w/rowc,pic_h*pGuiItem->h/linec);
}


int MSA_GuiDrawLine(MSA_OUTPUT_DEV dev,int start_x,int start_y,int end_x,int end_y,int lineStyle,unsigned short color)
{
	short * screen_buf;
	int y;
	int x;
	int i;
	int tmp;
	MSA_OUTPUT_DEV layer;

	layer = dev;

	if( layer >=HD_FB_NUM  || layer < 0 )
		return MSA_ERR_INPUT_PARAMETERS_ERR;
	
	screen_buf = (short *)hd_layer[layer].mem_buf;

	if( start_x != end_x  && start_y != end_y )
	{
		return MSA_ERR_INPUT_PARAMETERS_ERR;	
	}
	
	if( start_x == end_x )
	{
		if( start_y > end_y )
		{
			tmp = start_y;
			start_y = end_y;
			end_y = tmp;
		}
	
		for( y = start_y; y < end_y; y++ )
		{
			for( i = 0; i < lineStyle; i++)
			{
				screen_buf[y* hd_layer[layer].screen_stride /2 + start_x + i] = color;
			}
					
		}
	}else
	{	
		if( start_x > end_x )
		{
			tmp = start_x;
			start_x = end_x;
			end_x = tmp;
		}
	
		for( i = 0; i < lineStyle; i++)
		{
			y = start_y + i;
			for( x = start_x; x < end_x; x++ )
			{
				screen_buf[y* hd_layer[layer].screen_stride /2 + x] = color;			
			}
		}
	}

	return MSA_SUCCESS;	
}


#ifdef __cplusplus
}
#endif /* __cplusplus */





