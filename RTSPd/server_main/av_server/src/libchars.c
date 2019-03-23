
/*
 *get char mode
 * */
#include <stdio.h>

enum {
	hz_font,
	asc_font
};

#define ZF_FILE 	"ASC4824"
#define HZ_FILE 	"SHZK4848"
#define ASC_H 		48
#define ASC_W 		24
#define ASC_HZ_SIZE 	(2 * ASC_H * ASC_W / 8)
#define ASC_ZF_SIZE 	(ASC_H * ASC_W / 8)

static int asc_init = 0;
static char *origin_tmp = NULL;

unsigned char *ASC_MSK = NULL;

static  void getHzKCode(signed char *c, char buff[]);
static inline void do_blit(char *bitmap, char *origin, unsigned int BitbmpPix_X, int x, int y, int font);
/*  */
static  void get_zimo(char *str, char *buffer)
{
    //printf("Enter in get_zimo\n");
    //printf("str=%s,buffer=%p\n",str,buffer);
    int len;
    int i = 0;
    char *p;
    signed char c1, *str_;
    p = buffer;
    len = strlen(str);
    str_ = (signed char *)str;
    while (i < len)
    {
        c1 = str_[i];
        if(c1 >= 0)//ACSII码
        {
	    if (asc_init == 0)
		continue;

            if(c1 < 0x20)
            {
                memset(p, 0x00, ASC_ZF_SIZE);
            }
            else
            {
                memcpy(p, &ASC_MSK[(c1) * ASC_ZF_SIZE], ASC_ZF_SIZE);
            }

            p = p + ASC_ZF_SIZE;
        }
        else //汉子GB2312码
        {
            getHzKCode(&str_[i], p);
            //p = p + 32;
            p = p + ASC_HZ_SIZE;
            i++;
        }
        i++;
    }
    //printf("Enter out get_zimo i:%d\n",i);
}


static  void getHzKCode(signed char *c, char buff[])
{
    //printf("Enter in getHzKCode\n");
    unsigned char qh, wh;
    unsigned long offset;
    FILE *HZK;

    if((HZK = fopen(HZ_FILE, "rb")) == NULL)
    {
        printf("Can't open ziku,Please add it?");
    }
    qh     = *(c) - 0xa0;
    wh     = *(c + 1) - 0xa0;
    offset = (94 * (qh - 1) + (wh - 1)) * ASC_HZ_SIZE;
    fseek(HZK, offset, SEEK_SET);
    fread(buff, ASC_HZ_SIZE, 1, HZK);
    fclose(HZK);
    //printf("Enter out getHzKCode\n");
}

#define ZIFU_H			48
#define ZIFU_W			24
#define ZIFU_N			80
#define MAX_LIST_FD		50
#define PRE_BYTE_ADD 		16
#define MAX_PIC_LEN     (2 * (ZIFU_W) * (ZIFU_H) * (ZIFU_N) + PRE_BYTE_ADD)
#define MAX_CHAR_NUM	(ZIFU_W * 2 + PRE_BYTE_ADD)

