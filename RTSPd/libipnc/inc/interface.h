/*
*2012.02.19 	by Aaron
*/

#ifndef	_INTERFACE_H
#define _INTERFACE_H


#define	UN_AVSERVER_DOMAIN	"/tmp/avserver.domain"
#define UN_CAPOSD_DOMAIN	"/tmp/caposd.domain"

int create_server(char *domain);
int send_data_my(char *domain, void *data, int len);
int connect_local(char *domain);
int send_local(int socket, char *pbuf, int buf_len);
int recv_local(int socket, char *pbuf, int buf_len);
void close_local(int socket);

#endif



