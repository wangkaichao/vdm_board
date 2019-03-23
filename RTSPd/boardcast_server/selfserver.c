#include "selfserver.h"
#include "md5.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>          //Êñá‰ª∂ÊéßÂà∂ÂÆö‰πâ
#include <termios.h>     //POSIX‰∏≠Êñ≠ÊéßÂà∂ÂÆö‰πâ
#include <errno.h>        //ÈîôËØØÂè∑ÂÆö‰πâ
#include <time.h>

#include "hi_rtc.h"

#define TMP_UPDATE_FILE "/tmp/bin.tar.gz"
#define TMP_UPDATE_FILE_ALL "/tmp/bin.tar.gz"
extern config_value cfg_v;
struct sockaddr_in client;
static event_source_t *server_source = NULL;
static event_source_t *client_source = NULL;
int clientnum = 1;
int server_fd = 0;
int client_fd = 0;
int fp = NULL;
int len_filedata = 0;
char md5_r[17];
int g_check_update = 0;
int file_type = -1;
static event_source_t *client_check_source = NULL;


int g_insocket = -1;
static event_source_t *inserver_receiver_source = NULL;
static event_source_t *g_com_source = NULL;

/**********************************/
#define RTC_DEV_NAME		"/dev/hi_rtc"
#define COM_PACKAGE_LEN		(32)
static int date_y;
static int date_x;
static int time_y;
static int time_x;

static int ch1_time_osd = 0;
static int ch2_time_osd = 0;
static int set_time_flag = 0;
static void osd_callback(int chn, int x, int y, const unsigned char *p, int len);
/**********************************/

int g_char_socket = -1;
pthread_mutex_t mutex;
void closefile()
{
    pthread_mutex_lock(&mutex);
    if(fp )
    {
        len_filedata = 0;
        fclose(fp);
        fp = NULL;
	file_type = 0;
    }
    pthread_mutex_unlock(&mutex);
}

void close_client()
{
    pthread_mutex_lock(&mutex);
    printf("close client 1\n");
    if(fp)
    {
        close(fp);
        fp = NULL;
    }
    if(client_fd > 0)
    {
        close(client_fd);
        client_fd = 0;
    }
    if(client_source)
    {
        event_source_remove(client_source);
        client_source = NULL;
    }
    pthread_mutex_unlock(&mutex);
}

void close_server()
{
    pthread_mutex_lock(&mutex);
    if(server_fd > 0)
    {
        close(server_fd);
        server_fd = 0;
    }
    if(server_source)
    {
        event_source_remove(server_source);
        server_source = NULL;
    }
    pthread_mutex_unlock(&mutex);
}

void stop_selfserver()
{
    closefile();
    close_client();
    close_server();
}

void respose_for_file(int kind)
{
    char ret[1024];;
    memset(ret,0x00,sizeof(ret));
    sprintf(ret,"%d\n",kind);
    if(client_fd>0)
    {
        printf("send data:%d\n",kind);
        send(client_fd,ret,1024,0);
    }
}

void update()
{
	closefile();
	unsigned char ret[17];	    ret[16] = '\0';
	if(file_type == 2)
	{
	    create_md5_value_by_file_16(TMP_UPDATE_FILE,ret);
	    printf("receive file md5 data:%s\n",ret);
	    if(!strcmp(ret,md5_r))
	    {
                respose_for_file(1);
                system("sh /mnt/mtd/upload.sh /tmp/bin.tar.gz"); //reboot later
	    }
	    else
           {
                respose_for_file(0);
	         printf("md5 is not right\n");
           }
	    g_check_update = 0;
	    memset(md5_r,0x00,17);
	    // upload.sh will mv TMP_UPDATE_FILE -> rm /mnt/mtd/bin.tar.gz
	    unlink(TMP_UPDATE_FILE);
	}
	else if(file_type == 4)
	{
	    create_md5_value_by_file_16(TMP_UPDATE_FILE_ALL,ret);
	    printf("receive file md5 data:%s\n",ret);
	    if(!strcmp(ret,md5_r))
	    {
                 respose_for_file(1);
            //system("sh /web/html/upload.sh /tmp/zImage_from_client");
                 system("sh /mnt/mtd/upload.sh /tmp/bin.tar.gz");
	    }
	    else
            {
                 respose_for_file(0);
	        printf("md5 is not right in image\n");
            }
	    g_check_update = 0;
	    memset(md5_r,0x00,17);
	    //unlink(TMP_UPDATE_FILE_ALL);
	}
	
	file_type = 0;
	//close_client();
}



int connection_read(int fd, const void *data, size_t nbytes,int total)
{
    if(nbytes <= 0) return 0;
    //printf("size:%d total:%d\n",nbytes,total);
    if(total < nbytes)
        nbytes = total;
    int ret = 0;
    int remain = nbytes;
    char *p = (char *)data;
    //printf("read total:%d\n",nbytes);
    do{
        if((ret = read(fd, p + nbytes - remain, remain)) > 0)
        {
            remain -= ret;
        }
        else if(ret < 0)
        {
            if(errno == EAGAIN)//Âç≥ÂΩìlen<0‰∏îerrno=EAGAINÊó∂ÔºåË°®Á§∫Ê≤°ÊúâÊï∞ÊçÆ‰∫Ü„ÄÇ(ËØª/ÂÜôÈÉΩÊòØËøôÊ†∑)
                break;
            else
                return -1;//error
        }
        else
        {
            return -2;//ÂØπÊñπÂÖ≥Èó≠
        }
        //printf("read %d %s \n",ret,(char*)data);
    }while(remain > 0 && ret > 0);
    //printf("read %d %d\n",nbytes,remain);
    return nbytes - remain;
}