void lprint(char *origin, char *new_string, char *old_string, unsigned int BitbmpPix_X, int x, int y, char *buffer)
{
    //printf("enter in lprint ...\n");
    signed char *c = (signed  char *) new_string;
    int i;
    int x_, y_;
    int str_equ = 1;
    x_ = x;
    y_ = y;
    char *bitmap;
    int needCmp = 1;

    if (asc_init == 0) {
    	FILE *fp = NULL;
    	if ((fp = fopen(ZF_FILE, "rb")) == NULL) {
		printf("open zf file fail.\n");
	} else {
		ASC_MSK = (unsigned char *)malloc(20 * 1024);
		if (ASC_MSK == NULL) {
			printf("malloc asc msk fail\n");
		} else {
			fread(ASC_MSK, 20 * 1024, 1, fp);
			fclose(fp);
			asc_init = 1;
		}
	}
	if (origin_tmp == NULL)
    		origin_tmp = (char *)malloc(MAX_PIC_LEN * sizeof(char));
    }

    if (old_string == NULL)
        needCmp = 0;

    get_zimo(new_string, buffer);
		
    bitmap = buffer;
    for(i = 0; i < strlen(new_string); i++)
    {
    		/* ASCII */
        if(c[i] >= 0)
        {
            if(needCmp == 1)
            {
                str_equ = memcmp(&new_string[i], &old_string[i], 1);
                if(str_equ != 0)
                {
                    do_blit(bitmap, origin, BitbmpPix_X, x_, y_, asc_font);
                }
            }
            else
            {
                do_blit(bitmap, origin, BitbmpPix_X, x_, y_, asc_font);
            }
            bitmap += ASC_ZF_SIZE;
            x_ += ASC_W * 2;
        }
        else
        {
            if(needCmp)
            {
                str_equ = memcmp(&new_string[i], &old_string[i], 2);
                if(str_equ != 0)
                {
                    do_blit(bitmap, origin, BitbmpPix_X, x_, y_, hz_font);
                }
            }
            else
            {
                do_blit(bitmap, origin, BitbmpPix_X, x_, y_, hz_font);
            }
            bitmap += ASC_HZ_SIZE;
            x_ += ASC_W * 2 * 2;    //
            i++;
        }
    }
	
#if 1
    int row;int xx;int j;int m;

    if (NULL == origin_tmp)
        return;
    
    memcpy(origin_tmp,origin,MAX_PIC_LEN * sizeof(char));

    for(m = 0; m < i; m++) {
    	for(row = 1; row < (ASC_H - 1); row++)
    	{
            xx = row * (ASC_W * 2) * i + m * (ASC_W * 2);
            for(j = 1; j < (ASC_W * 2 - 1); j++)
            {
           	if((origin_tmp[xx+j*2] == 0xff)&&(origin_tmp[xx+j*2+1]==0x7f))
           	{
                    if((origin_tmp[xx+j*2-(ASC_W * 2)*i] != 0xff) && (origin_tmp[xx+j*2-(ASC_W * 2)*i+1] != 0x7f))
                    {
                        origin[xx+j*2-(ASC_W * 2)*i] = 0x00;
                        origin[xx+j*2-(ASC_W * 2)*i+1] = 0x00;
                    }
                    if((origin_tmp[xx+j*2-2] != 0xff) && (origin_tmp[xx+j*2-2+1] != 0x7f))
                    {
                        origin[xx+j*2-2] = 0x00;
                        origin[xx+j*2-2+1] = 0x00;
                    }
                    if((origin_tmp[xx+j*2+2] != 0xff) && (origin_tmp[xx+j*2+2+1] != 0x7f))
                    {
                        origin[xx+j*2+2] = 0x00;
                        origin[xx+j*2+2+1] = 0x00;
                    }
                    if((origin_tmp[xx+(ASC_W * 2)*i+j*2] != 0xff) && (origin_tmp[xx+(ASC_W * 2)*i+j*2+1] != 0x7f))
                    {
                        origin[xx+(ASC_W * 2)*i+j*2] = 0x00;
                        origin[xx+(ASC_W * 2)*i+j*2+1] = 0x00;
                    }
               }
            }
        //y_next++;
        }
    }
#endif

    //printf("lprint\n");
}


static inline void do_blit(char *bitmap, char *origin, unsigned int BitbmpPix_X, int x, int y, int font)
{
    char *pixel;
    int row, col;
    int y_next = y;
    int j;
    int zom_y;
    int Bpline = 0;

    if(font == hz_font)
    {
	Bpline = ASC_W * 2 / 8;
        for(row = 0; row < ASC_H; row++)
        {
            pixel = origin + (BitbmpPix_X  * 2*(y_next)) + x;

            for(j = 0; j < Bpline; j++)
            {
                // 1byte = 8bits
                for(col = 0; col < 8; col++)
                {
                    if(bitmap[row*Bpline+j] &(0x80 >> col))
                    {
                        pixel[(j*8 + col)*2] = 0xff;
                        pixel[(j*8 + col)*2 + 1] = 0x7f;
                    }
                    else
                    {
                        pixel[(col+j*8)*2] = 0x00;
                        pixel[(col+j*8)*2 + 1] = 0x80;
                    }
                }
            }
            y_next++; /* Next line in bitmap */
        }
        //y_next = 1;
    }
    else
    {
	Bpline = ASC_W / 8;
        for(row = 0; row < ASC_H; row++)
        {
            pixel = origin + (BitbmpPix_X * 2 * (y_next)) + x;
            for(j = 0; j < Bpline; j++)
            {
                for(col = 0; col < 8; col++)
                {
                    if(bitmap[row*Bpline+j] &(0x80 >> col))
                    {
                        pixel[(j*8 + col)*2] = 0xff;
                        pixel[(j*8 + col)*2 + 1] = 0x7f;
                    }
                    else
                    {
                        pixel[(col+j*8)*2] = 0x00;
                        pixel[(col+j*8)*2 + 1] = 0x80;
                    }
                }
            }
            y_next++; /* Next line in bitmap */
        }
    }
}
