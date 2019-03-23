/**************************************************************
 *GFNetFun.c
 *
 *����:ʵ��һЩ���糣�ú����ķ�װ
 *
 *����:ljm
 *
 *����ʱ��:2009-02-12
 *
 *
 *�޸���־:
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
  ����socket��һЩ��������

fd:��Ҫ���õ�socket
bReuseAddr:�������°�
nSndTimeO:����socket�ķ��ͳ�ʱʱ��(��λ����), ��0�����ø���
nRcvTimeO:����socket�ķ��ͳ�ʱʱ��(��λ����), ��0�����ø���
nSndBuf:����socket�ķ��ͻ������Ĵ�С, ��0�����ø���
nRcvBuf:����socket�Ľ��ջ������Ĵ�С, ��0�����ø���

����ֵ :�������óɹ�����GF_RETURN_OK, ���򷵻�GF_RETURN_FAILE
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
  ָ����Socket ��ʹ��Nagle �㷨

fd:��Ҫ���õ�socket

����ֵ :�������óɹ�����GF_RETURN_OK, ���򷵻�GF_RETURN_FAILE
 *********************************************************/
int GF_Set_Sock_NoDelay(int fd)
{
	int opt = 1;	

	if (fd <= 0)
		return GF_RETURN_FAIL;
	return setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&opt,sizeof(opt));
}
/*********************************************************
  ָ��Socket keepalive ����

  ����ֵ :�������óɹ�����GF_RETURN_OK, ���򷵻�GF_RETURN_FAILE
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
  ָ��Socket�㲥����

  ����ֵ :�������óɹ�����GF_RETURN_OK, ���򷵻�GF_RETURN_FAILE
 *********************************************************/
int  GF_Set_Sock_BoardCast(int fd)
{
	int    so_boardcast = 1;
	return setsockopt(fd,SOL_SOCKET,SO_BROADCAST,&so_boardcast,sizeof(so_boardcast));
}
/*********************************************************
  ָ��Socket�ಥ����

  ����ֵ :�������óɹ�����GF_RETURN_OK, ���򷵻�GF_RETURN_FAILE
 *********************************************************/
