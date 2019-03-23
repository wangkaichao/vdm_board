/**************************************************************
 *GFNetFun.c
 *
 *功能:实现一些网络常用函数的封装
 *
 *作者:ljm
 *
 *建立时间:2009-02-12
 *
 *
 *修改日志:
 *
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <net/if.h>

#include "global.h"

#include "GFNetFun.h"

#define GF_RETURN_FAIL -1
#define GF_RETURN_OK    0

/*********************************************************
  设置socket的一些常用属性

fd:所要设置的socket
bReuseAddr:设置重新绑定
nSndTimeO:设置socket的发送超时时间(单位毫秒), 填0不设置该项
nRcvTimeO:设置socket的发送超时时间(单位毫秒), 填0不设置该项
nSndBuf:设置socket的发送缓冲区的大小, 填0不设置该项
nRcvBuf:设置socket的接收缓冲区的大小, 填0不设置该项

返回值 :所用设置成功返回GF_RETURN_OK, 否则返回GF_RETURN_FAILE
 *********************************************************/

int GF_Set_Sock_Attr(int fd, int bReuseAddr, int nSndTimeO, int nRcvTimeO, int nSndBuf, int nRcvBuf)
{
	int err_ret = GF_RETURN_OK;	    
	struct timeval sndTo, rcvTo;

	if (fd <= 0)
		return GF_RETURN_FAIL;

	sndTo.tv_sec  = nSndTimeO / 1000;
	sndTo.tv_usec = (nSndTimeO % 1000) * 1000;

	rcvTo.tv_sec  = nRcvTimeO / 1000;
	rcvTo.tv_usec = (nRcvTimeO % 1000) * 1000;

	GF_Set_Sock_KeepAlive(fd); //2012-6-29 add by yangj

	if (bReuseAddr != 0 && setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(void *)&bReuseAddr,sizeof(int)) < 0)
		err_ret = GF_RETURN_FAIL;

	if (nSndTimeO != 0 && setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(void  *)&sndTo,sizeof(sndTo)) < 0)
		err_ret = GF_RETURN_FAIL;
	if (nSndTimeO != 0 && setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(void  *)&rcvTo,sizeof(rcvTo)) < 0)
		err_ret = GF_RETURN_FAIL;

	if (nSndBuf != 0 && setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(void  *)&nSndBuf,sizeof(nSndBuf)) < 0)
		err_ret = GF_RETURN_FAIL;
	if (nRcvBuf != 0 && setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(void  *)&nRcvBuf,sizeof(nSndBuf)) < 0)
		err_ret = GF_RETURN_FAIL;


	return err_ret;
}
/*********************************************************
  指定流Socket 是使用Nagle 算法

fd:所要设置的socket

返回值 :所用设置成功返回GF_RETURN_OK, 否则返回GF_RETURN_FAILE
 *********************************************************/
int GF_Set_Sock_NoDelay(int fd)
{
	int opt = 1;	

	if (fd <= 0)
		return GF_RETURN_FAIL;
	return setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&opt,sizeof(opt));
}
/*********************************************************
  指定Socket keepalive 属性

  返回值 :所用设置成功返回GF_RETURN_OK, 否则返回GF_RETURN_FAILE
 *********************************************************/
int GF_Set_Sock_KeepAlive(int fd)
{
	int opt = 1;	

	if (fd <= 0)
		return GF_RETURN_FAIL;
	return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,(char *)&opt,sizeof(opt));
}

int GF_Set_Sock_KeepAlive_TT(int fd, int keepIdle, int keepInterval, int keepCount)
{
	int opt = 1;

	if (fd <= 0)
		return GF_RETURN_FAIL;

	setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle)); 	
	setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval)); 	
	setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount)); 
	return GF_RETURN_OK;

}

/*********************************************************
  指定Socket广播属性

  返回值 :所用设置成功返回GF_RETURN_OK, 否则返回GF_RETURN_FAILE
 *********************************************************/
int  GF_Set_Sock_BoardCast(int fd)
{
	int    so_boardcast = 1;
	return setsockopt(fd,SOL_SOCKET,SO_BROADCAST,&so_boardcast,sizeof(so_boardcast));
}
/*********************************************************
  指定Socket多播属性

  返回值 :所用设置成功返回GF_RETURN_OK, 否则返回GF_RETURN_FAILE
 *********************************************************/
int  GF_Set_Sock_MultiCast(int fd)
{
	int opt=0;
	return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&opt,  sizeof(char));
}
/*********************************************************
  指定Socket加入多播组

  返回值 :所用设置成功返回GF_RETURN_OK, 否则返回GF_RETURN_FAILE
 *********************************************************/
int  GF_Set_Sock_Multi_MemberShip(int fd, char *multiAddr, int interfaceIp)
{
	int    ret = 0;
	struct ip_mreq	ipmreq;

	memset(&ipmreq, 0, sizeof(ipmreq));
	//add multicast membership
	ipmreq.imr_multiaddr.s_addr = inet_addr(multiAddr);
	ipmreq.imr_interface.s_addr = interfaceIp;

	ret = setsockopt(fd, IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&ipmreq,
			sizeof(ipmreq));
	return ret;
}
/*********************************************************
  指定Socket退出多播组

  返回值 :所用设置成功返回GF_RETURN_OK, 否则返回GF_RETURN_FAILE
 *********************************************************/
