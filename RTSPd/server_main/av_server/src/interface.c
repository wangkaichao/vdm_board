
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>


int create_ipc_svr(int port, int listen_num)
{
	int sock;
	struct sockaddr_in svr_addr;
	socklen_t sin_size;
	int reuse = 1;
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("create socket fail\n");
        return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 
				&reuse, sizeof(int)) == -1) {
		perror("setsockopt fail\n");
        return -1;
	}

	svr_addr.sin_family = AF_INET;
	svr_addr.sin_port = htons(port);
	svr_addr.sin_addr.s_addr = INADDR_ANY;
	memset(svr_addr.sin_zero, 0, sizeof(svr_addr.sin_zero));
	if (bind(sock, (struct sockaddr*)&svr_addr, 
			sizeof(svr_addr)) == -1) {
		perror("bind fail\n");
        return -1;
	}
	
	if (listen(sock, listen_num) < 0) {
		perror("listen fail\n");
        return -1;
	} 

	printf("listen port: %d\n", port);
	return sock;	
}