int write_file(char* data,int len,int fd)
{
    //printf("write file %d\n",len);
    if((len == 0))
    {
        closefile();
        return 1;
    }
    int remain = len;
    int ret=0;
    char *p = data;
    do{
        if((ret = fwrite(p + len - remain, 1,remain,fp)) > 0)
        {
            remain -= ret;
        }
    }while(remain > 0 && ret > 0);
    return 0;
}

 void saveconfig()
 {
     FILE *cfgfp = NULL;
     
     cfgfp = fopen(CONFIG_FILE_PATH_TMP, "w+");
     if (cfgfp == NULL)
     {printf("open config file error\n"); return;}

     char info[50];
     memset(info, 0x00, 50);
     sprintf(info, "ip %s\n", cfg_v.ip);
     fwrite(info, strlen(info), 1, cfgfp);

     memset(info, 0x00, 50);
     sprintf(info, "netmask %s\n", cfg_v.netmask);
     fwrite(info, strlen(info), 1, cfgfp);
     
     memset(info, 0x00, 50);
     printf("mac %s\n", cfg_v.mac);
     sprintf(info, "mac %s\n", cfg_v.mac);
     fwrite(info, strlen(info), 1, cfgfp);
     fclose(cfgfp);

    system("mv /mnt/mtd/if_tmp.cfg /mnt/mtd/if.cfg");
 }


 void save_iptable()
 {
     printf("save_iptable....\n");
     FILE *cfgfp = NULL;
     int i = 0;char info[300];
     cfgfp = fopen(CONFIG_FILE_BOARD_IPS_TMP,"w+");
     if(cfgfp == NULL)
     {   printf("open config file error\n"); return;}

     for(i=0;i<4;i++)
     {
         memset(info,0x00,300);
         sprintf(info,"%s\n",cfg_v.ips[i]);
         fwrite(info,strlen(info),1,cfgfp);
     }

    fclose(cfgfp);

    system("mv /mnt/mtd/iptable_tmp.txt /mnt/mtd/iptable.txt");
    close_client();
 }

void saveconfig_uid()
{
    FILE *cfgfp = NULL;
    cfgfp = fopen(CONFIG_FILE_UID_TMP,"w+");
    if(cfgfp == NULL)
    {   printf("open config file error\n"); return;}


    char info[64];
    memset(info,0x00,64);
    sprintf(info,"%s\n",cfg_v.uid);
    fwrite(info,strlen(info),1,cfgfp);

   fclose(cfgfp);

   system("mv /mnt/mtd/uid_tmp.cfg /mnt/mtd/uid.cfg");
}

void saveconfig_netkeyboard_id()
{
    FILE *cfgfp = NULL;
    cfgfp = fopen(CONFIG_FILE_NETKEYBOARDID_TMP,"w+");
    if(cfgfp == NULL)
    {   printf("open config file error\n"); return;}


    char info[64];
    memset(info,0x00,64);
    sprintf(info,"netkeyboard_id %d\n",cfg_v.netkeyboard_id);
    fwrite(info,strlen(info),1,cfgfp);

   fclose(cfgfp);

   system("mv /mnt/mtd/netkeyboard_id_tmp.cfg /mnt/mtd/netkeyboard_id.cfg");
}

 void saveconfig_xy()
 {
     FILE *cfgfp = NULL;
     cfgfp = fopen(CONFIG_FILE_XY_tmp,"w+");
     if(cfgfp == NULL)
     {printf("save:open xy config file error\n"); return;}

     char info[50];
     memset(info,0x00,50);
     sprintf(info,"x %d\n",cfg_v.x);
     fwrite(info,strlen(info),1,cfgfp);
     memset(info,0x00,50);
     sprintf(info,"y %d\n",cfg_v.y);
     fwrite(info,strlen(info),1,cfgfp);
    fclose(cfgfp);

    system("mv /mnt/mtd/xy_tmp.cfg /mnt/mtd/xy.cfg");

    system("/sbin/reboot");
    return;
 }
 extern int urls_num;