int GF_Set_Sock_Rm_Multi_MemberShip(int fd, char *multiAddr)
{
	int    ret = 0;
	struct ip_mreq	ipmreq;

	memset(&ipmreq, 0, sizeof(ipmreq));
	//add multicast membership
	ipmreq.imr_multiaddr.s_addr = inet_addr(multiAddr);
	ipmreq.imr_interface.s_addr = htonl(0);

	ret = setsockopt(fd ,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char *)&ipmreq,
			sizeof(ipmreq));
	return ret;
}
/*********************************************
  建立一个tcp服务器

host:服务器的域名,本机可填NULL
serv:服务器提供的服务,可填绑定的端口
addrlenp: 返回sock地址结构的大小

返回值:成功返回使用的SOCKET，否则返回GF_RETURN_FAIL
 **********************************************/		  
int Listen(int fd, int backlog)
{
	char	*ptr = NULL;

	if (fd <= 0)
		return GF_RETURN_FAIL;

	/*4can override 2nd argument with environment variable */
	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0)
		return GF_RETURN_FAIL;
	return GF_RETURN_OK;
}

int GF_Tcp_Listen(const char *host, const char *serv, int *addrlenp)
{
	int      listenfd, n;
	struct addrinfo hints, *res, *resSave;

	bzero(&hints, sizeof (struct addrinfo)) ;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	//host为空，即获取本机的地址信息
	if ( (n = getaddrinfo (host, serv, &hints, &res)) != 0)
		return GF_RETURN_FAIL;

	resSave = res;//本机地址队列
	do {
		//建立流套接字TCP
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
		{
			sleep(3);	
			continue;            /* error, try next one */
		}

		GF_Set_Sock_Attr(listenfd, 1, 0, 0, 0, 0);
		//将该tcp绑定到指定的本机地址及端口(5000)
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;               /* success */
		close (listenfd);        /* bind error, close and try next one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL)            /* errno from final socket () or bind () */
	{
		if (resSave)
		{
			freeaddrinfo (resSave);
			resSave = NULL;
		}
		return GF_RETURN_FAIL;
	}

	// 建立TCPsocket侦听队列，队列长度为1024
	Listen (listenfd, LISTENQ);

	if (addrlenp)
		*addrlenp = res->ai_addrlen;     /* return size of protocol address */
	if (resSave)
		freeaddrinfo (resSave);

	return (listenfd);
}
/************************************************
  设置socket为阻塞socket

nSock: 所要设置的socket

返回值: 成功时返回GF_RETURN_OK， 否则返回GF_RETURN_FAIL
 ************************************************/	   
int GF_Set_Sock_Block(int nSock)
{	
	int bBlock = 0;
	if (nSock <= 0)
		return GF_RETURN_FAIL;

	if (ioctl(nSock, FIONBIO, &bBlock) < 0 )
		return GF_RETURN_FAIL;
	return GF_RETURN_OK;
}
/*******************************************************
  设置socket为非阻塞socket

nSock: 所要设置的socket

返回值: 成功时返回GF_RETURN_OK， 否则返回GF_RETURN_FAIL
 ********************************************************/	   
int GF_Set_Sock_NoBlock(int nSock)
{	
	int bNoBlock = 1;

	if (nSock <= 0)
		return GF_RETURN_FAIL;

	if (ioctl(nSock, FIONBIO, &bNoBlock) < 0 )
		return GF_RETURN_FAIL;
	return GF_RETURN_OK;
}

/********************************************************
  tcp阻塞方式连接服务端

localHost:本地端的域名或者ip地址
localServ:本地端的服务名称或者是端口

host: 服务端的域名或者ip地址
serv: 服务端的服务名称或者是端口

返回值：连接成功返回连接使用的socket, 否则返回GF_RETURN_FAIL
 **********************************************************/	  
int GF_Tcp_Block_Connect (const char *localHost, const char *localServ, const char *dstHost, const char *dstServ)
{
	int     sockfd, n;
	struct addrinfo hints, *dstRes, *localRes, *resSave;

	dstRes = localRes = resSave = NULL;

	if (dstHost == NULL || dstServ == NULL)
		return GF_RETURN_FAIL;	
	bzero(&hints, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo (dstHost, dstServ, &hints, &dstRes)) != 0)
		return GF_RETURN_FAIL;
	resSave = dstRes;

	do {
		sockfd = socket (dstRes->ai_family, dstRes->ai_socktype, dstRes->ai_protocol);
		if (sockfd < 0)
		{
			sleep(1);
			continue;            /*ignore this one */
		}   

		if (localServ)
		{
			if ( (n = getaddrinfo (localHost, localServ, &hints, &localRes)) != 0)
			{
				if (resSave)
				{
					freeaddrinfo(resSave);
					resSave = NULL;
				}
				return GF_RETURN_FAIL;
			}
			GF_Set_Sock_Attr(sockfd, 1, 0, 0, 0, 0);  //set the sock reuser_addr attribute

			if (bind(sockfd, localRes->ai_addr, localRes->ai_addrlen) == 0)
				break;               /* success */
		}

		if (connect (sockfd, dstRes->ai_addr, dstRes->ai_addrlen) == 0)
			break;               /* success */

		close(sockfd);          /* ignore this one */
	} while ( (dstRes = dstRes->ai_next) != NULL);

	if (dstRes == NULL)             /* errno set from final connect() */
		sockfd =  GF_RETURN_FAIL;
	if (resSave)
	{
		freeaddrinfo(resSave);
		resSave = NULL;
	}

	if (localRes)
	{
		freeaddrinfo(localRes);
		localRes = NULL;
	}

	return (sockfd);
}

