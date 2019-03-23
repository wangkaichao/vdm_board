/**************************************************************************
 * 	FileName:		global.h
 *	Description:	system param define(ϵͳȫ�ֶ���)
 *	Copyright(C):	2006-2008 GFDS Inc.
 *	Version:		V 1.0
 *	Author:			ChenYG
 *	Created:		2008-08-25
 *	Updated:		
 *					
 **************************************************************************/
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <errno.h>


#ifndef	  FRAME_FLAG_VP
#define	  FRAME_FLAG_VP           0x0b			//��Ƶ��P
#define	  FRAME_FLAG_VI           0x0e			//��Ƶ��I
#define	  FRAME_FLAG_A            0x0d			//
#endif

#ifndef PACKED
#define PACKED		__attribute__((packed, aligned(1)))
#endif


typedef struct __COM_PARAM
{
	unsigned int 			    Baudrate;				//300---115200
	unsigned char				Databit;				//��������λ����			5=5λ 6=6λ 7=7λ 8=8λ	����=8
	unsigned char				Stopbit;				//ֹͣλλ��				1=1λ 2=2λ ����=1λ
	unsigned char				CheckType;				//У��						0=��  1=��  2=ż  3=��1  4=��0
	unsigned char				Flowctrl; 				//������/Ӳ����/������		����
}PACKED COM_CONFIG;


typedef struct __COMM_CTRL
{
	unsigned char		COMMNo;					//0 ~ 1
	unsigned char		AD[3];
	COM_CONFIG	        COMConfig;		        //
	unsigned short	    DataLength;			    //���ݳ���
	unsigned char		Data[256];				//����
	unsigned char		AE[2];
}PACKED COMM_CTRL;

#define    GF_LOG_EMERG   0
#define    GF_LOG_ERR     1
#define    GF_LOG_WARN    2
#define    GF_LOG_DEBUG   3

//�궨��
#define MAX_SYSLOG_BUFSIZE		(32 + 8*1024)
#define MAX_SYSLOG_MEMSIZE		(256 * 1024)
#define MAX_SYSLOG_FILENUM		5
#define MAX_SYSLOG_FILESIZE		(48*1024)


typedef enum  _GF_VIDEO_ENCODER_E
{
		GF_VENC_NONE   = 0x0,
		GF_VENC_H264   = 0x01,
		GF_VENC_MPEG4  = 0x02,
		GF_VENC_MJPEG  = 0x03,
		GF_VENC_JPEG   = 0x04,
}GF_VIDEO_ENCODER_E;



//����
//#define GF_SYSLOG   printf

#endif