int  GF_Set_Sock_MultiCast(int fd)
{
	int opt=0;
	return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&opt,  sizeof(char));
}
/*********************************************************
  ָ��Socket����ಥ��

  ����ֵ :�������óɹ�����GF_RETURN_OK, ���򷵻�GF_RETURN_FAILE
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
  ָ��Socket�˳��ಥ��

  ����ֵ :�������óɹ�����GF_RETURN_OK, ���򷵻�GF_RETURN_FAILE
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
  ����һ��tcp������

host:������������,��������NULL
serv:�������ṩ�ķ���,����󶨵Ķ˿�
addrlenp: ����sock��ַ�ṹ�Ĵ�С

����ֵ:�ɹ�����ʹ�õ�SOCKET�����򷵻�GF_RETURN_FAIL
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

	//hostΪ�գ�����ȡ�����ĵ�ַ��Ϣ
	if ( (n = getaddrinfo (host, serv, &hints, &res)) != 0)
		return GF_RETURN_FAIL;

	resSave = res;//������ַ����
	do {
		//�������׽���TCP
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
		{
			sleep(3);	
			continue;            /* error, try next one */
		}

		GF_Set_Sock_Attr(listenfd, 1, 0, 0, 0, 0);
		//����tcp�󶨵�ָ���ı�����ַ���˿�(5000)
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

	// ����TCPsocket�������У����г���Ϊ1024
	Listen (listenfd, LISTENQ);

	if (addrlenp)
		*addrlenp = res->ai_addrlen;     /* return size of protocol address */
	if (resSave)
		freeaddrinfo (resSave);

	return (listenfd);
}
/************************************************
  ����socketΪ����socket

nSock: ��Ҫ���õ�socket

����ֵ: �ɹ�ʱ����GF_RETURN_OK�� ���򷵻�GF_RETURN_FAIL
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
  ����socketΪ������socket

nSock: ��Ҫ���õ�socket

����ֵ: �ɹ�ʱ����GF_RETURN_OK�� ���򷵻�GF_RETURN_FAIL
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
  tcp������ʽ���ӷ����

localHost:���ض˵���������ip��ַ
localServ:���ض˵ķ������ƻ����Ƕ˿�

host: ����˵���������ip��ַ
serv: ����˵ķ������ƻ����Ƕ˿�

����ֵ�����ӳɹ���������ʹ�õ�socket, ���򷵻�GF_RETURN_FAIL
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
  tcp����socket���պ���

sockfd:���յ�socket
rcvBuf:���յĻ�����
bufSize:���յĻ�������С
rcvSize:ָ�����յ��ֽ���, ��0��ָ�����յ��ֽ���

����ֵ�����ӳɹ����ؽ��յ����ֽ���,���򷵻�GF_RETURN_FAIL
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
  tcp������ʽ����

fd:��Ҫ�������ݵ�socket
vptr:��Ҫ���͵�����
n: ��Ҫ���͵����ݵ��ֽ���

����ֵ:���ط��͵��ֽ���,���򷵻�GF_RETURN_FAIL
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
  tcp ������ʽaccept

  fd[in]: ������socket
  sa[out]: �Զ˵�socket��ַ�ṹ
  salenptr[in/out]:����Ϊstruct sockaddr �Ľṹ��С, ���Ϊ�Զ˵�socket��ַ�ṹ

  ����ֵ:�ɹ��������õ�socket, ���򷵻�GF_RETURN_FAIL
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
  tcp��������ʽ���ӷ����

localHost:���ض˵���������ip��ַ
localServ:���ض˵ķ������ƻ����Ƕ˿�

dstHost: ����˵���������ip��ַ
dstServ: ����˵ķ������ƻ����Ƕ˿�
timeout_ms: ���ӷ�������ʱʱ��(��λ����)

����ֵ�����ӳɹ���������ʹ�õ�socket, ���򷵻�GF_RETURN_FAIL
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
  tcp������socket���պ���

sockfd:���յ�socket
rcvBuf:���յĻ�����
bufSize:���յĻ�������С
rcvSize:�ƶ����յ��ֽ���, ��0��ָ�����յ��ֽ���

����ֵ:�ɹ����ؽ��յ����ֽ���, ���򷵻�GF_RETURN_FAIL
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
		if (rcvSize <= 0) //û��ָ�������ݵĳ���,����һ�ξͷ����յ��ĳ���
			break;
	}
	return dwRecved;
}


/*******************************************************
  tcp��������ʽ����

hSock:��Ҫ�������ݵ�socket
pbuf:��Ҫ���͵�����
size: ��Ҫ���͵����ݵ��ֽ���
pBlock: �ж���·�ǲ��ǶϿ� (������GF_MAX_NOBLOCK_SEND_TIME�β��ɹ�����Ϊ����Ͽ�)
����ֵ:�ɹ����ط��͵��ַ���, ���򷵻�GF_RETURN_FAIL
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
  tcp��������ʽ����

hSock:��Ҫ�������ݵ�socket
pbuf:��Ҫ���͵�����
size: ��Ҫ���͵����ݵ��ֽ���
mtu:  ÿ�η��͵�����ֽ���
����ֵ:�ɹ����ط��͵��ַ���, ���򷵻�GF_RETURN_FAIL
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
  tcp ��������ʽaccept

  fd[in]: ������socket
  sa[out]: �Զ˵�socket��ַ�ṹ
  salenptr[in/out]:����Ϊstruct sockaddr �Ľṹ��С, ���Ϊ�Զ˵�socket��ַ�ṹ
to:      accept��ʱ�ṹ

����ֵ:�ɹ������������õ�socket�����򷵻�GF_RETURN_FAIL
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
  udp������socket���պ���

sockfd:���յ�socket
rcvBuf:���յĻ�����
bufSize:���յĻ�������С
rcvSize:�ƶ����յ��ֽ���, ��0��ָ�����յ��ֽ���
from:��ȡ���Ͷ˵�socket�ṹ
fromlen:��ȡ���Ͷ˵�socket�ṹ����

����ֵ:�ɹ����ؽ��յ����ֽ���, ���򷵻�GF_RETURN_FAIL
 *******************************************************/
