/*
*2012.02.19 	by Aaron
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> 


int create_server(char *domain)
{
	int listen_fd;
	struct sockaddr_un srv_addr;

	listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(listen_fd<0){
		return 0;
	}

	srv_addr.sun_family=AF_UNIX;
	strncpy(srv_addr.sun_path, domain, sizeof(srv_addr.sun_path)-1);
	unlink(domain);

	if(bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr))<0){
		close(listen_fd);   
		unlink(domain);   
		return -1;
	}

	return listen_fd;
}


int send_data_my(char *domain, void *data, int len)
{
	int client_fd;
	struct sockaddr_un srv_addr;
	char *pdata = data;

	client_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(client_fd<0){
		return -1;
	}

	srv_addr.sun_family = AF_UNIX;
	strncpy(srv_addr.sun_path, domain, sizeof(srv_addr.sun_path)-1);
	if(connect(client_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr))<0){
		close(client_fd);
		return -1;
	}

#define SZ_4K	4*1024
	while(len>0){
		if(len>SZ_4K){
			write(client_fd, pdata, SZ_4K);
			len -= SZ_4K;
			pdata += SZ_4K;
		}else{
			write(client_fd, pdata, len);
			len -= len;
		}
	}
#undef SZ_4K

	close(client_fd);
	return 0;
}


int connect_local(char *domain)
{
	int client_fd;
	struct sockaddr_un srv_addr;

	client_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(client_fd<0){
		return -1;
	}

	srv_addr.sun_family = AF_UNIX;
	strncpy(srv_addr.sun_path, domain, sizeof(srv_addr.sun_path)-1);
	if(connect(client_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr))<0){
		close(client_fd);
		return -1;
	}

	return client_fd;
}

int send_local(int socket, char *pbuf, int buf_len)
{
	if(socket<=0)
		return -1;

#define SZ_4K	4*1024
	while(buf_len>0){
		if(buf_len>SZ_4K){
			write(socket, pbuf, SZ_4K);
			buf_len -= SZ_4K;
			pbuf += SZ_4K;
		}else{
			write(socket, pbuf, buf_len);
			buf_len -= buf_len;
		}
	}
#undef SZ_4K

	return 0;
}

int recv_local(int socket, char *pbuf, int buf_len)
{
	char *p_tmp;
	int reallen = 0;

	fd_set readset;
	int ret = 0;
	struct timeval timeout;

	if(socket<=0||pbuf==NULL){
		return -1;
	}
	p_tmp = pbuf;

	FD_ZERO(&readset);
	FD_SET(socket, &readset);
	timeout.tv_sec = 0;
	timeout.tv_usec = 200000;
	ret = select(socket+1, &readset, NULL, NULL, &timeout);
	if(ret<=0){
		return reallen;
	}
	if(FD_ISSET(socket, &readset)){
		reallen = read(socket, p_tmp, buf_len);
	}
	p_tmp[reallen] = '\n';

	return reallen;
}

void close_local(int socket)
{
	if(socket<=0)
		return;

	close(socket);

	return;
}



