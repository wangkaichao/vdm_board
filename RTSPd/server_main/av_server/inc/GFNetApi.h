 /******************************************************************************
* ��Ȩ��Ϣ��
* ϵͳ���ƣ�
* �ļ����ƣ�GFNet.h
* �ļ�˵�������ļ������˶�������ݽṹ�ͺ����ӿ�

* ����˵����20090409��	�޸ĵ�¼�û���������ͬʱ��¼16��
						������������ģʽ���޸Ĳ���ͬ����ɵĲ��ȶ�����

			20090415	������һ�������첽���޸Ľ����첽������ɵĲ��ȶ�

			20110421    �޸ĵ�¼�������ݷ��ͺͱ����ȷ��Ͳ���
						����һ��socket ����¼��select socket�б����д������ݺͱ�����Ҫ
						���͸�PC��ʱ��ͨ���������ݵ���socket ���ٻ���select������
						���д������ݺͱ����ķ���
			20110805    1���޸ĵ�¼�������ݷ��ͷ�ʽ����������ǰ�ڶ��û���¼��ʱ�򴮿�����ֻ��������IP�������
							���ݷ������е�¼�û�
						2������ͼƬ���ͽӿ�
			20120224	�޸�UPNP����ӦD-LINK�Ķ˿�ӳ������
******************************************************************************/

#ifndef		GF_SERVER_NET_H_
#define		GF_SERVER_NET_H_

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <time.h>

/////////////////////////**********************�궨��
#define USER_NAME_LENGTH_IN		16
#define USER_PSW_LENGTH_IN		16

/////////////////////////**********************���ݽṹ

//��ͷ
typedef struct _MP4_FRAME_HEAD
{
	unsigned int 	nMagicCode;
	unsigned int	nTimeTick;
	unsigned int	nVideoSize;
	unsigned int	bKeyFrame;
	unsigned int	nAudioSize;
	unsigned int	nReserve;	
}MP4_FRAME_HEAD,*PMP4_FRAME_HEAD;

typedef enum _GFERR_CODE
{
	GFERR_SUCCESS,					//�����ɹ�
	GFERR_FAILURE,					//����ʧ��
	GFERR_REFUSE_REQ,				//���󱻾ܾ�
	GFERR_USER_FULL,				//��¼�û�����
	GFERR_PREVIEW_FULL,				//Ԥ���û�����
	GFERR_TASK_FULL,				//ϵͳ����æ�����᳢������
	GFERR_CHANNEL_NOT_EXIST,		//Ҫ�򿪵�ͨ�������ڻ�����
	GFERR_DEVICE_NAME,				//�򿪵��豸������
	GFERR_IS_TALKING,				//���ڶԽ�
	GFERR_QUEUE_FAILUE,				//���г���
	GFERR_USER_PASSWORD,			//�û����������ϵͳ��ƥ��
	GFERR_SHARE_SOCKET,
	GFERR_RELAY_NOT_OPEN,
	GFERR_RELAY_MULTI_PORT,
	GFERR_CAPTURE_OVERTIME,
	
	GFERR_INVALID_PARAMETER=100,	//���������Ч
	GFERR_LOGON_FAILURE,			//��¼ʧ��
	GFERR_TIME_OUT,					//������ʱ
	GFERR_SOCKET_ERR,				//SOCKET����
	GFERR_NOT_LINKSERVER,			//��δ���ӷ�����
	GFERR_BUFFER_EXTCEED_LIMIT,		//ʹ�û��峬������	
	GFERR_LOW_PRIORITY,				//����Ȩ�޲���
	GFERR_BUFFER_SMALL,				//����̫С
	GFERR_IS_BUSY,					//ϵͳ������æ
	GFERR_UPDATE_FILE,				//�����ļ�����
	GFERR_UPDATE_UNMATCH,			//�����ļ��ͻ�����ƥ��
	GFERR_PORT_INUSE,				//�˿ڱ�ռ��
	GFERR_RELAY_DEVICE_EXIST,		//

    //2007.6.13
	GFERR_CONNECT_REFUSED,			//��?�ʱ��?�?
	GFERR_PROT_NOT_SURPPORT,		//��֧��?

	GFERR_FILE_OPEN_ERR,            //?���?
	GFERR_FILE_SEEK_ERR,            //fseek
	GFERR_FILE_WRITE_ERR,           //fwrite
	GFERR_FILE_READ_ERR,            //fread
	GFERR_FILE_CLOSING,             //            
}GFERR_CODE;

