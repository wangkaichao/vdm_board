

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <time.h>

#include <linux/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "ipnc.h"
#include "misc.h"

int ipOk(char *ip)
{
	//open_debug();
	int arr[4];
	//printf("ip=%s\n",ip);
	if(4!=sscanf(ip,"%d.%d.%d.%d",&arr[0],&arr[1],&arr[2],&arr[3]))
	{
		//puts("error ip format ,right is: x.x.x.x");
		return 0;
	}
	else
	{
		int i=1;
		if(arr[0]<10 || arr[0]>255)
		{
			//puts("error ip value");
			return 0;
		}
		for(i=1;i<4;i++)
		{
			// printf("arr[%d]=%d\n",i,arr[i]);
			if(arr[i]>255)
			{
				// puts("error ip value");
				return 0;
			}
		}
	} 
	return 1;
}

void getruntime(char *ctime)
{
	struct sysinfo s_info;
	long uptime_1 = 0;

	time_t timep;
	struct tm *p;

	if(0==sysinfo(&s_info))
	{
		uptime_1 = s_info.uptime;
	}

	time(&timep);
	timep -= uptime_1;

	p = localtime(&timep);
	strftime(ctime, sizeof(ctime),"%Y-%m-%d %H:%M:%S", p);

	DBG("system run time is : %s.\n", ctime);

	return;
}

int getIp(char *outip)
{
	int i=0;
	int sockfd;
	struct ifconf ifconf;
	char buf[512];
	struct ifreq *ifreq;
	char* ip;
	//初始化ifconf
	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
	{
		return -1;
	}
	ioctl(sockfd, SIOCGIFCONF, &ifconf);    //获取所有接口信息
	close(sockfd);
	//接下来一个一个的获取IP地址
	ifreq = (struct ifreq*)buf;
	for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
	{
		ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);

		if(strcmp(ip,"127.0.0.1")==0)  //排除127.0.0.1，继续下一个
		{
			ifreq++;
			continue;
		}
		strcpy(outip,ip);
		return 0;
	}

	return -1;
}