/*************************************************
  tcp阻塞socket接收函数

sockfd:接收的socket
rcvBuf:接收的缓冲区
bufSize:接收的缓冲区大小
rcvSize:指定接收的字节数, 填0不指定接收的字节数

返回值：连接成功返回接收到的字节数,否则返回GF_RETURN_FAIL
 **************************************************/
int GF_Tcp_Block_Recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize)
{
	int nleft;
	int nread;
	int nTryTimes = 0;
	char *ptr;

	ptr  = rcvBuf;
	nleft= rcvSize;

	if  (sockfd <= 0 || rcvBuf == NULL || bufSize <= 0)
		return GF_RETURN_FAIL; 

	if (rcvSize <= 0)
		nleft = bufSize;

	while(nleft > 0 && nTryTimes < GF_MAX_BLOCK_RECV_TIME) 
	{
		if ((nread = recv(sockfd, ptr, nleft, 0)) < 0)
		{
			if (errno == EINTR)
				nread = 0;
			else
				return GF_RETURN_FAIL;    	
		}
		else if (nread == 0)
		{
			break;
		}

		nleft -= nread;
		ptr   += nread;
		++nTryTimes;
		if (rcvSize <= 0 && nread > 0)
			break;
	}

	if (rcvSize <= 0)
		return nread;
	return  rcvSize - nleft;   	     	
}

/*****************************************
  tcp阻塞方式发送

fd:所要发送数据的socket
vptr:所要发送的数据
n: 所要发送的数据的字节数

返回值:返回发送的字节数,否则返回GF_RETURN_FAIL
 *****************************************/   
int GF_Tcp_Block_Send(int fd, const void *vptr, int n)
{
	int nleft;
	int nwritten;
	int nTryTimes = 0;
	const char *ptr;

	if  (fd <= 0 || vptr == NULL || n <= 0)
		return GF_RETURN_FAIL; 

	ptr = vptr;
	nleft = n;
	while (nleft > 0 && nTryTimes < GF_MAX_BLOCK_SEND_TIME)
	{
		if ( (nwritten = send(fd, ptr, nleft, 0)) <= 0) 
		{
			if (nwritten < 0 && errno == EINTR)
			{
				nwritten = 0;   /* and call write() again */	
				nTryTimes = -1;
			}
			else
				return (GF_RETURN_FAIL);    /* error */
		}

		nleft -= nwritten;
		ptr += nwritten;
		++nTryTimes;
	}
	return (n - nleft);
}
/*****************************************************************************
  tcp 阻塞方式accept

  fd[in]: 监听的socket
  sa[out]: 对端的socket地址结构
  salenptr[in/out]:输入为struct sockaddr 的结构大小, 输出为对端的socket地址结构

  返回值:成功返回所用的socket, 否则返回GF_RETURN_FAIL
 *****************************************************************************/
int GF_Tcp_Block_Accept(int fd, struct sockaddr *sa, int *salenptr)
{
	int n = GF_RETURN_FAIL;
	char szIp[32] = {0};
again:
	if ( (n = accept(fd, sa, salenptr)) < 0) 
	{
		if (errno == ECONNABORTED)
			goto again;
	}
	if (sa != NULL)
		printf("RECV REMOTE CONNECT %s\n", 
				inet_ntop(AF_INET, &((struct   sockaddr_in*)sa)->sin_addr, szIp, sizeof(szIp)));
	return (n);
}
/*********************************************
  tcp非阻塞方式连接服务端

localHost:本地端的域名或者ip地址
localServ:本地端的服务名称或者是端口

dstHost: 服务端的域名或者ip地址
dstServ: 服务端的服务名称或者是端口
timeout_ms: 连接服务器超时时间(单位毫秒)

返回值：连接成功返回连接使用的socket, 否则返回GF_RETURN_FAIL
 ***********************************************/			
