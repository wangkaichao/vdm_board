#include "boardcast.h"
#include "selfserver.h"

struct sockaddr_in boardcast_ser;
static event_source_t *boardcast_source = NULL;
struct sockaddr_in boardcast_ser_1;
static event_source_t *boardcast_source_1 = NULL;

static event_source_t *g_net_source = NULL;
#define INTERVAL_TIME_NET   2*1000
char *g_mac = NULL;
extern config_value cfg_v;
char* getMAC()
{
    if(g_mac == NULL)
    {
        g_mac=(char*)malloc(20*sizeof(char));
        memset(g_mac,0x00,20);
        struct ifreq ifr;
        int skfd;
        if ((skfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            return NULL;
        }
        strcpy(ifr.ifr_name, "eth0");
        if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
        {
            close(skfd);
            return NULL ;
        }

        if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
        {
            close(skfd);
            return NULL;
        }
        else
        {
            sprintf(g_mac,"%02x%02x%02x%02x%02x%02x",
                    (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
        }
        close(skfd);
    }
    return g_mac;
}

void getMAC1(char mac[7])
{
    //char* mac = NULL ;

    int x = 0;
   // if(g_mac == NULL)
    {
         //mac=(char*)malloc(6*sizeof(char));mac[6] = '\0';
        memset(mac, 0x00,7);
        struct ifreq ifr;
        int skfd;
        if ((skfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            return NULL;
        }
        strcpy(ifr.ifr_name, "eth0");
        if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
        {
            close(skfd);
            return NULL ;
        }

        if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
        {
            close(skfd);
            return NULL;
        }
        else
        {
            for( x=0;x<6;x++)
                mac[x] = (unsigned char)ifr.ifr_hwaddr.sa_data[x];
            char mac1[20];
            sprintf(mac1,"%02x%02x%02x%02x%02x%02x",
                    (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
        }
        close(skfd);
    }
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

void getnetip(char *dev_name, char *ip, int len)
{
    struct ifreq ifr;
    int skfd;
    
    if ((skfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {
        return ;
    }
    strcpy(ifr.ifr_name, dev_name);
    if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
        close(skfd);
        return ;
    }
    
    memset(ip, 0x00, len);
    strcpy(ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr) );
    
    close(skfd);
}


int s;
int s1;

void resposeboardcast(int fd,int cmdtype,int result)
{
    char ret[1024];;
    memset(ret,0x00,sizeof(ret));
    char* mac = getMAC();
    sprintf(ret,"%s&%s",mac,cfg_v.version);
    if (mac != NULL) {
    	free(mac);
    }
    //printf("send data:%s\n",ret);
    //int s;
    struct sockaddr_in srv;

   //char buf[MAXBUF];
    //memset(buf,0x00,MAXBUF);
    bzero(&srv, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PUERTO+1);
    if (inet_aton(GROUP, &srv.sin_addr) < 0)
    {
        perror("inet_aton");
        //return 1;
    }

    //int x = sendto(s,ret,1024,0,(struct sockaddr *)&srv, sizeof(srv));
    int ss = 0;
    if((ss = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
           // perror("socket");
            return ;
    }

    struct sockaddr_in des;
    bzero(&des, sizeof(des));
    des.sin_family = AF_INET;
    des.sin_addr.s_addr = inet_addr(inet_ntoa(boardcast_ser.sin_addr));
    des.sin_port = htons(10086);
        static int check = 0;
        while(connect(ss,&des, sizeof(des)) < 0)
        {
            printf("connect server error,try %d\n",check+1);
            sleep(1);check++;
            if(check == 5) break;
        }
        if(check == 5)
        {
            close(ss);check=0;return;
        }
        send(ss,ret,1024,0);
        close(ss);
    /*if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
            perror("socket");
            return 1;
    }*/
    //sockaddr_in des;
    //des.sin_port = 5001;
    //des.sin_family = AF_INET;
    //inet_aton("192.168.0.143", &des.sin_addr);
    //boardcast_ser.sin_port = 5001;

    //close(s);
    //printf("send data IP:%s\n port:%d\n",inet_ntoa(boardcast_ser.sin_addr),boardcast_ser.sin_port);
    //printf("send data num:%d\n",x);
}

int process_boardcast(message_t* msg,int fd)
{
    printf("process_boardcast:%s\n",msg->message);
    if(msg->message[msg->length-1] != '#') return -1;
    msg->message[msg->length-1] = '\0';
    if(!strcmp(msg->message,BOARDCAST_VALUE))
    {
        //printf("boardcast ok\n");
        resposeboardcast(fd,cmd_type_boardcast,0);
    }
    else
    {
        printf("boardcast error\n");
        return -1;
    }
}

int parser_msg(message_t* msg,int fd,int index)
{
    //printf("cmd type:%d\n",msg->cmdtype);
    int ret = -1;
    if(msg->cmdtype == cmd_type_boardcast)
    {
        ret = process_boardcast(msg,fd);
        return ret;
    }
    /*if(msg->cmdtype != cmd_type_other)
    {
        if(clientok[index] == 0)
        {
            resposeclient(fd,COMMAND_TYPE_OTHERS,Result_NO_LOGIN);
            return ret;
        }
    }
    switch(msg->cmdtype)
    {
    case cmd_type_file:
        {
            ret = process_file(msg,fd);
            break;
        }
    case cmd_type_other:
        {
            ret = process_other(msg,fd,index);
            break;
        }
    default:
        {
            printf("the data is error\n");
        }
    }*/
    return ret;
}

int boardcast_func(int fd, void *data,struct epoll_event *ep)
{
    printf("boardcast_func\n");
    int i=0;int n = sizeof(boardcast_ser);int ret;
    char info[MAXBUF];
    memset(info,0x00,MAXBUF);//boardcast_ser
    ret = recvfrom(fd,info,MAXBUF,0,(struct sockaddr *)&boardcast_ser,(socklen_t*)&n);

    printf("IP:%s\n port:%d\n",inet_ntoa(boardcast_ser.sin_addr),boardcast_ser.sin_port);
    //int ret = connection_read(fd, info, sizeof(info));
    if(( ret < 0))
    {
        fprintf(stderr, "boardcast_func:failed to connection_read, errno: %d\n", errno);
        return -1;
    }
    //printf("boardcast_func info:%s\n",info);
    if(ret == 0) return 1;
    int len = strlen(info);
    if(len < 3) return 1;
    message_t msg;
    memset(&msg, 0x00, sizeof(msg));
    if(info[0] != '#') return -1;//数据无效
    char type[2];
    type[0] = info[1];type[1] = '\0';
    msg.cmdtype = atoi(type);
    if(msg.cmdtype < 1) return -1;//数据无效
    for(i=3;i<len;i++)
    {
        msg.message[i-3] = info[i];
    }
    msg.length = len-3;
    ret = parser_msg(&msg,fd,-1);
    if(ret == 1)
    {
        return -1;
    }
    return 0;
}

int create_boardcast_worker(main_loop_t *main_loop)
{
    struct sockaddr_in srv;
    struct ip_mreq mreq;

    bzero(&srv, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PUERTO);
    if (inet_aton(GROUP, &srv.sin_addr) < 0)
    {
        perror("inet_aton");
        return 1;
    }
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        return 1;
    }
    if(bind(s, (struct sockaddr *)&srv, sizeof(srv)) < 0)
    {
        perror("bind");
        return 1;
    }
    if (inet_aton(GROUP, &mreq.imr_multiaddr) < 0)
    {
        perror("inet_aton");
        return 1;
    }
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if(setsockopt(s, IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))< 0)
    {
        perror("setsockopt");
        return 1;
    }
    boardcast_source = event_loop_add_fd((event_loop_t *)main_loop->loop, s, boardcast_func, 1, NULL);
    event_source_fd_update(boardcast_source,0);
    return 0;
}

void stop_boardcast()
{
    if(boardcast_source != NULL)
    {
        event_source_remove(boardcast_source);
        boardcast_source = NULL;
    }
}

#define MSG_STRING "grandstream"
#define NET_PARAM 1
#define BOARD_PARAM 2
#define DEVICE_DVS "DVS"
#define DEVIVE_IPCAM "IPCAMERA"
#define DEVICE_DECODER "DECODER"

#define DEVICE_NAME "HD_IPNC"
struct BoardParam
{
    u_char message[12];
    u_char msg_type;
    u_char ipaddress[30];
};

struct RecvParam
{
    u_char message[12];
    u_char msg_type;
    u_char dev_type[10];
    u_char mac_value[7];
    u_char netmask_value[30];
    u_char ip_value[30];
    u_char dev_name[64];
    u_int web_port;
    u_int rtsp_port;
    u_char uc_reserve[10];
    u_char devmodel[32];
};

union endian{
    unsigned char a[4];
    u_int b;
};

void stop_boardcast_1()
{
    if(boardcast_source_1 != NULL)
    {
        event_source_remove(boardcast_source_1);
        boardcast_source_1 = NULL;
    }
}

// s1, boardcast_source_1: bind INADDR_ANY:6789 UDP
// callback for accept can read s1(fd)
// repose for pc boardcast ip:6789 UDP
int boardcast_func_1(int fd, void *data,struct epoll_event *ep)
{
    printf("boardcast_func_1\n");
    int i=0;
    int n = sizeof(boardcast_ser_1);
    int ret;

    struct BoardParam board_value;
    memset(&board_value,0x00,sizeof(board_value));//boardcast_ser
    ret = recvfrom(fd, &board_value, sizeof(board_value), 0, (struct sockaddr *)&boardcast_ser_1,(socklen_t*)&n);

    printf("IP:%s\n port:%d\n", inet_ntoa(boardcast_ser_1.sin_addr), boardcast_ser_1.sin_port);

    if(( ret < 0))
    {
        fprintf(stderr, "boardcast_func_1:failed to connection_read, errno: %d\n", errno);
        return -1;
    }
    if(ret == 0) return 1;
    printf("message:%s type:%d\n",board_value.message, board_value.msg_type);
    if(strcmp(board_value.message, MSG_STRING))
    {
        printf("message is error\n"); return 1;
    }
    if(board_value.msg_type != 2)
    {
        printf("message type is error\n"); return 1;
    }

    char mac[7];getMAC1(mac);
    
    getnetmask("eth0", cfg_v.netmask, sizeof(cfg_v.netmask));
    printf("netmask:%s\n", cfg_v.netmask);

    struct RecvParam recv_value;
    memset(&recv_value,0x00,sizeof(recv_value));
    memcpy(recv_value.message,MSG_STRING,strlen(MSG_STRING));
    recv_value.msg_type = 1;
    memcpy(recv_value.dev_type,DEVIVE_IPCAM,strlen(DEVIVE_IPCAM));
    memcpy(recv_value.mac_value,mac,7);
    memcpy(recv_value.netmask_value, cfg_v.netmask, strlen(cfg_v.netmask));
    memcpy(recv_value.ip_value, cfg_v.ip, strlen(cfg_v.ip));
    //memcpy(recv_value.devmodel,"model",strlen("model"));
    memset(recv_value.devmodel,0x00,sizeof(recv_value.devmodel));
    sprintf(recv_value.devmodel,"%d&%d",cfg_v.x, cfg_v.y);
    sprintf(&recv_value.devmodel[10],"%s",cfg_v.uid);
    memcpy(recv_value.dev_name,DEVICE_NAME,strlen(DEVICE_NAME));

    if(strlen(cfg_v.version) < 10)
        memcpy(recv_value.uc_reserve,cfg_v.version,strlen(cfg_v.version));
    else
        memcpy(recv_value.uc_reserve,cfg_v.version,10);

    union endian tmp;
    union endian tmp2;
    tmp.b = cfg_v.netkeyboard_id;
    tmp2.b = 554;

    recv_value.web_port = cfg_v.netkeyboard_id;//((tmp.a[0]<<24)|(tmp.a[1]<<16)|(tmp.a[2]<<8)|(tmp.a[3]));
    recv_value.rtsp_port = ((tmp2.a[0]<<24)|(tmp2.a[1]<<16)|(tmp2.a[2]<<8)|(tmp2.a[3]));
    ret = sendto(s, &recv_value,sizeof(recv_value),0,(struct sockaddr *)&boardcast_ser_1, sizeof(boardcast_ser_1));
    //printf("message:%s type:%d devtype:%s mac:%s mask:%s ip:%s\n",recv_value.message,recv_value.msg_type,recv_value.dev_type,recv_value.mac_value,recv_value.netmask_value,recv_value.ip_value);

    return 0;
}


// Create UDP Server: socket_fd bind INADDR_ANY:6789
int create_boardcast_worker_1(main_loop_t *main_loop)
{
    //struct sockaddr_in client;
    struct sockaddr_in srv;
    socklen_t len;
    if ((s1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket1");
        return 1;
    }
   
    bzero(&srv, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port = htons(6789);
    //printf("create_boardcast_worker_1 ip :%s\n", cfg_v.ip);
    //inet_pton(AF_INET, cfg_v.ip, &srv.sin_addr);
    srv.sin_addr.s_addr = htons(INADDR_ANY);

    if(bind(s1, (struct sockaddr *)&srv, sizeof(srv)) < 0)
    {
        perror("bind1");
        return 1;
    }

    boardcast_source_1 = event_loop_add_fd((event_loop_t *)main_loop->loop, s1,boardcast_func_1,0, NULL);
    event_source_fd_update(boardcast_source_1,0);
    return 0;
}


#if 0
static int is_cable_linked()
{
    #define LINKTEST_GLINK 0x0000000a
    struct linktest_value
    {
        unsigned int    cmd;
        unsigned int    data;
    };
    int sd = -1;
    struct ifreq ifr;
    struct linktest_value edata;
    memset(&edata, 0x00, sizeof(struct linktest_value));
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        //debug("get network status: open socket error:%s\n", strerror(errno));
        return 0;
    }
    edata.cmd = LINKTEST_GLINK;
    memset(&ifr, 0x00, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name)-1);
    ifr.ifr_data = (char *) &edata;
    if (ioctl(sd, SIOCETHTOOL, &ifr) == -1)
    {
        //debug("ETHTOOL_GLINK failed: %s\n", strerror(errno));
        close(sd);
        return 0;
    }
    if (edata.data)
    {
        close(sd);
        return 1;
    }
    close(sd);
    return 0;
}
int check_net()
{
    int i = 0;
    if(!is_cable_linked())
    {
        printf("the net cut off\n");
    }

    event_source_timer_update(g_net_source,INTERVAL_TIME_NET);
    return 0;
}

void create_network_timer(main_loop_t *main_loop)
{
    g_net_source = event_loop_add_timer((event_loop_t *)main_loop->loop,check_net,0,NULL);
    event_source_timer_update(g_net_source,INTERVAL_TIME_NET);
}

void stop_checknet()
{
    if(g_net_source != NULL)
    {
        event_source_remove(g_net_source);
        g_net_source = NULL;
    }
}
#endif