static int client_connection_active(int fd, void *data,struct epoll_event *ep)
{
    char info[10240];
    memset(info,0x00,10240);
   
    if(fp != NULL)
    {
        int size = 10240;
        int xret = 1;
        while(1)
        {
             xret = 1;
            if(len_filedata<10240) 
		size = len_filedata;

            xret = connection_read(fd, info, size,size);
            if(xret < 0)
            {
                fprintf(stderr, "failed to connection_read 1, errno: %d\n", errno);
                closefile();
                close_client();
                return -1;
            }
            if(xret == 0) break;

            write_file(info,size,fd);
            len_filedata = len_filedata-size;
            //printf("len file data:%d\n",len_filedata);
        }
        //int xret = connection_read(fd, info, size,size);
       //printf("len file data:%d\n",len_filedata);
        if(len_filedata <=0)
        {
            g_check_update = 1;
            fclose(fp);fp = NULL;
        }
        return 0;
    }
	
    if(client_fd <= 0) return -1;
    int ret = connection_read(fd, info, 4096,4096);
    if (ret < 0)
    {
        fprintf(stderr, "failed to connection_read 2, errno: %d\n", errno);	
        closefile();
        close_client();
        return -1;
    }

    if(info[0] != '#')
    {
        return -1;
    }
    char type[2];
    type[0] = info[1];
    type[1] = '\0';
    int cmdtype = atoi(type);
    printf("cmdtype:%d\n",cmdtype);
	
    if((cmdtype == 2)||(cmdtype == 4))
    {
	 file_type = cmdtype;
        char lens[10];
        memset(lens,0x00,10);
        int xlen = strlen(info);
        int x= 3;
        for(x=3;x<xlen-1;x++) {
            if(info[x] != '&')
            	lens[x-3] = info[x];
            else
            {lens[x-3]='\0';break;}
        }
        len_filedata = atoi(lens);
        x++;int xx=0;
        for(;x<xlen-1;x++) {
		md5_r[xx++] = info[x];
        	if(xx == 16) break;
	 }
        md5_r[xx]='\0';
       // printf("len file data:%d\n",len_filedata);
       // printf("file md5 data:%s\n",md5_r);
        if(fp != NULL)
        {
            fclose(fp);g_check_update=0;file_type=0;
        }
        
	if (file_type == 2)
	     fp = fopen(TMP_UPDATE_FILE, "wb+");
	if (file_type == 4) 
		fp = fopen(TMP_UPDATE_FILE_ALL, "wb+");
	if (fp == NULL)
	  	printf("fp  null\n");
    }
    else if(cmdtype == 5)	//get values
    {
        char cmds[10];
        memset(cmds,0x00,10);
        int xlen = strlen(info);
        int x= 3;
        for(x=3;x<xlen-1;x++)
        {
            if((info[x] != '&')||(info[x] != '#'))
                cmds[x-3] = info[x];
            else
            {cmds[x-3]='\0';break;}
        }
        int cmd_kind =  atoi(cmds);


        printf("kind:%d\n",cmd_kind);

        if(cmd_kind == 1)//get uid
        {
            int num = send(fd,cfg_v.uid,64,0);
            printf("send uid ok,%d\n",num);
            usleep(1000);return;
        }
    }
    else if(cmdtype == 7)
    {
        int num = 0;
        printf("to get urls,%d\n", urls_num);
        char xxinfo[4096];
        memset(xxinfo,0x00,4096);
        switch(urls_num)
        {
        case 1:
            sprintf(xxinfo,"#1#%s#",cfg_v.ips[0]);printf("url info:%s\n",xxinfo);
            num = send(fd,xxinfo,4096,0);
            printf("send iptable ok,%d\n",num);
            usleep(1000);return;
            break;
        case 2:
            sprintf(xxinfo,"#2#%s#%s#",cfg_v.ips[0],cfg_v.ips[1]);printf("url info:%s\n",xxinfo);
            num = send(fd,xxinfo,4096,0);
            printf("send iptable ok,%d\n",num);
            usleep(1000);return;
            break;
        case 3:
            sprintf(xxinfo,"#3#%s#%s#%s#",cfg_v.ips[0],cfg_v.ips[1],cfg_v.ips[2]);printf("url info:%s\n",xxinfo);
            num = send(fd,xxinfo,4096,0);
            printf("send iptable ok,%d\n",num);
            usleep(1000);return;
            break;
        case 4:
            sprintf(xxinfo,"#4#%s#%s#%s#%s#",cfg_v.ips[0],cfg_v.ips[1],cfg_v.ips[2],cfg_v.ips[3]);printf("url info:%s\n",xxinfo);
            num = send(fd,xxinfo,4096,0);
            printf("send iptable ok,%d\n",num);
            usleep(1000);return;
            break;
        default:
            return;
            break;
        }

        return ;
    }
    else if(cmdtype == 3)
    {
        char cmds[10];
        memset(cmds,0x00,10);
        char value[300];
        memset(value,0x00,300);
        
        int xlen = strlen(info);
        int x= 3;
        for (x = 3; x < xlen - 1; x++)
        {
            if (info[x] != '&')
                cmds[x-3] = info[x];
            else
            {cmds[x-3]='\0';break;}
        }
        int cmd_kind =  atoi(cmds);
		
        x++;int xx=0;
        for (; x < xlen - 1; x++) {
        	value[xx++] = info[x];
        	if (xx >= 299) break;
        }
        value[xx]='\0';
        printf("kind:%d\n",cmd_kind);
        printf("value:%s\n",value);
        
        if(cmd_kind == 1) //ip
        {
            strncpy(cfg_v.ip,value,32);
        }
        else if(cmd_kind == 2) //mac
        {
            strncpy(cfg_v.mac,value,32);
        }
        else if(cmd_kind == 4) //netmask
        {
        	strncpy(cfg_v.netmask, value, 32);
        }
        else if(cmd_kind == 5) //netkeyboard id
        {
	     cfg_v.netkeyboard_id = atoi(value);
            printf("save netkeyboard_id:%d\n",cfg_v.netkeyboard_id);
            saveconfig_netkeyboard_id();return;
        }
        else if(cmd_kind == 3) //x, y
        {
            int xylen = strlen(value);
            int yy=0;
            char tmptmp1[10];
            memset(tmptmp1,0x00,10);
            char tmptmp2[10];
            memset(tmptmp2,0x00,10);
            for(yy=0;yy<xylen;yy++)
            {
                if(value[yy] != '&')
                    tmptmp1[yy] = value[yy];
                else
                {tmptmp1[yy] = '\0';break;}
            }
            cfg_v.x = atoi(tmptmp1);
            yy++;
            int xxx=0;
            for(;yy<xylen;yy++)
            {  tmptmp2[xxx] = value[yy];xxx++;
            }
            tmptmp2[xxx]='\0';
            cfg_v.y = atoi(tmptmp2);printf("save x:%d,y:%d\n",cfg_v.x,cfg_v.y);
            saveconfig_xy();
            return;
        }
        else if(cmd_kind == 0)
        {
            //sleep(2);
            system("/sbin/reboot");
            return;
        }
        else if(cmd_kind == 6)//board ip1
        {
            strncpy(cfg_v.ips[0],value,300);
            //strncpy(cfg_v.ip,value,32);
            save_iptable();
            return;

        }
        else if(cmd_kind == 7)//board ip1
        {
            strncpy(cfg_v.ips[1],value,300);
            save_iptable();
            return;
        }
        else if(cmd_kind == 8)//board ip1
        {
            strncpy(cfg_v.ips[2],value,300);
            save_iptable();
            return;
        }
        else if(cmd_kind == 9)//board ip1
        {
            strncpy(cfg_v.ips[3],value,300);
            save_iptable();
            return;
        }

        saveconfig();
    }

}