int GF_Tcp_NoBlock_Connect(const char *localHost, const char *localServ, const char *dstHost, const char *dstServ, int timeout_ms)
{
	int     sockfd, n;
	struct addrinfo hints, *dstRes, *localRes, *resSave;
	int	    error = 1;
	int	    error_len = sizeof(int);
	fd_set  fset;
	int     ret = 0;
	int     opt = 0;

	struct timeval to = {0};

	dstRes = resSave = localRes = NULL;

	if (dstHost == NULL || dstServ == NULL)
		return GF_RETURN_FAIL;	

	bzero(&hints, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	to.tv_sec = timeout_ms / 1000;
	to.tv_usec= (timeout_ms % 1000)*1000;	
	if ( (n = getaddrinfo (dstHost, dstServ, &hints, &dstRes)) != 0)
		return GF_RETURN_FAIL;
	resSave = dstRes;

	do {
		sockfd = socket (dstRes->ai_family, dstRes->ai_socktype, dstRes->ai_protocol);
		if (sockfd < 0)
		{
			sleep(3);     
			continue;            /*ignore this one */
		}   

		GF_Set_Sock_NoBlock(sockfd);
		opt = 1;
		setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&opt,sizeof(opt));

		if (localServ)
		{
			if ( (n = getaddrinfo (localHost, localServ, &hints, &localRes)) != 0)
			{
				if (resSave)
				{
					freeaddrinfo (resSave);
					resSave = NULL;
				}
				close(sockfd); 
				return GF_RETURN_FAIL;
			}
			GF_Set_Sock_Attr(sockfd, 1, 0, 0, 0, 0);  //set the sock reuser_addr attribute

			if (bind(sockfd, localRes->ai_addr, localRes->ai_addrlen) == 0)
				break;               /* success */		
		}
		ret = connect (sockfd, dstRes->ai_addr, dstRes->ai_addrlen);
		if (ret == 0)
			break;	
		if (ret < 0 && errno == EINPROGRESS)
		{
			FD_ZERO(&fset);
			FD_SET(sockfd,&fset);

			if ((ret = select(sockfd + 1, NULL, &fset, NULL, &to)) > 0)
			{
				if (FD_ISSET(sockfd, &fset) > 0)
				{
					if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&error, &error_len) < 0)
						error = 1;
				}
			}
			if (error == 0)
				break;
		}

		close(sockfd);          /* ignore this one */
	} while ( (dstRes = dstRes->ai_next) != NULL);
	if (dstRes == NULL)             /* errno set from final connect() */
		sockfd =  GF_RETURN_FAIL;

	if (resSave)
	{
		freeaddrinfo (resSave);
		resSave = NULL;
	}
	if (localRes)
	{
		freeaddrinfo (localRes);
		resSave = NULL;
	}

	return (sockfd);
}

/*******************************************************
  tcp非阻塞socket接收函数

sockfd:接收的socket
rcvBuf:接收的缓冲区
bufSize:接收的缓冲区大小
rcvSize:制定接收的字节数, 填0不指定接收的字节数

返回值:成功返回接收到的字节数, 否则返回GF_RETURN_FAIL
 *******************************************************/
int GF_Tcp_NoBlock_Recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize)
{
	int		ret;
	unsigned long	dwRecved = 0;
	unsigned long	nTryTime = 0;
	int                 nSize    = rcvSize;

	if (sockfd <= 0 || rcvBuf == NULL || bufSize <= 0)
		return -1;

	if (rcvSize <= 0)
		nSize = bufSize;	           

	while(dwRecved < nSize)
	{
		ret = recv(sockfd, rcvBuf + dwRecved, nSize - dwRecved, 0);
		if(0 == ret)
			return -1;
		else if(ret < 1)
		{
			if(ECONNRESET == errno)
				return -1;
			else if(EWOULDBLOCK == errno  || errno == EINTR || errno == EAGAIN)
			{
				if(nTryTime++ < GF_MAX_NOBLOCK_RECV_TIME)
				{				
					usleep(10000);
					continue;
				}
				else
					break;
			}

			return -1;
		}
		nTryTime = 0;
		dwRecved += ret;
		if (rcvSize <= 0) //没有指定收数据的长度,则收一次就返回收到的长度
			break;
	}
	return dwRecved;
}


/*******************************************************
  tcp非阻塞方式发送

hSock:所要发送数据的socket
pbuf:所要发送的数据
size: 所要发送的数据的字节数
pBlock: 判读网路是不是断开 (连续发GF_MAX_NOBLOCK_SEND_TIME次不成功则认为网络断开)
返回值:成功返回发送的字符数, 否则返回GF_RETURN_FAIL
 *******************************************************/	
int GF_Tcp_NoBlock_Send(int hSock,char *pbuf,int size, int *pBlock)
{
	int  block = 0;	
	int  alllen = size;
	int  sended = 0;

	if (hSock < 0 || pbuf == NULL || size <= 0)
		return 0;

	if (pBlock != NULL)
		*pBlock = 0;

	while(alllen > 0)
	{	
		sended = send(hSock,pbuf,alllen,0);
		if(0 == sended)
		{
			printf("GF_Tcp_NoBlock_Send send ret 0\n");
			return GF_RETURN_FAIL;
		}
		else if(sended < 1)
		{
			if(block > GF_MAX_NOBLOCK_SEND_TIME)
			{
				return GF_RETURN_FAIL;
			}
			if(errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)
			{			
				if(block++ < GF_MAX_NOBLOCK_SEND_TIME)
				{
					usleep(1000);
					continue;
				}
				else
				{
					if (pBlock != NULL)	
						*pBlock = 1;
					break;
				}
			}
			printf("GF_Tcp_NoBlock_Send send err\n");			
			return GF_RETURN_FAIL;
		}
		else
		{	
			if (pBlock != NULL)	
				*pBlock = 0;
			pbuf += sended;
			alllen -= sended;
		}
	}

	if(alllen > 0)
		return GF_RETURN_FAIL;
	return size;
}
/*******************************************************
  tcp非阻塞方式发送

hSock:所要发送数据的socket
pbuf:所要发送的数据
size: 所要发送的数据的字节数
mtu:  每次发送的最大字节数
返回值:成功返回发送的字符数, 否则返回GF_RETURN_FAIL
 *******************************************************/	