//֪ͨӦ�ó��򴰿���Ϣ����
typedef enum _GFMSG_NOTIFY
{
	GFMSG_CONNECT_CLOSE,			//��¼���ӹر�
	GFMSG_CHANNEL_CLOSE,			//ͨ�����ӹر�
	GFMSG_TALK_CLOSE,				//�Խ����ӹر�
	GFMSG_ALARM_OUTPUT,				//�������
	GFMSG_UPDATE_SEND_PERCENT,		//���������Ͱٷֱ�
	GFMSG_UPDATE_SAVE_PERCENT,		//����д�뷢�Ͱٷֱ�
	GFMSG_BROADCAST_CLOSE,			//�����㲥��һ���Ͽ�
	GFMSG_SENSOR_CAPTURE,			//̽ͷ������ץ��
	GFMSG_COM_DATA,					//���ڲɼ�����
}GFMSG_NOTIFY;

typedef enum _GFCMD_NET
{
	GFCMD_GET_ALL_PARAMETER,		//0. �õ����б���������
	GFCMD_SET_DEFAULT_PARAMETER,	//1. �ָ����б�����Ĭ�ϲ���
	GFCMD_SET_RESTART_DVS,			//2. ����������
	GFCMD_GET_SYS_CONFIG,			//3. ��ȡϵͳ����
	GFCMD_SET_SYS_CONFIG,			//4. ����ϵͳ����
	GFCMD_GET_TIME,					//5. ��ȡ������ʱ��
	GFCMD_SET_TIME,					//6. ���ñ�����ʱ��
	GFCMD_GET_AUDIO_CONFIG,			//7. ��ȡ��Ƶ����
	GFCMD_SET_AUDIO_CONFIG,			//8. ������Ƶ����
	GFCMD_GET_VIDEO_CONFIG,			//9. ��ȡ��Ƶ����
	GFCMD_SET_VIDEO_CONFIG,			//10.������Ƶ����
	GFCMD_GET_VMOTION_CONFIG,		//11.��ȡ�ƶ��������
	GFCMD_SET_VMOTION_CONFIG,		//12.�����ƶ��������
	GFCMD_GET_VMASK_CONFIG,			//13.��ȡͼ����������
	GFCMD_SET_VMASK_CONFIG,			//14.����ͼ����������
	GFCMD_GET_VLOST_CONFIG,			//15.��ȡ��Ƶ��ʧ����
	GFCMD_SET_VLOST_CONFIG,			//16.������Ƶ��ʧ����
	GFCMD_GET_SENSOR_ALARM,			//17.��ȡ̽ͷ�����������
	GFCMD_SET_SENSOR_ALARM,			//18.����̽ͷ�����������
	GFCMD_GET_USER_CONFIG,			//19.��ȡ�û�����
	GFCMD_SET_USER_CONFIG,			//20.�����û�����
	GFCMD_GET_NET_CONFIG,			//21.��ȡ�������ýṹ
	GFCMD_SET_NET_CONFIG,			//22.�����������ýṹ
	GFCMD_GET_COM_CONFIG,			//23.��ȡ��������
	GFCMD_SET_COM_CONFIG,			//24.���ô�������
	GFCMD_GET_YUNTAI_CONFIG,		//25.��ȡ������̨��Ϣ
	GFCMD_SET_YUNTAI_CONFIG,		//26.����������̨��Ϣ
	GFCMD_GET_VIDEO_SIGNAL_CONFIG,	//27.��ȡ��Ƶ�źŲ��������ȡ�ɫ�ȡ��Աȶȡ����Ͷȣ�
	GFCMD_SET_VIDEO_SIGNAL_CONFIG,	//28.������Ƶ�źŲ��������ȡ�ɫ�ȡ��Աȶȡ����Ͷȣ�
	GFCMD_SET_PAN_CTRL,				//29.��̨����
	GFCMD_SET_COMM_SENDDATA,		//30.͸�����ݴ���
	GFCMD_SET_COMM_START_GETDATA,	//31.��ʼ�ɼ�͸������
	GFCMD_SET_COMM_STOP_GETDATA,	//32.ֹͣ�ɼ�͸������
	GFCMD_SET_OUTPUT_CTRL,			//33.�̵�������
	GFCMD_SET_PRINT_DEBUG,			//34.������Ϣ����
	GFCMD_SET_ALARM_CLEAR,			//35.�������
	GFCMD_GET_ALARM_INFO,			//36.��ȡ����״̬�ͼ̵���״̬
	GFCMD_SET_TW2824,				//37.���ö໭��оƬ����(����)
	GFCMD_SET_SAVE_PARAM,			//38.���ñ������
	GFCMD_GET_USERINFO,				//39.��ȡ��ǰ��½���û���Ϣ
	GFCMD_GET_DDNS,					//40.��ȡDDNS
	GFCMD_SET_DDNS,					//41.����DDNS
	GFCMD_GET_CAPTURE_PIC,			//42.ǰ��ץ��
	GFCMD_GET_SENSOR_CAP,			//43.��ȡ����ץ������
	GFCMD_SET_SENSOR_CAP,			//44.���ô���ץ������
	GFCMD_GET_EXTINFO,				//45.��ȡ��չ����
	GFCMD_SET_EXTINFO,				//46.������չ����
	GFCMD_GET_USERDATA,				//47.��ȡ�û�����
	GFCMD_SET_USERDATA,				//48.�����û�����
	GFCMD_GET_NTP,					//49.��ȡNTP����
	GFCMD_SET_NTP,					//50.����NTP����
	GFCMD_GET_UPNP,					//51.��ȡUPNP����
	GFCMD_SET_UPNP,					//52.����UPNP����
	GFCMD_GET_MAIL,					//53.��ȡMAIL����
	GFCMD_SET_MAIL,					//54.����MAIL����
	GFCMD_GET_ALARMNAME,			//55.��ȡ����������
	GFCMD_SET_ALARMNAME,			//56.���ñ���������
	GFCMD_GET_WFNET,				//57.��ȡ������������
	GFCMD_SET_WFNET,				//58.����������������
	GFCMD_GET_SEND_DEST,			//59.������Ƶ������Ŀ���
	GFCMD_SET_SEND_DEST,			//60.������Ƶ������Ŀ���
	GFCMD_GET_AUTO_RESET,			//61.ȡ�ö�ʱ����ע��
	GFCMD_SET_AUTO_RESET,			//62.���ö�ʱ����ע��
	GFCMD_GET_REC_SCHEDULE,			//63.ȡ��¼�����
	GFCMD_SET_REC_SCHEDULE,			//64.����¼�����
	GFCMD_GET_DISK_INFO,			//65.ȡ�ô�����Ϣ
	GFCMD_SET_MANAGE,				//66.��������Ͳ���
	GFCMD_GET_CMOS_REG,				//67.ȡ��CMOS����
	GFCMD_SET_CMOS_REG,				//68.����CMOS����
	GFCMD_SET_SYSTEM_CMD,			//69.����ִ������
	GFCMD_SET_KEYFRAME_REQ,			//70.���ùؼ�֡����
	GFCMD_GET_CONFIGENCPAR,			//71.ȡ����Ƶ����
	GFCMD_SET_CONFIGENCPAR,			//72.������Ƶ����
	//--------------------------
	GFCMD_GET_ALL_PARAMETER_NEW,	//73.��ȡ���в���
	GFCMD_FING_LOG,					//74.������־(��ѯ��ʽ:0��ȫ����1�������ͣ�2����ʱ�䣬3����ʱ������� 0xFF-�رձ�������)
	GFCMD_GET_LOG,					//75.��ȡ���ҵ�����־	
	GFCMD_GET_SUPPORT_AV_FMT,		//76.��ȡ�豸֧�ֵı����ʽ����߼���Ƶ��ʽ
	GFCMD_GET_VIDEO_CONFIG_NEW,		//77.��Ƶ������new
	GFCMD_SET_VIDEO_CONFIG_NEW,		//78.
	GFCMD_GET_VMOTION_CONFIG_NEW,	//79.�ƶ�����������new
	GFCMD_SET_VMOTION_CONFIG_NEW,	//80.
	GFCMD_GET_VLOST_CONFIG_NEW,		//81.��Ƶ��ʧ����������new
	GFCMD_SET_VLOST_CONFIG_NEW,		//82.
	GFCMD_GET_SENSOR_ALARM_NEW,		//83.̽ͷ����������new
	GFCMD_SET_SENSOR_ALARM_NEW,		//84.
	GFCMD_GET_NET_ALARM_CONFIG,		//85.������ϱ���������new
	GFCMD_SET_NET_ALARM_CONFIG,		//86.
	GFCMD_GET_RECORD_CONFIG,		//87.��ʱ¼�����
	GFCMD_SET_RECORD_CONFIG,		//88.
	GFCMD_GET_SHOOT_CONFIG,			//89.��ʱץ�Ĳ���
	GFCMD_SET_SHOOT_CONFIG,			//90.
	GFCMD_GET_FTP_CONFIG,			//91.FTP����
	GFCMD_SET_FTP_CONFIG,			//92.
	GFCMD_GET_RF_ALARM_CONFIG,		//93.���߱�������
	GFCMD_SET_RF_ALARM_CONFIG,		//94.
	GFCMD_GET_EXT_DATA_CONFIG,		//95.������չ����(��ƽ̨������������)
	GFCMD_SET_EXT_DATA_CONFIG,		//96.
	GFCMD_GET_FORMAT_PROCESS,		//97.��ȡӲ�̸�ʽ������
	GFCMD_GET_PING_CONFIG,			//98.PING ���û�ȡ
	GFCMD_SET_PING_CONFIG,			//99.PING ��������

	//����������
	DDCMD_GET_ALL_PARAMETER = 100,	//��ȡ��������������
	DDCMD_GET_TIME,					//��ȡϵͳʱ��
	DDCMD_SET_TIME,					//����ϵͳʱ��
	DDCMD_GET_SYS_CONFIG,			//��ȡϵͳ����
	DDCMD_SET_SYS_CONFIG,			//����ϵͳ����
	DDCMD_GET_NET_CONFIG,			//��ȡ��������
	DDCMD_SET_NET_CONFIG,			//������������
	DDCMD_GET_COM_CONFIG,			//��ȡ��������
	DDCMD_SET_COM_CONFIG,			//���ô�������
	DDCMD_GET_VIDEO_CONFIG,			//��ȡ��Ƶ����
	DDCMD_SET_VIDEO_CONFIG,			//������Ƶ����
	DDCMD_GET_ALARM_OPT,			//��ȡ����ѡ��
	DDCMD_SET_ALARM_OPT,			//���ñ���ѡ��
	DDCMD_GET_USER_INFO,			//��ȡ�û�������Ϣ
	DDCMD_SET_USER_INFO,			//�����û�������Ϣ
	DDCMD_GET_ALARM_RECORD,			//��ȡ������¼��Ϣ
	DDCMD_GET_ADRRESS_BOOK,			//��ȡ��ַ������
	DDCMD_SET_ADRRESS_BOOK,			//���õ�ַ������
	DDCMD_SET_COMM,					//���÷��ʹ�������
	DDCMD_SET_CMD,					//����͸��������
	DDCMD_GET_YUNTAI_INFO,			//��ȡ��̨��Ϣ
	DDCMD_GET_YUNTAI_CONFIG,		//��ȡ��̨����
	DDCMD_SET_YUNTAI_CONFIG,		//������̨����
	DDCMD_GET_ONELINK_ADDR,			//��ȡ��������·���ӵ���Ϣ
	DDCMD_SET_ONELINK_ADDR,			//���ý�������·���ӵ���Ϣ
	DDCMD_GET_CYCLELINK_ADDR,		//��ȡ������ѭ�����ӵ���Ϣ
	DDCMD_SET_CYCLELINK_ADDR,		//���ý�����ѭ�����ӵ���Ϣ
	DDCMD_GET_DDNS,					//��ȡDDNS
	DDCMD_SET_DDNS,					//����DDNS
	
	GFCMD_GET_VPN_CONFIG = 200,
	GFCMD_SET_VPN_CONFIG,
	GFCMD_GET_3G_CONFIG,
	GFCMD_SET_3G_CONFIG,
	GFCMD_GET_GPS_CONFIG,
	GFCMD_SET_GPS_CONFIG,
		
	GFCMD_GET_VERSION_STRING = 400,	//3518���ƽ̨����ȡ�汾�ַ���
	
	NETCMD_GET_VI_SENSOR=1000,
	NETCMD_SET_VI_SENSOR,
	NETCMD_GET_VI_SCENE,
	NETCMD_SET_VI_SCENE,
	NETCMD_GET_VI_CFG,
	NETCMD_SET_VI_CFG,
	NETCMD_GET_DOME_PTZ_CFG,
	NETCMD_SET_DOME_PTZ_CFG,	
	NETCMD_GET_DOME_PRESET_CFG,
	NETCMD_SET_DOME_PRESET_CFG,	
	NETCMD_MAX_CONFIG,	
}GFCMD_NET;

typedef enum _NET_MSG_NOTIFY
{
	MSG_LOGIN_LINK,
	MSG_LOGIN_UNLINK,
	MSG_PREVIEW_LINK,
	MSG_PREVIEW_UNLINK,
	MSG_TALK_LINK,
	MSG_TALK_UNLINK,
	MSG_BROADCAST_LINK,
	MSG_BROADCAST_UNLINK,
	MSG_FILE_LINK,
	MSG_FILE_UNLINK,
	MSG_PIC_LINK,
	MSG_PIC_UNLINK,
	MSG_PIC_REQUEST,
	MSG_CENTER_LINK,
	MSG_CENTER_UNLINK,
}NET_MSG_NOTIFY;

unsigned long GFSNet_WriteFrame(int nChannelNo,const char *pFrameData,const char *pAudioData);


#endif
























