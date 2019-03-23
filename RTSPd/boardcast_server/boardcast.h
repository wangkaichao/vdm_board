#ifndef BOARDCAST_H
#define BOARDCAST_H

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <assert.h>
#include <time.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <stdint.h>
#include <limits.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <math.h>
#include <linux/input.h>
#include <dlfcn.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "eventloop.h"
#include "util.h"

#define MAXBUF 1024
#define PUERTO 5001
#define GROUP "224.0.1.2"
#define BOARDCAST_VALUE "cyhqzflz2012"
typedef struct main_loop_st
{
     event_loop_t    *loop;
    int running;
}main_loop_t;

typedef enum cmd_type_stint
{
    cmd_type_boardcast=1, //按键类型
    cmd_type_file,  //传输文件
    cmd_type_other //其他命令
}cmd_type_t;

typedef struct message_st
{
    cmd_type_t cmdtype;
    int length;
    unsigned char message[1016];
}message_t;

void stop_boardcast();
int create_boardcast_worker(main_loop_t *main_loop);

void stop_boardcast_1();
int create_boardcast_worker_1(main_loop_t *main_loop);

//void create_network_timer(main_loop_t *main_loop);
//void stop_checknet();
#endif