int GF_Tcp_NoBlock_MTU_Send(int hSock,char *pbuf,int size, int mtu)
{
	int ret       = size;
	int sendLen   = 0;
	int nBlock    = 0;

	if (hSock <= 0 || pbuf == NULL || size <= 0)
		return GF_RETURN_FAIL;


	while(size > 0)
	{
		sendLen = size > mtu ? mtu : size;
		if (sendLen != GF_Tcp_NoBlock_Send(hSock, pbuf, sendLen, &nBlock))
			return GF_RETURN_FAIL;

		pbuf += sendLen;
		size -= sendLen;
	}

	return ret;
}
/****************************************************************************
  tcp 非阻塞方式accept

  fd[in]: 监听的socket
  sa[out]: 对端的socket地址结构
  salenptr[in/out]:输入为struct sockaddr 的结构大小, 输出为对端的socket地址结构
to:      accept超时结构

返回值:成功返回连接所用的socket，否则返回GF_RETURN_FAIL
 ****************************************************************************/		 		 
int GF_Tcp_NoBlock_Accept(int fd, struct sockaddr *sa, int *salenptr, struct timeval * to)
{
	int nready = 0;
	int maxfd = fd+1;
	fd_set rset;

	if (fd <= 0 || to == NULL)
		return GF_RETURN_FAIL;

	FD_ZERO(&rset);
	FD_SET(fd, &rset);

	nready = select(maxfd, &rset, NULL, NULL, to);
	if (nready > 0 && FD_ISSET(fd, &rset))
	{
		return GF_Tcp_Block_Accept(fd, sa, salenptr);
	}

	return GF_RETURN_FAIL;
}

/******************************************************
  udp非阻塞socket接收函数

sockfd:接收的socket
rcvBuf:接收的缓冲区
bufSize:接收的缓冲区大小
rcvSize:制定接收的字节数, 填0不指定接收的字节数
from:获取发送端的socket结构
fromlen:获取发送端的socket结构长度

返回值:成功返回接收到的字节数, 否则返回GF_RETURN_FAIL
 *******************************************************/
int GF_Udp_Recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize, struct sockaddr *from, int *fromlen)
{
	int		ret;
	//实际接收的字节数
	unsigned long	dwRecved = 0;
	unsigned long	nTryTime = 0;
	int             nSize    = rcvSize;

	if (rcvSize <= 0 || rcvBuf == NULL || bufSize <= 0)
		nSize = bufSize;	           

	while(dwRecved < nSize)
	{
		ret = recvfrom(sockfd, rcvBuf + dwRecved, nSize - dwRecved, 0, from, fromlen);
		if(0 == ret)//超时
			return GF_RETURN_FAIL;
		else if(ret < 1)
		{
			if(ECONNRESET == errno)
				return GF_RETURN_FAIL;
			else if(EWOULDBLOCK == errno  || errno == EINTR || errno == EAGAIN)
			{//接收缓冲区数据已经取完
				if(nTryTime++ < GF_MAX_BLOCK_RECV_TIME)
				{				
					usleep(10000);
					continue;
				}
				else
					break;
			}

			return GF_RETURN_FAIL;
		}
		nTryTime = 0;
		dwRecved += ret;

		if (rcvSize <= 0)
			break;
	}
	// 返回实际收到的字节数
	return dwRecved;
}

/*************************************************
  udp非阻塞方式发送

hSock:所要发送数据的socket
pbuf:所要发送的数据
size: 所要发送的数据的字节数
distAddr:目的地址

返回值:成功返回发送的字符数, 否则返回GF_RETURN_FAIL
 **************************************************/		 
int GF_Udp_Send(int hSock,char *pbuf,int size, struct sockaddr * distAddr)
{
	int  block  = 0;	
	int  alllen = size;
	int  sended = 0;

	if (hSock <= 0 || pbuf == NULL || size <= 0 || distAddr == NULL)
	{
		printf("GF_Udp_Send %d %d\n", hSock, size);
		return 0;
	}

	while(alllen > 0)
	{	
		sended = sendto(hSock,pbuf,alllen,0, distAddr, sizeof(struct sockaddr));
		if(0 == sended)
		{
			return GF_RETURN_FAIL;
		}
		else if(sended < 1)
		{
			if(block > GF_MAX_NOBLOCK_SEND_TIME)
			{
				return GF_RETURN_FAIL;
			}
			if(errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)
			{			
				if(block++ < GF_MAX_BLOCK_SEND_TIME)
				{
					usleep(1000);
					continue;
				}
				else
				{
					break;
				}
			}
			return GF_RETURN_FAIL;
		}
		else
		{		
			pbuf += sended;
			alllen -= sended;
		}
	}

	if(alllen > 0)
		return GF_RETURN_FAIL;
	return size;
}
/**********************************************************
  ping 功能实现函数 

ips:目的端ip addr
timeout: 超时时间(ms)
max_times:连续尝试次数

返回值:成功返回GF_RETURN_OK, 否则返回GF_RETURN_FAIL
 **********************************************************/
