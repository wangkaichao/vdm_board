/*
   av_server.c
   */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <signal.h>


#include "ipnc.h"
#include "cmdpares.h"
#include "cmd_type.h"
#include "interface.h"
#include "init_system.h"

#include "GFNetComm.h"

static int avserver_start = 0;

#define MAX_CHANNEL		SYS_CHANNEL
#define MAX_LIST_FD		10
void av_server(void)
{
	int ch_num, stream, i;
	int server_fd, client_fd, arrClient_fd[MAX_LIST_FD];
	int max_socket_fd = -1;
	struct sockaddr_un client_addr;
	int readlen = -1;
	int	ret = -1;

	char *pMsg_buf;
	char *pRet_buf;
	fd_set	readfset;
	int len = 0;
	//char log[256];

	printf("av_server -->\n");

	pMsg_buf = (char *)malloc(MAX_CMD_LEN*sizeof(char));
	if(NULL==pMsg_buf){
		return;
	}
	pRet_buf = (char *)malloc(MAX_CMD_LEN*sizeof(char));
	if(NULL==pRet_buf){
		free(pMsg_buf);
		return;
	}

	server_fd = create_server(UN_AVSERVER_DOMAIN);
	if(server_fd<0){
		printf("avserver: error create server_fd.\n");
		system("echo \"avserver: error create server_fd.\" >> /mnt/nfs/system.log");
		free(pMsg_buf);
		return;
	}

	if(listen(server_fd, MAX_LIST_FD - 1)<0)
	{
		goto error_avserver;
	}
	for(i=0; i<MAX_LIST_FD; i++){
		arrClient_fd[i] = 0;
	}
	arrClient_fd[0] = server_fd;

	sleep(2);	
	avserver_start = 1;

	printf("start...\n");
	while(1){
		FD_ZERO(&readfset);
		FD_SET(arrClient_fd[0], &readfset);
		max_socket_fd = arrClient_fd[0];
		for(i=1; i<MAX_LIST_FD; i++){
			if(arrClient_fd[i]>0){
				FD_SET(arrClient_fd[i], &readfset);
				max_socket_fd = (max_socket_fd<arrClient_fd[i])?(arrClient_fd[i]):(max_socket_fd);
			}
		}

		ret = select(max_socket_fd+1, &readfset, NULL, NULL, NULL);
		if(ret<=0){
			continue;
		}
		if(FD_ISSET(server_fd, &readfset)){

			len = sizeof(client_addr);
			client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
			if(client_fd<0){
				continue;
			}

			for(i=1; i<MAX_LIST_FD; i++){
				if(arrClient_fd[i]<=0){
					arrClient_fd[i] = client_fd;
					break;
				}
			}
			if(i==MAX_LIST_FD){
				close(client_fd);
				continue;			
			}

			if(--ret==0)
				continue;
		}
		for(i=1; i<MAX_LIST_FD; i++){
			if(FD_ISSET(arrClient_fd[i], &readfset)){

				client_fd = arrClient_fd[i];
				memset(pMsg_buf, 0, MAX_CMD_LEN*sizeof(char));
				readlen = read(client_fd, (void *)pMsg_buf, MAX_CMD_LEN);
				//				printf("read len is %d\n",readlen);
				if(readlen<=3){
					FD_CLR(client_fd, &readfset);
					close(client_fd);
					arrClient_fd[i] = 0;
					continue;
				}

				//				fprintf(stdout, "get len is %d, data: %s.\n", readlen, pMsg_buf);
				//sprintf(log, "echo \"%s\" >> /mnt/mtd/time.log", pMsg_buf);
				//system(log);
				memset(pRet_buf, 0, MAX_CMD_LEN*sizeof(char));
				len = processMsg(pMsg_buf, strlen(pMsg_buf), pRet_buf);
				//				fprintf(stdout, "return len: %d.\n", len);
				if(len==0)	continue;
				//				fprintf(stdout, "return data: %s.\n", pRet_buf);
				if(write(client_fd, pRet_buf, /*strlen(pRet_buf)*/MAX_CMD_LEN)<0){
					//					fprintf(stdout, "write error.\n");
					if (errno == EBADF) {
						FD_CLR(client_fd, &readfset);
						close(client_fd);
						arrClient_fd[i] = 0;
					}
				}
			}
		}
	}

error_avserver:
	if(pMsg_buf!=NULL)
		free(pMsg_buf);
	if(pRet_buf!=NULL)
		free(pRet_buf);

	if(server_fd>0)
		close(server_fd);

	return;
}

