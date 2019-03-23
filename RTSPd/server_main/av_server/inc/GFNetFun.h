#ifndef _GF_NET_FUN_H
#define _GF_NET_FUN_H

#include <sys/time.h>
#include <sys/socket.h>

#define LISTENQ 1024
#define GF_MAX_BLOCK_SEND_TIME 3
#define GF_MAX_BLOCK_RECV_TIME 3

#define GF_MAX_NOBLOCK_RECV_TIME 500
#define GF_MAX_NOBLOCK_SEND_TIME 2000
#define GF_MAX_NOBLOCK_CHECK_SND_TIME 100 //每次检测时间约为20ms
//设置socket的一些常用属性
int GF_Set_Sock_Attr(int fd, int bReuseAddr, int nSndTimeO, int nRcvTimeO, int nSndBuf, int nRcvBuf);
// 指定流Socket 是使用Nagle 算法
int GF_Set_Sock_NoDelay(int fd);
//建立一个tcp服务器
int GF_Tcp_Listen(const char *host, const char *serv, int *addrlenp);
//设置socket为阻塞socket
int GF_Set_Sock_Block(int nSock);
//设置socket为非阻塞socket
int GF_Set_Sock_NoBlock(int nSock);
//tcp阻塞方式连接服务端
int GF_Tcp_Block_Connect (const char *localHost, const char *localServ, const char *dstHost, const char *dstServ);
//tcp阻塞socket接收函数
int GF_Tcp_Block_Recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize);
//tcp阻塞方式发送
int GF_Tcp_Block_Send(int fd, const void *vptr, int n);
//tcp 阻塞方式accept
int GF_Tcp_Block_Accept(int fd, struct sockaddr *sa, int *salenptr);
//tcp非阻塞方式连接服务端
int GF_Tcp_NoBlock_Connect(const char *localHost, const char *localServ, const char *dstHost, const char *dstServ, int timeout_ms);
//tcp非阻塞socket接收函数
int GF_Tcp_NoBlock_Recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize);
//tcp非阻塞方式发送
int GF_Tcp_NoBlock_Send(int hSock,char *pbuf,int size, int *pBlock);

int GF_Tcp_NoBlock_MTU_Send(int hSock,char *pbuf,int size, int mtu);
//tcp 非阻塞方式accept
int GF_Tcp_NoBlock_Accept(int fd, struct sockaddr *sa, int *salenptr, struct timeval * to);

//udpsocket接收函数
int GF_Udp_Recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize, struct sockaddr *from, int *fromlen);
//udp非阻塞方式发送
int GF_Udp_Send(int hSock,char *pbuf,int size, struct sockaddr * distAddr);
//create socket
int GF_Create_Sock(int iType);
//ping 功能实现函数 
int GF_Ping(char *ips, int timeout, int max_times);
//取得给出的host的ip地址(返回的第一个地址)
int GF_Get_Host_Ip(int af, char *host);
//监控多个fd 
int GF_Select(int *fd_array, int fd_num, int fd_type, int time_out);
//关闭socket
int GF_Close_Socket(int *fd);
//
int GF_Bind_Sock(int sockfd, int ip, int nPort);

int GF_Get_Sock_Ip(int sock);

int GF_Get_Sock_Port(int sock);
//  指定Socket keepalive 属性
int GF_Set_Sock_KeepAlive(int fd);
//  指定Socket广播属性
int  GF_Set_Sock_BoardCast(int fd);
//  指定Socket多播属性
int  GF_Set_Sock_MultiCast(int fd);
//  指定Socket加入多播组
int  GF_Set_Sock_Multi_MemberShip(int fd, char *multiAddr, int interfaceIp);
//  指定Socket退出多播组
int GF_Set_Sock_Rm_Multi_MemberShip(int fd, char *multiAddr);

int GF_Sock_Bind_Interface(int sockfd, char *interfaceName);
	
unsigned long GF_Ip_N2A(unsigned long ip, char *ourIp, int len);
//ip字符转long
unsigned long GF_Ip_A2N(char *szIp);
//ip字符或域名转long
unsigned long GF_Ip_Ext_A2N(char *pstrIP);

void   GF_itoa(int   n,   char   s[]);

/*
SO_LINGER

   此选项指定函数close对面向连接的协议如何操作（如TCP）。
   缺省close操作是立即返回，如果有数据残留在套接口缓冲区中
   则系统将试着将这些数据发送给对方。
SO_LINGER选项用来改变此缺省设置。使用如下结构：
struct linger {
     int l_onoff; // 0 = off, nozero = on 
     int l_linger; // linger time 
};

有下列三种情况：

l_onoff为0，则该选项关闭，l_linger的值被忽略，等于缺省情况，close立即返回； 

l_onoff为非0，l_linger为0，则套接口关闭时TCP夭折连接，
TCP将丢弃保留在套接口发送缓冲区中的任何数据并发送一个RST给对方，
而不是通常的四分组终止序列，这避免了TIME_WAIT状态； 

l_onoff 为非0，l_linger为非0，当套接口关闭时内核将拖延一段时间（由l_linger决定）。
如果套接口缓冲区中仍残留数据，进程将处于睡眠状态，
直 到（a）所有数据发送完且被对方确认，
之后进行正常的终止序列（描述字访问计数为0）或（b）延迟时间到。
此种情况下，应用程序检查close的返回值是非常重要的，
如果在数据发送完并被确认前时间到，
close将返回EWOULDBLOCK错误且套接口发送缓冲区中的任何数据都丢失。
close的成功返回仅告诉我们发送的数据（和FIN）已由对方TCP确认，
它并不能告诉我们对方应用进程是否已读了数据。
如果套接口设为非阻塞的，它将不等待close完 成。 


l_linger的单位依赖于实现，4.4BSD假设其单位是时钟滴答（百分之一秒），
但Posix.1g规定单位为秒。

*/
int GF_Sock_Set_LINGER(int sockfd);

int GF_Sock_Unset_LINGER(int sockfd);


#endif