unsigned short cal_chksum(unsigned short *addr, int len)
{
	int leave = len;
	int sum   = 0;
	unsigned short *w=addr;
	unsigned short answer=0;

	while(leave > 1)
	{
		sum += *w++;
		leave -= 2;
	}

	if( leave == 1)
	{       
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;

	return answer;
}

int GF_Ping(char *ips, int timeout, int max_times)
{
	struct timeval timeo;
	int sockfd;
	struct sockaddr_in addr;
	struct sockaddr_in from;

	struct timeval *tval;
	struct ip 		 *iph;
	struct icmp 	 *icmp;

	char sendpacket[128];
	char recvpacket[128];
	char from_ip[32];
	int n;
	int ping_times = 0;
	int ret = 0;
	pid_t pid;
	int maxfds = 0;
	fd_set readfds;

	if (ips == NULL || strcmp(ips, "") == 0)
		return 0;
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ips, &addr.sin_addr);  

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		return GF_RETURN_FAIL;
	}

	timeo.tv_sec = timeout / 1000000;
	timeo.tv_usec = (timeout % 1000);

	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo)) == -1)
	{
		close(sockfd);
		sleep(5);
		return GF_RETURN_FAIL;
	}

	memset(sendpacket, 0, sizeof(sendpacket));

	pid=getpid();
	int packsize = 0;

	icmp=(struct icmp*)sendpacket;
	icmp->icmp_type=ICMP_ECHO;
	icmp->icmp_code=0;
	icmp->icmp_cksum=0;
	icmp->icmp_seq=0;
	icmp->icmp_id=pid;
	packsize=8+56;
	tval= (struct timeval *)icmp->icmp_data;
	gettimeofday(tval,NULL);
	icmp->icmp_cksum=cal_chksum((unsigned short *)icmp,packsize);

	n = sendto(sockfd, (char *)&sendpacket, packsize, 0, (struct sockaddr *)&addr, sizeof(addr));
	if (n < 1)
	{
		close(sockfd);
		sleep(5);
		return GF_RETURN_FAIL;
	}

	while(ping_times++ < max_times)
	{	
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		maxfds = sockfd + 1;
		n = select(maxfds, &readfds, NULL, NULL, &timeo);
		if (n <= 0)
		{
			ret = GF_RETURN_FAIL;
			continue;
		}

		memset(recvpacket, 0, sizeof(recvpacket));
		int fromlen = sizeof(from);
		n = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, (struct sockaddr *)&from, &fromlen);
		if (n < 1) 
		{
			ret = GF_RETURN_FAIL;
			continue;
		}

		inet_ntop(AF_INET, &from.sin_addr, from_ip, sizeof(from_ip));
		if (strcmp(from_ip,ips) != 0)
		{
			ret = GF_RETURN_FAIL;
			continue;
		}

		iph = (struct ip *)recvpacket;

		icmp=(struct icmp *)(recvpacket + (iph->ip_hl<<2));

		if (icmp->icmp_type == ICMP_ECHOREPLY && icmp->icmp_id == pid)
		{
			ret = GF_RETURN_OK;
			break;
		}  
		else
		{
			ret = GF_RETURN_FAIL;
			continue;
		}
	}

	close(sockfd);
	return ret;
}
/****************************************************
  取得给出的host的ip地址(返回的第一个地址)

af: host 使用的协议族
host:host 名

返回值：成功返回host的ip值(网络序),否则返回GF_RETURN_FAIL
 ****************************************************/	 
int GF_Get_Host_Ip(int af, char *host)
{
	unsigned long dwIp = 0;
	int           ret  = 0;
	struct addrinfo hints, *res;

	if(host == NULL)
		return GF_RETURN_FAIL;
	if(strcmp(host,"255.255.255.255") == 0 || strcmp(host, "") == 0)
		return GF_RETURN_FAIL;

	bzero (&hints, sizeof (struct addrinfo));
	hints.ai_flags  = AI_CANONNAME;   /* always return canonical name */
	hints.ai_family = AF_UNSPEC;   /* AF_UNSPEC, AF_INET, AF_INET6, etc. */
	hints.ai_socktype = 0;  /* 0, SOCK_STREAM, SOCK_DGRAM, etc. */
	if ( (ret = getaddrinfo(host, NULL, &hints, &res)) != 0)
		return GF_RETURN_FAIL;
	dwIp =  ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;	

	freeaddrinfo(res);

	return dwIp;
}
/****************************************************
  监听fd_array数组中的socket

fd_array: socket 数组
fd_num:   socket 的个数
fd_type:  bit 1 为 1 代表监听socket 是否可读
bit 2 为 1 代表监听socket 是否可写
bit 3 为 1 代表监听socket 是否有异常

返回值：超时返回GF_RETURN_OK,出错返回GF_RETURN_FAIL，否则
如果有可读的返回该socket值|0x10000
有可写返回该socket值|0x20000
有异常返回该socket值|0x40000
 ****************************************************/	