void *rtsp_server(void *p)
{
	unsigned int nVBufferCount = 1024 * 28 / 10;

	printf("rtsp_server -->\n");

	GFSNet_Init(SER_MAX_CHANNEL, 0, 0xffffffff, 
			nVBufferCount, 400, NULL, "123",
			NULL);

	printf("rtsp_server --> 1\n");

	GFSNet_Start();

	printf("rtsp_server --> 2\n");

	StartRTSPTask();

	do {
		sleep(2);
	}while(1);
}


void get_ip_val(char *ipaddr_v, int *p)
{
	unsigned int ip_x = 0;

	if (ipaddr_v == NULL || p == NULL)
		return;

	ip_x = (unsigned int)inet_addr(ipaddr_v);
	*p = ip_x & 0xff;
	*(p+1) = (ip_x >> 8) & 0xff;
	*(p+2) = (ip_x >> 16) & 0xff;
	*(p+3) = (ip_x >> 24) & 0xff;

	return;
}

void getnetmask(char *dev_name, char *pnetmask, int len)
{
    struct ifreq ifr;
    int skfd;
    
    if ((skfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
        return ;
    }
    strcpy(ifr.ifr_name, dev_name);
    if (ioctl(skfd, SIOCGIFNETMASK, &ifr) < 0) {
        close(skfd);
        return ;
    }
    
    memset(pnetmask, 0x00, len);
    strcpy(pnetmask, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr) );
    
    close(skfd);
}

void *boot_last(void *p)
{
	char cmd[256] = {0};
	char net_dev[16] = {0};
	char netmask[16] = {0};
	int i = 0;
	int ip[4];
	unsigned int ip_x = 0;

	while (avserver_start == 0);

	sprintf(net_dev, "eth0");
	ip_x = (unsigned int)net_get_ip(0, net_dev);
	getnetmask(net_dev, netmask, strlen(netmask));

	ip[0] = ip_x & 0xff;
	ip[1] = (ip_x >> 8) & 0xff;
	ip[2] = (ip_x >> 16) & 0xff;
	ip[3] = (ip_x >> 24) & 0xff;
	if (ip[3] - MAX_CHANNEL < 1) {
		printf("boot onvifserver: ip param wrong\n");
		return NULL;
	}

	for (i = 0; i < MAX_CHANNEL; i++) {
		if (i == 0) {
			sprintf(net_dev, "eth0");
		} else {
			sprintf(net_dev, "eth0:%d", i-1);
		}
		sprintf(cmd, "ifconfig %s %d.%d.%d.%d netmask %s up", 
				net_dev, ip[0], ip[1], ip[2], ip[3] + i, netmask);
		printf("i:%d cmd: %s\n", i, cmd);
		system(cmd);
		sleep(1);

		sprintf(cmd, "./onvifserver.out -a %d.%d.%d.%d -i %d >/dev/null &", 
				ip[0], ip[1], ip[2], ip[3] + i, i);
		printf("i:%d cmd: %s\n", i, cmd);
		system(cmd);

		sleep(2);
	}
}



extern void *rtsp_client(void *pArgs);
extern void *stat_set(void *pArgs);

pthread_t   rtspc_Thread;
pthread_t   rtspd_Thread;
pthread_t   stat_Thread;
pthread_t   boot_Thread;

int boot_other()
{
	/*rtsp_client*/
	pthread_create(&rtspc_Thread, 0, rtsp_client, NULL);

	/*rtsp_server*/
	pthread_create(&rtspd_Thread, 0, rtsp_server, NULL);

	/*sys stat set*/
	pthread_create(&stat_Thread, 0, stat_set, NULL);
	
	/*start onvif server*/
	pthread_create(&boot_Thread, 0, boot_last, NULL);

	return 0;
}