static int server_accept_net(int fd, void *data, struct epoll_event *ep)
{
    printf("SSSS\n");
    main_loop_t *main_loop = (main_loop_t*)data;
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(struct sockaddr);
    if(client_fd > 0)
    {

         printf("is connected\n");
         closefile();
         close_client();

        return -1;
    }
    int i=0;
    //while(1)
    {
        client_fd = accept(fd, (struct sockaddr *)&clientaddr,&len);
        if (client_fd < 0)
        {
            fprintf(stderr, "failed to accept, errno: %d\n", errno);
            return -1;
        }
        else
        {
                printf(" client is connected now\n");
                //close_client();
                client_source = event_loop_add_fd((event_loop_t *)main_loop->loop, client_fd,
                                                      client_connection_active, 1,main_loop);
                event_source_fd_update(client_source,1);
        }
    }
    return 1;
}

//Create TCP Server: socket_fd bind INADDR_ANY:16101
int create_selfserver(main_loop_t *main_loop)
{
    pthread_mutex_init(&mutex, NULL);

    struct sockaddr_in serveraddr;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        fprintf(stderr, "failed to create socket, errno: %d\n", errno);
        return -1;
    }
    bzero(&serveraddr,sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SELF_SERVER_PORT);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serveraddr.sin_zero),8);
    if(bind(server_fd,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr)) == -1)
    {
        //fprintf(stderr, "failed to bind socket, errno: %d\n", errno);
        close_server();
        return -1;
    }
    if(listen(server_fd,clientnum)==-1)
    {
        fprintf(stderr, "failed to listen socket, errno: %d\n", errno);
        close_server();
        return -1;
    }

    if ((server_source = event_loop_add_fd((event_loop_t *)main_loop->loop, server_fd,
                          server_accept_net, 0,main_loop)) == NULL)
    {
        close_server();
        return -1;
    }
     event_source_fd_update(server_source,0);
    return 0;
}

#define INTERVAL_TIME_CHECK_CLIENT 1000

// 1s timer
int check_client()
{
	int fd = -1;
	rtc_time_t tm;
	char strcurdate[] = "YYYY-MM-DD"; 
	char strcurtime[] = "HH:MM:SS";

	if(g_check_update) update();
	
	event_source_timer_update(client_check_source, INTERVAL_TIME_CHECK_CLIENT);
	
	if ((ch1_time_osd && set_time_flag) || (ch2_time_osd && set_time_flag)) {
		fd = open(RTC_DEV_NAME, O_RDWR);
		if (!fd) {
			printf("opendev_name failed\n", RTC_DEV_NAME);
			return 0;
		}

		if (ioctl(fd, HI_RTC_RD_TIME, &tm) < 0) {
			printf("ioctl: HI_RTC_RD_TIME failed\n");
			close(fd);
			return 0;
		}
		close(fd);
		
		sprintf(strcurdate, "%04d-%02d-%02d", tm.year, tm.month, tm.date);
		sprintf(strcurtime, "%02d:%02d:%02d", tm.hour, tm.minute, tm.second);
		
		if (ch1_time_osd && set_time_flag) {
			osd_callback(0, date_x * 24, date_y, strcurdate, strlen(strcurdate));
			osd_callback(0, time_x * 24, time_y, strcurtime, strlen(strcurtime));
		}

		if (ch2_time_osd && set_time_flag) {
			osd_callback(1, date_x * 24, date_y, strcurdate, strlen(strcurdate));
			osd_callback(1, time_x * 24, time_y, strcurtime, strlen(strcurtime));
		}	
	}

    return 0;
}