int GF_Udp_Recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize, struct sockaddr *from, int *fromlen)
{
	int		ret;
	//ʵ�ʽ��յ��ֽ���
	unsigned long	dwRecved = 0;
	unsigned long	nTryTime = 0;
	int             nSize    = rcvSize;

	if (rcvSize <= 0 || rcvBuf == NULL || bufSize <= 0)
		nSize = bufSize;	           

	while(dwRecved < nSize)
	{
		ret = recvfrom(sockfd, rcvBuf + dwRecved, nSize - dwRecved, 0, from, fromlen);
		if(0 == ret)//��ʱ
			return GF_RETURN_FAIL;
		else if(ret < 1)
		{
			if(ECONNRESET == errno)
				return GF_RETURN_FAIL;
			else if(EWOULDBLOCK == errno  || errno == EINTR || errno == EAGAIN)
			{//���ջ����������Ѿ�ȡ��
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
	// ����ʵ���յ����ֽ���
	return dwRecved;
}

/*************************************************
  udp��������ʽ����

hSock:��Ҫ�������ݵ�socket
pbuf:��Ҫ���͵�����
size: ��Ҫ���͵����ݵ��ֽ���
distAddr:Ŀ�ĵ�ַ

����ֵ:�ɹ����ط��͵��ַ���, ���򷵻�GF_RETURN_FAIL
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
  ping ����ʵ�ֺ��� 

ips:Ŀ�Ķ�ip addr
timeout: ��ʱʱ��(ms)
max_times:�������Դ���

����ֵ:�ɹ�����GF_RETURN_OK, ���򷵻�GF_RETURN_FAIL
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
  ȡ�ø�����host��ip��ַ(���صĵ�һ����ַ)

af: host ʹ�õ�Э����
host:host ��

����ֵ���ɹ�����host��ipֵ(������),���򷵻�GF_RETURN_FAIL
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
  ����fd_array�����е�socket

fd_array: socket ����
fd_num:   socket �ĸ���
fd_type:  bit 1 Ϊ 1 �������socket �Ƿ�ɶ�
bit 2 Ϊ 1 �������socket �Ƿ��д
bit 3 Ϊ 1 �������socket �Ƿ����쳣

����ֵ����ʱ����GF_RETURN_OK,������GF_RETURN_FAIL������
����пɶ��ķ��ظ�socketֵ|0x10000
�п�д���ظ�socketֵ|0x20000
���쳣���ظ�socketֵ|0x40000
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
  �ر�ָ����socket

GFd : ��Ҫ�رյ�socket 	
����ֵ����ʱ����GF_RETURN_OK,������GF_RETURN_FAIL
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
 *����һ��socket
 *
 *iType : ������socket������ 1:tcp 2:udp
 *
 *����ֵ���ɹ�����socket ,������GF_RETURN_FAIL
 ****************************************************/	
int GF_Create_Sock(int iType)
{
	int fd = 0;
	fd = socket(AF_INET,iType == 1 ? SOCK_STREAM : SOCK_DGRAM,0);
	return fd;
}
/****************************************************
 *��socket��һ���ƶ��Ķ˿�
 *
 *sockfd : ��󶨵�һ��socket
 *
 *nport:   �󶨵Ķ˿�
 *
 *����ֵ���ɹ�����socket ,������GF_RETURN_FAIL
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
 *��socket����SO_LINGER����
 *
 *sockfd : socket
 *
 *
 *����ֵ���ɹ�����GF_RETURN_OK ,������GF_RETURN_FAIL
 ****************************************************/	
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
 *GF_get_sock_ip ȡ��socket��ip
 *
 *sock:�׽���
 *
 *����ֵ���ɹ�����ip ,������GF_RETURN_FAIL
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
 *GF_get_sock_port ȡ��socket��port
 *
 *sock:�׽���
 *
 *����ֵ���ɹ�����port ,������GF_RETURN_FAIL
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
 *GF_itoa ��windows ��itoaһ��
 *
 *n:��ֵ
 *
 *s:�ִ�
 *
 *����ֵ:
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