int GF_Select(int *fd_array, int fd_num, int fd_type, int time_out)
{
	int maxfd = 0;
	int index = 0;
	int ret   = 0;

	fd_set read_fd, *p_read_fd;
	fd_set write_fd, *p_write_fd;
	fd_set except_fd, *p_except_fd;
	struct timeval timeO, *p_time_out;

	if (fd_array == NULL || fd_num < 0)
		return GF_RETURN_FAIL;

	p_time_out = NULL;
	if (time_out > 0)
	{
		timeO.tv_sec = time_out / 1000;
		timeO.tv_usec= (time_out % 1000)*1000;	
		p_time_out = &timeO;
	}

	p_read_fd = p_write_fd = p_except_fd = NULL;

	if (fd_type & 0x1)
	{
		p_read_fd = &read_fd;
		FD_ZERO(p_read_fd);
	}

	if (fd_type & 0x2)
	{
		p_write_fd = &write_fd;
		FD_ZERO(p_write_fd);
	}

	if (fd_type & 0x4)
	{
		p_except_fd = &except_fd;
		FD_ZERO(p_except_fd);
	}

	for (index = 0; index < fd_num; ++index)
	{
		if(fd_array[index] <= 0)
			continue;
		maxfd = maxfd > fd_array[index] ? maxfd : fd_array[index];

		if (p_read_fd)
			FD_SET(fd_array[index], p_read_fd);
		if (p_write_fd)
			FD_SET(fd_array[index], p_write_fd);
		if (p_except_fd)
			FD_SET(fd_array[index], p_except_fd);
	}

	if (maxfd <= 0)
		return GF_RETURN_FAIL;

	maxfd += 1;

	while (1)
	{
		ret = select(maxfd, p_read_fd, p_write_fd, p_except_fd, p_time_out);
		if (ret < 0 && errno == EINTR)
			continue;
		else if (ret < 0)
			return GF_RETURN_FAIL;
		else if (ret == 0)
			return GF_RETURN_OK;
		else
		{
			for (index = 0; index < fd_num; ++index)
			{
				if (fd_array[index] <= 0)  //socket error
					continue;

				if (p_read_fd)
				{
					if (FD_ISSET(fd_array[index], p_read_fd))
						return fd_array[index] | 0x10000;
				}
				else if (p_write_fd)
				{
					if (FD_ISSET(fd_array[index], p_write_fd))
						return fd_array[index] | 0x20000;
				}
				else if (p_except_fd)
				{
					if (FD_ISSET(fd_array[index], p_except_fd))
						return fd_array[index] | 0x40000;;
				}			
			}
			return GF_RETURN_OK;
		}
	}	
	return GF_RETURN_FAIL;	
}
/****************************************************
  关闭指定的socket

GFd : 所要关闭的socket 	
返回值：超时返回GF_RETURN_OK,出错返回GF_RETURN_FAIL
 ****************************************************/	

int GF_Close_Socket(int *pSock)
{
	if (pSock != NULL && *pSock > 0)
	{
		//		shutdown(*pSock, 2); //ljm close 2010-07-15
		close(*pSock);
		*pSock = -1;
		return GF_RETURN_OK;
	}
	return GF_RETURN_FAIL;	
}
/****************************************************
 *建立一个socket
 *
 *iType : 建立的socket的类型 1:tcp 2:udp
 *
 *返回值：成功返回socket ,出错返回GF_RETURN_FAIL
 ****************************************************/	
int GF_Create_Sock(int iType)
{
	int fd = 0;
	fd = socket(AF_INET,iType == 1 ? SOCK_STREAM : SOCK_DGRAM,0);
	return fd;
}
/****************************************************
 *给socket绑定一个制定的端口
 *
 *sockfd : 需绑定的一个socket
 *
 *nport:   绑定的端口
 *
 *返回值：成功返回socket ,出错返回GF_RETURN_FAIL
 ****************************************************/	