//Create timer 1s, fd = timerfd_create();
void create_client_timer(main_loop_t *main_loop)
{
    client_check_source = event_loop_add_timer((event_loop_t *)main_loop->loop,check_client,0,NULL);
    event_source_timer_update(client_check_source,INTERVAL_TIME_CHECK_CLIENT);
}

void stop_checkclient()
{
    if(client_check_source != NULL)
    {
        event_source_remove(client_check_source);
        client_check_source = NULL;
    }
}

int parser_cmd_from_server(char* info)
{
    int len = strlen(info);
    int i = 0;
    if(info[0] != '$') return -1;
    if(info[len-1] != '#') return -1;
    char key[10][3];
    char value[10][30];
    for(i=0;i<10;i++)
    {
        memset(key[i],'\0',3);
        memset(value[i],'\0',30);
    }
    int index = 0;
    int move = 0;
    int check = 0;
    for(i=1;i<len;i++)
    {
        if(info[i] == '&')
        {
            value[index][move] = '\0';
            //printf("index %d value %s\n",index,value[index]);
            move = 0;
            check = 0;
            index++;
            if(index == 10)
            {
                //printf("num is big to 10\n");index = 9;
                break;
            }
        }
        else if(info[i] == '=')
        {
            key[index][move] = '\0';
            //printf("index %d key %s\n",index,key[index]);
            move = 0;
            check = 1;
        }
        else if(info[i] == '#')//ÁªìÊùü
        {
            move = 0;
            check = 0;
        }
        else
        {
            if(check == 0)
            {
                key[index][move] = info[i];
                move++;
            }
            else
            {
                value[index][move] = info[i];
                move++;
            }
        }
    }
    //printf("num:%d \n",index);
    //for(i=0;i<=index;i++)
    //    printf("key:%s value:%s\n",key[i],value[i]);
}

int receiver_func(int fd, void *data,struct epoll_event *ep)
{
    //printf("receive msg from in server\n");

    int i=0;
    int ret = -1;
    int len = MSG_LENGTH;
    char info[len];
    memset(info,0x00,len);
    ret = connection_read(fd,info,len,len);
    if( ret < 0 )
    {
        return -1;
    }
    parser_cmd_from_server(info);

    return 0;

}

int create_insocket(main_loop_t *main_loop)
{
    if(g_insocket > 0) return 1;
    struct sockaddr_un srv_addr;
    g_insocket = socket(PF_UNIX, SOCK_STREAM, 0);
    if(g_insocket<0)
    {
        printf("in socket error\n");
            return -1;
    }
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, UN_AVSERVER_DOMAIN, sizeof(srv_addr.sun_path)-1);
    if(connect(g_insocket, (struct sockaddr *)&srv_addr, sizeof(srv_addr))<0)
    {
        printf("connect insocker  error\n");
            close(g_insocket);g_insocket = -1;
            return -1;
    }
    inserver_receiver_source = event_loop_add_fd((event_loop_t *)main_loop->loop,g_insocket,receiver_func,0,NULL);
    event_source_fd_update(inserver_receiver_source,0);
    return 1;
}


int senddata_to_inserver(void *data, int len)
{
    //printf("send to server:%s\n",(char*)data);
    if(g_insocket < 0) return -1;
#define SZ_4K	4*1024
        while(len>0){
                if(len>SZ_4K){
                        write(g_insocket, data, SZ_4K);
                        len -= SZ_4K;
                        data += SZ_4K;
                }else{
                        write(g_insocket, data, len);
len -= len;
                }
        }
#undef SZ_4K

        return 0;
}

void close_insocket()
{
    if(g_insocket > 0)
    {
        close(g_insocket);
        g_insocket = -1;
    }
}

int com_fd = -1;
int com_init()
{
    printf("com init......\n");
    com_fd = open("/dev/ttyAMA3", O_RDWR | O_NOCTTY);
    if(com_fd < 0)
    {
        printf("open com port error\n");
        return 1;
    }
    struct termios Opt;

    if(tcgetattr(com_fd, &Opt) != 0)
    {
        printf("tcgetattr error\n");
        return 1;
    }
    cfsetispeed(&Opt, B9600);
    cfsetospeed(&Opt, B9600);
    
    /*
    tcflush(com_fd, TCIOFLUSH);
    if(tcsetattr(com_fd, TCSANOW, &Opt) != 0)
    {
        printf("tcsetattr error\n");
        return 1;
    } */

    Opt.c_cflag |= (CLOCAL | CREAD);		//ªÚ¬‘µ˜÷∆Ω‚µ˜∆˜◊¥Ã¨––£¨∆Ù”√Ω” ’
    Opt.c_cflag &= ~CSIZE;
    Opt.c_cflag |= CS8;
    Opt.c_cflag &= ~CSTOPB;					// 1 stop bit
    Opt.c_cflag &= ~PARENB;				 	// Œﬁ∆Ê≈º–£—È

    Opt.c_iflag &= ~(BRKINT | INPCK |ISTRIP | ICRNL | INLCR | IXON | IXOFF | IXANY);
    
    Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN);
    
    Opt.c_oflag &= ~OPOST;

    Opt.c_cc[VTIME] = 0;	// Ω” ’◊Ó∂Ãµ»¥˝ ±º‰
    Opt.c_cc[VMIN] = 0;		// Ω” ’◊Ó…Ÿ∏ˆ ˝
    
    tcflush(com_fd, TCIFLUSH);
    if(tcsetattr(com_fd, TCSANOW, &Opt) != 0)
    {
        printf("tcsetattr fd 2\n");
        return 1;
    }
    
