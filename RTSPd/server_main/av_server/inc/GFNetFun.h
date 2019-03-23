#ifndef _GF_NET_FUN_H
#define _GF_NET_FUN_H

#include <sys/time.h>
#include <sys/socket.h>

#define LISTENQ 1024
#define GF_MAX_BLOCK_SEND_TIME 3
#define GF_MAX_BLOCK_RECV_TIME 3

#define GF_MAX_NOBLOCK_RECV_TIME 500
#define GF_MAX_NOBLOCK_SEND_TIME 2000
#define GF_MAX_NOBLOCK_CHECK_SND_TIME 100 //ÿ�μ��ʱ��ԼΪ20ms
//����socket��һЩ��������
int GF_Set_Sock_Attr(int fd, int bReuseAddr, int nSndTimeO, int nRcvTimeO, int nSndBuf, int nRcvBuf);
// ָ����Socket ��ʹ��Nagle �㷨
int GF_Set_Sock_NoDelay(int fd);
//����һ��tcp������
int GF_Tcp_Listen(const char *host, const char *serv, int *addrlenp);
//����socketΪ����socket
int GF_Set_Sock_Block(int nSock);
//����socketΪ������socket
int GF_Set_Sock_NoBlock(int nSock);
//tcp������ʽ���ӷ����
int GF_Tcp_Block_Connect (const char *localHost, const char *localServ, const char *dstHost, const char *dstServ);
//tcp����socket���պ���
int GF_Tcp_Block_Recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize);
//tcp������ʽ����
int GF_Tcp_Block_Send(int fd, const void *vptr, int n);
//tcp ������ʽaccept
int GF_Tcp_Block_Accept(int fd, struct sockaddr *sa, int *salenptr);
//tcp��������ʽ���ӷ����
int GF_Tcp_NoBlock_Connect(const char *localHost, const char *localServ, const char *dstHost, const char *dstServ, int timeout_ms);
//tcp������socket���պ���
int GF_Tcp_NoBlock_Recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize);
//tcp��������ʽ����
int GF_Tcp_NoBlock_Send(int hSock,char *pbuf,int size, int *pBlock);

int GF_Tcp_NoBlock_MTU_Send(int hSock,char *pbuf,int size, int mtu);
//tcp ��������ʽaccept
int GF_Tcp_NoBlock_Accept(int fd, struct sockaddr *sa, int *salenptr, struct timeval * to);

//udpsocket���պ���
int GF_Udp_Recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize, struct sockaddr *from, int *fromlen);
//udp��������ʽ����
int GF_Udp_Send(int hSock,char *pbuf,int size, struct sockaddr * distAddr);
//create socket
int GF_Create_Sock(int iType);
//ping ����ʵ�ֺ��� 
int GF_Ping(char *ips, int timeout, int max_times);
//ȡ�ø�����host��ip��ַ(���صĵ�һ����ַ)
int GF_Get_Host_Ip(int af, char *host);
//��ض��fd 
int GF_Select(int *fd_array, int fd_num, int fd_type, int time_out);
//�ر�socket
int GF_Close_Socket(int *fd);
//
int GF_Bind_Sock(int sockfd, int ip, int nPort);

int GF_Get_Sock_Ip(int sock);

int GF_Get_Sock_Port(int sock);
//  ָ��Socket keepalive ����
int GF_Set_Sock_KeepAlive(int fd);
//  ָ��Socket�㲥����
int  GF_Set_Sock_BoardCast(int fd);
//  ָ��Socket�ಥ����
int  GF_Set_Sock_MultiCast(int fd);
//  ָ��Socket����ಥ��
int  GF_Set_Sock_Multi_MemberShip(int fd, char *multiAddr, int interfaceIp);
//  ָ��Socket�˳��ಥ��
int GF_Set_Sock_Rm_Multi_MemberShip(int fd, char *multiAddr);

int GF_Sock_Bind_Interface(int sockfd, char *interfaceName);
	
unsigned long GF_Ip_N2A(unsigned long ip, char *ourIp, int len);
//ip�ַ�תlong
unsigned long GF_Ip_A2N(char *szIp);
//ip�ַ�������תlong
unsigned long GF_Ip_Ext_A2N(char *pstrIP);

void   GF_itoa(int   n,   char   s[]);

/*
SO_LINGER

   ��ѡ��ָ������close���������ӵ�Э����β�������TCP����
   ȱʡclose�������������أ���������ݲ������׽ӿڻ�������
   ��ϵͳ�����Ž���Щ���ݷ��͸��Է���
SO_LINGERѡ�������ı��ȱʡ���á�ʹ�����½ṹ��
struct linger {
     int l_onoff; // 0 = off, nozero = on 
     int l_linger; // linger time 
};

���������������

l_onoffΪ0�����ѡ��رգ�l_linger��ֵ�����ԣ�����ȱʡ�����close�������أ� 

l_onoffΪ��0��l_lingerΪ0�����׽ӿڹر�ʱTCPز�����ӣ�
TCP�������������׽ӿڷ��ͻ������е��κ����ݲ�����һ��RST���Է���
������ͨ�����ķ�����ֹ���У��������TIME_WAIT״̬�� 

l_onoff Ϊ��0��l_lingerΪ��0�����׽ӿڹر�ʱ�ں˽�����һ��ʱ�䣨��l_linger��������
����׽ӿڻ��������Բ������ݣ����̽�����˯��״̬��
ֱ ����a���������ݷ������ұ��Է�ȷ�ϣ�
֮�������������ֹ���У������ַ��ʼ���Ϊ0����b���ӳ�ʱ�䵽��
��������£�Ӧ�ó�����close�ķ���ֵ�Ƿǳ���Ҫ�ģ�
��������ݷ����겢��ȷ��ǰʱ�䵽��
close������EWOULDBLOCK�������׽ӿڷ��ͻ������е��κ����ݶ���ʧ��
close�ĳɹ����ؽ��������Ƿ��͵����ݣ���FIN�����ɶԷ�TCPȷ�ϣ�
�������ܸ������ǶԷ�Ӧ�ý����Ƿ��Ѷ������ݡ�
����׽ӿ���Ϊ�������ģ��������ȴ�close�� �ɡ� 


l_linger�ĵ�λ������ʵ�֣�4.4BSD�����䵥λ��ʱ�ӵδ𣨰ٷ�֮һ�룩��
��Posix.1g�涨��λΪ�롣

*/
int GF_Sock_Set_LINGER(int sockfd);

int GF_Sock_Unset_LINGER(int sockfd);


#endif

