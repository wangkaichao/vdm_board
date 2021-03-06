

#ifndef _FAVC_MOTION_DETCTION_
#define _FAVC_MOTION_DETCTION_

struct favc_md
{
	 unsigned int mb_width; 
	 unsigned int mb_height;
	
   char *mvs0_x;    //adaptive mvs0 x
   char *mvs0_y;    //adaptive mvs0 y
   char *mvs1_x;    //adaptive mvs1 x
   char *mvs1_y;    //adaptive mvs1 y
   
   unsigned char *l_mvsp;      //PMB flag
   unsigned char *IMB_flag;    //IMB number
   unsigned char *l_mvsi;      //IMB flag
   
   unsigned int sad16_cur;
   unsigned int sad16_pre;
   unsigned int sad16_Th;     //adaptive sad threshold

   int sad_mask;
} favc_md;

struct md_cfg {
    int             interlace_mode;  //interlace mode (1: interlaced; 0: non-interlaced)
    unsigned int    usesad;          //use sad info flag 
    unsigned int    sad_offset;
    unsigned char  *mb_cell_en;
    unsigned char  *mb_cell_th;
    unsigned char   mb_time_th;      //successive N IMB regards as motion block (default: 3)
    unsigned char   rolling_prot_th; //(default: 30)
    unsigned int    alarm_th;
} md_cfg;


struct md_res{
	unsigned int  active_num;
	unsigned char *active_flag;
} md_res;

//-----------------------------------------------------------------------------
int favc_motion_info_init(struct md_cfg *md_param, struct md_res *active, struct favc_md *favcmd, unsigned int mb_width, unsigned int mb_height); 
int favc_motion_detection(unsigned char *mb_array, struct md_cfg *md_param, struct md_res *active, struct favc_md *favcmd);
int favc_motion_info_end(struct md_cfg *md_param, struct md_res *active, struct favc_md *favcmd); 
void favc_do_noise_filtering(unsigned char *lmvs, unsigned int mbwidth, unsigned int mbheight);
void favc_do_IMB_filtering(unsigned char *lmvs, unsigned int mbwidth, unsigned int mbheight);

#endif