/* #if DEMO_DEF
    printf("do not init  INFRAD\n");
#else
    printf("do  init  INFRAD\n");
    if(init_infrad_gpio() == 0)
    {
        printf("init infrad fd OK\n");
    }
    else
    {
        printf("init infrad fd error\n");
        return 1;
    }
#endif
 */

    return 0;
}

static void osd_callback(int chn, int x, int y, const unsigned char *p, int len)
{
    printf("[%s]-%d:enter chn=%d (%d,%d)\n", __FUNCTION__, __LINE__, chn, x, y);
    
	unsigned char sndbuf[256];
	unsigned int *tmp = (unsigned int *)sndbuf;
	
	*tmp++ = 0xFFFFFFFF;
	*tmp++ = (chn << 16) | x;
	*tmp++ = y;
	*tmp++ = len;
	*tmp++ = 0;
	if (p != NULL && len > 0)
		memcpy(tmp, p, len);
		
	int sndlen = ((unsigned char *)tmp - sndbuf) + len;

	if (g_char_socket < 0) {
		create_char_socket();
	}

    if (g_char_socket > 0) {
        int mb = send(g_char_socket, sndbuf, sndlen, 0);
        if (mb < 0) {
            close(g_char_socket);
            g_char_socket = -1;
            printf("[%s]-%d: send err !\n", __FUNCTION__, __LINE__);
        }
    } else {
        printf("[%s]-%d: com str send err !\n", __FUNCTION__, __LINE__);
    }

	printf("[%s]-%d:exit \n", __FUNCTION__, __LINE__);
}

static unsigned char com_rcv_buf[256];
static unsigned char com_rcv_start = 0;
static unsigned char com_rcv_end = 0;
int com_receiver_func(int fd, void *data, struct epoll_event *ep)
{
	unsigned char rcvbuf[256];
    unsigned char buff[COM_PACKAGE_LEN];
    int i, chnum;
    unsigned check_byte;
	rtc_time_t set_tm;

	char strcurdate[] = "YYYY-MM-DD"; 
	char strcurtime[] = "HH:MM:SS";
	int nread;
	int rtc_fd = -1;
	
    memset(rcvbuf, 0x00, sizeof(rcvbuf));
    if ((nread = read(fd, rcvbuf, sizeof rcvbuf )) <= 0) {
    	printf("[%s]-%d read(com3)\n", __FUNCTION__, __LINE__);
    	return 0;
	}
    
    for (i = 0; i < nread; i++) {
    	printf("%02x ", (unsigned int)rcvbuf[i]);
    	com_rcv_buf[com_rcv_end++] = rcvbuf[i];
    	
    	if (com_rcv_end == com_rcv_start) {
    		printf("com_rcv_buf overflow!\n");
    		com_rcv_end--;
    		break;
    	}
    }
    printf("\n");

    while (com_rcv_start != com_rcv_end) {
    	unsigned char uch = com_rcv_end - com_rcv_start;
    	if (uch < COM_PACKAGE_LEN)
    		break;

		if (com_rcv_buf[com_rcv_start] == 0xFF && com_rcv_buf[(com_rcv_start + 1) % 256] <= 0x03) {
			for (i = 0; i < COM_PACKAGE_LEN; ++i) 
				buff[i] = com_rcv_buf[(com_rcv_start + i) % 256];
				
			for (check_byte = buff[2], i = 3; i < COM_PACKAGE_LEN - 1; ++i)
				check_byte ^= buff[i];
				
			if (check_byte != buff[COM_PACKAGE_LEN - 1]) {
				com_rcv_start++;
				continue;
			}
			
			com_rcv_start += COM_PACKAGE_LEN;
			printf("com_cmd[%02x]\n", (unsigned int)buff[2]);
			switch (buff[2]) {
			case 0x10: //  ±º‰°¢»’∆⁄Œª÷√…Ë÷√√¸¡Ó
				date_y = buff[3] + 1;
				date_x = buff[4];
				time_y = buff[5] + 1;
				time_x = buff[6];
				break;
				
			case 0x20: //  ±º‰–£◊ºASCII: 2003ƒÍ1‘¬5»’8 ±23∑÷00√Îs
				set_tm.year   = (buff[3] - '0') * 1000 + (buff[4] - '0') * 100 + (buff[5] - '0') * 10 + (buff[6] - '0');
				set_tm.month   = (buff[7] - '0') * 10 + (buff[8] - '0');
				set_tm.date = (buff[9] - '0') * 10 + (buff[10] - '0');
				set_tm.hour   = (buff[11] - '0') * 10 + (buff[12] - '0');
				set_tm.minute    = (buff[13] - '0') * 10 + (buff[14] - '0');
				set_tm.second     = (buff[15] - '0') * 10 + (buff[16] - '0');

				rtc_fd = open(RTC_DEV_NAME, O_RDWR);
				if (!rtc_fd) {
					printf("open %s failed\n", RTC_DEV_NAME);
					break;
				}

				if (ioctl(rtc_fd, HI_RTC_SET_TIME, &set_tm) < 0) {
					printf("ioctl: HI_RTC_SET_TIME failed\n");
					close(rtc_fd);
					break;
				}	

				set_time_flag = 1;
				close(rtc_fd);
				break;
				
			case 0x30:
			case 0x31:
				osd_callback(0, 0, 1, NULL, 0);
				if (buff[2] == 0x31)
				    break;
			case 0x32:
				osd_callback(1, 0, 1, NULL, 0);
				if (buff[2] == 0x32)
				    break;
				
				break;

			case 0x40:
			case 0x41: //y = buff[3], x = buff[4], y != 0
				for (i = 0; i < COM_PACKAGE_LEN - 8; ++i) {
					if (buff[7 + i] <= ' ')
					    buff[7 + i] = ' ';
				}
				osd_callback(0, buff[4] * 24, (buff[3] & 0x0F) + 1, buff + 7, COM_PACKAGE_LEN - 8);
				if (buff[2] == 0x41)
					break;
			case 0x42:
				for (i = 0; i < COM_PACKAGE_LEN - 8; ++i) {
					if (buff[7 + i] <= ' ')
					    buff[7 + i] = ' ';
				}
				osd_callback(1, buff[4] * 24, (buff[3] & 0x0F) + 1, buff + 7, COM_PACKAGE_LEN - 8);
				if (buff[2] == 0x42)
					break;
			
				break;

			case 0x70:	// ±≥æ∞…Ë÷√ø™πÿ
			case 0x71:
			case 0x72:
				break;

			case 0x80:	// ±º‰œ‘ æø™πÿ
			case 0x81:
				if (buff[3] == 0) {
					ch1_time_osd = 1;
				} else {
					ch1_time_osd = 0;
					memset(strcurdate, ' ', strlen(strcurdate));
					memset(strcurtime, ' ', strlen(strcurtime));
					osd_callback(0, date_x * 24, date_y, strcurdate, strlen(strcurdate));
					osd_callback(0, time_x * 24, time_y, strcurtime, strlen(strcurtime));
				}
				if (buff[2] == 0x81) break;
			case 0x82:
				if (buff[3] == 0) {
					ch2_time_osd = 1;
				} else {
					ch2_time_osd = 0;
					memset(strcurdate, ' ', strlen(strcurdate));
					memset(strcurtime, ' ', strlen(strcurtime));
					osd_callback(1, date_x * 24, date_y, strcurdate, strlen(strcurdate));
					osd_callback(1, time_x * 24, time_y, strcurtime, strlen(strcurtime));
				}
				if (buff[2] == 0x82) break;
				
				break;
				
			default:
				printf("channel err!\n");
				break;
			}
		} else {
			com_rcv_start++;
		}
    }

    return 0;
}