int GF_Bind_Sock(int sockfd, int ip, int nPort)
{
	struct sockaddr_in addr_local;
	memset(&addr_local, 0, sizeof(struct sockaddr_in));

	addr_local.sin_family      = AF_INET;
	addr_local.sin_addr.s_addr = htonl(ip);
	addr_local.sin_port        = htons(nPort);	

	GF_Set_Sock_Attr(sockfd, 1, 0, 0, 0, 0);  //set the sock reuser_addr attribute
	if (bind(sockfd,(struct sockaddr*)&addr_local, sizeof(struct sockaddr_in)) < 0)
		return GF_RETURN_FAIL;
	return GF_RETURN_OK;
}
/****************************************************
 *给socket设置SO_LINGER属性
 *
 *sockfd : socket
 *
 *
 *返回值：成功返回GF_RETURN_OK ,出错返回GF_RETURN_FAIL
 ****************************************************/	
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
int GF_Sock_Set_LINGER(int sockfd)
{
	struct linger nLinger;

	if (sockfd <= 0)
		return GF_RETURN_FAIL;

	memset(&nLinger, 0, sizeof(struct linger));
	nLinger.l_onoff = 1;
	nLinger.l_linger= 0;

	if (setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(void *)&nLinger,sizeof(struct linger)) < 0)
		return GF_RETURN_FAIL;
	return GF_RETURN_OK;
}

int GF_Sock_Unset_LINGER(int sockfd)
{
	struct linger nLinger;

	if (sockfd <= 0)
		return GF_RETURN_FAIL;

	memset(&nLinger, 0, sizeof(struct linger));
	nLinger.l_onoff = 0;
	nLinger.l_linger= 0;

	if (setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(void *)&nLinger,sizeof(struct linger)) < 0)
		return GF_RETURN_FAIL;
	return GF_RETURN_OK;
}

int GF_Sock_Bind_Interface(int sockfd, char *interfaceName)
{
	struct ifreq ifReq;
	int ret = 0;

	strncpy(ifReq.ifr_name, interfaceName, IFNAMSIZ);

	ret = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,
			(char *)&ifReq, sizeof(ifReq));
	if (ret < 0)
	{
		printf("GF_Sock_Bind_Interface bind to %s failed %s\n", 
				interfaceName, strerror(errno));
	}
	else
	{
		printf("GF_Sock_Bind_Interface bind to %s ok\n", interfaceName);
	}
	return ret;
}
/****************************************************
 *GF_get_sock_ip 取得socket绑定ip
 *
 *sock:套接字
 *
 *返回值：成功返回ip ,出错返回GF_RETURN_FAIL
 ****************************************************/	
int GF_Get_Sock_Ip(int sock)
{
	struct sockaddr_in sockAddr;
	int addrLen = sizeof(struct sockaddr);

	if (0 != getsockname(sock, (struct sockaddr *)&sockAddr, &addrLen))
		return GF_RETURN_FAIL;
	return sockAddr.sin_addr.s_addr;
}
/****************************************************
 *GF_get_sock_port 取得socket绑定port
 *
 *sock:套接字
 *
 *返回值：成功返回port ,出错返回GF_RETURN_FAIL
 ****************************************************/	
int GF_Get_Sock_Port(int sock)
{
	struct sockaddr_in sockAddr;
	int addrLen = sizeof(struct sockaddr);

	if (0 != getsockname(sock, (struct sockaddr *)&sockAddr, &addrLen))
		return GF_RETURN_FAIL;
	return htons(sockAddr.sin_port);
}

unsigned long GF_Ip_N2A(unsigned long ip, char *ourIp, int len)
{
	if(!ourIp)
		return GF_RETURN_FAIL;

	memset(ourIp, 0, len);
	ip = ntohl(ip);
	inet_ntop(AF_INET, &ip, ourIp, len);	
	return GF_RETURN_OK;
}

unsigned long  GF_Ip_A2N(char *szIp)
{
	int nIp = 0;
	if(!szIp)
		return 0xFFFFFFFF;
	inet_pton(AF_INET, szIp,&nIp);
	return ntohl(nIp);
}

unsigned long GF_Ip_Ext_A2N(char *pstrIP)
{
	unsigned long ret;
	struct hostent *hostinfo;
	struct sockaddr_in address;

	ret = htonl(GF_Ip_A2N(pstrIP));

	if(0xFFFFFFFF == ret)
	{
		if(strcmp(pstrIP,"255.255.255.255") == 0)
		{
			return 0xFFFFFFFF;
		}

		if(!(hostinfo = gethostbyname(pstrIP)))
		{
			return 0;
		}

		address.sin_addr = *((struct in_addr *)(*(hostinfo->h_addr_list)));
		ret = address.sin_addr.s_addr;
	}

	return ret;
}

void  reverse(char   s[])   
{   
	int   c,   i,   j;   

	for   (i   =   0,   j   =   strlen(s)-1;   i   <   j;   i++,   j--)
	{   
		c   =   s[i];   
		s[i]   =   s[j];   
		s[j]   =   c;   
	}   
}   
/*********************************************************
 *GF_itoa 跟windows 上itoa一样
 *
 *n:数值
 *
 *s:字串
 *
 *返回值:
 *********************************************************/
void   GF_itoa(int   n,   char   s[])   
{   
	int   i,   sign;   

	if ((sign = n) < 0)     /*   record   sign   */   
		n = -n;                     /*   make   n   positive   */   
	i = 0;   
	do{             /*   generate   digits   in   reverse   order   */   
		s[i++] = n % 10 + '0';     /*   get   next   digit   */   
	}while ((n /= 10) > 0);         /*   delete   it   */   

	if (sign < 0)   
		s[i++] = '-';   
	s[i] = '\0';   
	reverse(s);   
}

