#ifndef _OSD_LIB_HEAD_H_
#define _OSD_LIB_HEAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FB_BK_COLOR 0xfc00//0x7c00


#define GUI_PIC_DATA  (0)
#define GUI_POT_DATA  (1)

typedef struct  _GUI_DRAW_ITEM_
{
	int title_id;
	int item_id;
	int x;
	int y;
	int w;
	int h;
	int rgb_mode;  // 2 8 16 24
	int draw_mode;  // PIC_DATA POT_DATA
	char name[20];
	char pic_file_name[100];
	int data_offset;
	int data_length;
	void * data_buf;
	void * other_info;
}GUI_DRAW_ITEM;



#ifdef __cplusplus
}
#endif

#endif 