int com_close()
{
    printf("close com....\n");
    if(com_fd>0)
    {
        close(com_fd);
        com_fd = -1;
    }
    return 0;
}

#if 0
int index_com_read = 0;
int com_read_check = 0;
char com_info[64];
//g_char_socket
void parser_cmd_from_com(char* info,int length)
{
    //printf("parser_cmd_from_com....,length:%d\n",length);
    int mb = 0;
    /*for( mb=0;mb<length;mb++)
    {
        printf("%02X ",info[mb]);
    }
    printf("\n");*/
    int len = info[2];
    int cmd = info[3];
    int x = info[4]*20;
    int y = info[5];
    int channel = 0;
    unsigned char endinfo[1024];
    memset(endinfo,0x00,sizeof(endinfo));
    printf("cmd :%02X\n",cmd);
    unsigned int tmp1;
    unsigned int* X =(unsigned int*) endinfo;
    int num = 1;
    int set_or_clear = 0;//0 set 1 clear
    int i = 0;
    switch(cmd)
    {
    case 0xDD:
        num = 1;
        channel = 0;
        set_or_clear =0;
        break;
    case 0xED:
        num = 1;
        channel = 1;
        set_or_clear = 0;
        break;
    case 0xFD:
        num = 2;
        set_or_clear = 0;
        break;
    case 0xDC:
        num = 1;
        channel = 0;
        set_or_clear = 1;
        x = 0;
        y = 1;
        len = 3;
        break;
    case 0xEC:
        num = 1;
        channel = 1;
        set_or_clear = 1;
        x = 0;
        y = 1;
        len = 3;
        break;
    case 0xF0:
        num = 2;
        set_or_clear = 1;
        x = 0;
        y = 1;
        len = 3;
        break;
    default:
        printf("the cmd is not support\n");
        return;

    }
    //if(set_or_clear == 0)
    {
        if(num == 1)
        {
            X =(unsigned int*) endinfo;
            tmp1 = (x | ((channel)<<16));
            *X = tmp1;
            X =(unsigned int*) (endinfo+4);
            *X = y;
            X =(unsigned int*) (endinfo+8);
            *X = len-3;
            X =(unsigned int*) (endinfo+12);
            *X = 0;

            memcpy(endinfo+16, info+6, len-3);
			 if (g_char_socket < 0) {
            	create_char_socket();
            }

            //printf("send 1 %s %d\n",info+6,strlen(endinfo));
            if(g_char_socket)
            {
                mb = send(g_char_socket,endinfo,16+len-3,0);
                //printf("send char infomation to avserver %d\n",mb);
                if(mb < 0)
                {
                    close(g_char_socket);
                    g_char_socket = -1;
                }
            }
        }
        else
        {
            for( i=0;i<num;i++)
            {
                X =(unsigned int*) endinfo;
                tmp1 = (x | ((i)<<16));
                *X = tmp1;
                X =(unsigned int*) (endinfo+4);
                *X = y;
                X =(unsigned int*) (endinfo+8);
                *X = len-3;
                X =(unsigned int*) (endinfo+12);
                *X = 0;
                memcpy(endinfo+16,info+6,len-3);
                //printf("send 2 %s \n",info+6);
                if (g_char_socket < 0) {
                  create_char_socket();
                }

                if (g_char_socket > 0) {
                    mb = send(g_char_socket, endinfo, 16+len-3, 0);
                    //printf("send char infomation to avserver %d\n",mb);
                    if (mb < 0) {
                        close(g_char_socket);
							g_char_socket = -1;
                    }
                }
            }
        }
    }
}

int com_receiver_func(int fd, void *data, struct epoll_event *ep)
{
    //printf("receive com message  from caojj\n");
    int index = 0;
    int tmpread = 0;
    char buff[256];
    memset(buff,0x00,sizeof(buff));
    int nread = read(fd, buff, sizeof(buff));
    
    //printf("com readlength=%d\n", nread);
    if(nread <= 0) return 0;
    buff[nread]='\0';
    if((buff[0] == 0xDD) && (buff[1] == 0xBB))
    {
        index_com_read = 0;
        com_read_check = buff[2];
        printf("1 new com cmd data coming, length:%d\n",buff[2]);
    }
    else if(nread > com_read_check)
    {
        printf("the cmd is mix\n");
        for(index=0;index<nread;index++)
        {
            if((buff[index] == 0xDD) && (buff[index+1] == 0xBB))
            {
                index_com_read = 0;
                com_read_check = buff[index+2];
                printf("2 new com cmd data coming, length:%d\n",buff[index+2]);
                break;
            }
        }
    }
    if(index == nread) return 0;
    if((index+3+com_read_check)>nread) tmpread = nread;
    else tmpread = index+3+com_read_check;
	
    for(index=index;index<tmpread;index++) {
        com_info[index_com_read]=buff[index];
        index_com_read++;
    }
		
    if(index_com_read<com_read_check+3) return 1;
    index_com_read = 0;
    parser_cmd_from_com(com_info,com_read_check+3);
   /* ptz_command_t st;
    int i = 0;
    st.value1 = com_info[i];i++;
    st.value2 = com_info[i];i++;
    st.value3 = com_info[i];i++;
    st.value4 = com_info[i];i++;
    st.value5 = com_info[i];i++;
    st.value6 = com_info[i];i++;
    st.value7 = com_info[i];



    if((st.value1 == 0xff)&&(st.value2 == 0x01)&&(st.value3 == 0xAA)&&(st.value4 == 0x00)&&(st.value6 == 0x01))
    {
        //
        i = 0;
        char* info = "1:1";
        for(i=0;i<MAX_CLIENT;i++)
        {
            if(clients[i].kind == 3)
            {

                int x = send(clients[i].fd,info,strlen(info),0);printf("send check infomation to SDK %d\n",x);
            }
        }
        return 0;
    }*/
    return 1;

}
#endif 


void create_com_monior(main_loop_t *main_loop)
{
    if(com_fd <= 0)
        com_init();
    if(com_fd <= 0)
        return;
    printf("and com monitor to listen...\n");

    g_com_source = event_loop_add_fd((event_loop_t *)main_loop->loop,com_fd,com_receiver_func,0,NULL);

    if(g_com_source == NULL) printf("add com fd error\n");
    event_source_fd_update(g_com_source,0);
}

void stop_com_monior()
{
    if(g_com_source != NULL)
    {
        event_source_remove(g_com_source);
        g_com_source = NULL;
    }
    com_close();
}

//TCP Client : 127.0.0.1:20051
#define PORT_SETCHAR  20051
int create_char_socket()
{
    //struct sockaddr_in srv_addr;
    //bzero(&srv_addr, sizeof(srv_addr));
    g_char_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_char_socket < 0)
    {
        printf("[%s]-%d socket err!\n", __FUNCTION__, __LINE__);
        return -1;
    }
    //srv_addr.sin_family = AF_INET;
    //srv_addr.sin_port = htons(PORT_SETCHAR);
    struct sockaddr_in des_addr;
    bzero(&des_addr, sizeof(des_addr));
    des_addr.sin_family = AF_INET;
    des_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    //des_addr.sin_addr.s_addr = inet_addr(inet_ntoa(boardcast_ser.sin_addr));
    des_addr.sin_port = htons(PORT_SETCHAR);
    if (connect(g_char_socket, (struct sockaddr *)&des_addr, sizeof(des_addr)) < 0)
    {
        printf("connect for set char error\n");
        close(g_char_socket);
        g_char_socket = -1;
        return -1;
    }
	printf("g_char_socket connect ok!\n");
    //receiver_source =event_loop_add_fd((event_loop_t *)application_eventloop(app), g_insocket,receiver_func, NULL);
    //event_source_fd_update(receiver_source);
    return 0;
}

int close_char_socket()
{
    if(g_char_socket>0)
    {
        close(g_char_socket);
        g_char_socket = -1;

    }
    return 0;
}
