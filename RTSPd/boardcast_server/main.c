
#include "boardcast.h"
#include "selfserver.h"
config_value cfg_v;

#define CONFIG_FILE_NETKEYBOARD_ID "/mnt/mtd/netkeyboard_id.cfg"
void load_netkeyboard_id()
{
    char name[20];
    char value[20];

    FILE *cfgfp = NULL;
    cfgfp = fopen(CONFIG_FILE_NETKEYBOARD_ID,"r+");
    if(cfgfp == NULL)
    {printf("open netkeyboard_id config file error\n");cfg_v.netkeyboard_id=1;return;}
	
    while(feof(cfgfp) == 0)
    {
        memset(name,0x00,sizeof(name));
        memset(value,0x00,sizeof(value));
        fscanf(cfgfp,"%s %s",name,value);
        if(!strcmp(name,"netkeyboard_id"))
        {
            cfg_v.netkeyboard_id= atoi(value);
        }
    }
    printf("netkeyboard_id :%d\n",cfg_v.netkeyboard_id);
    fclose(cfgfp);
}

//加载IP表"/mnt/mtd/iptable.txt"
int urls_num = 0;
void load_iptable()
{
    urls_num = 0;
    int i = 0;
    for(i=0;i<4;i++)
    {
        memset(cfg_v.ips[i],0x00,300);
    }

    char info[300];
    FILE *cfgfp = NULL;
    cfgfp = fopen(CONFIG_FILE_BOARD_IPS,"r");
    if(cfgfp == NULL)
    {printf("open uid config file error\n");return;}
    i = 0;
    while(feof(cfgfp) == 0)
    {
        memset(info,0x00,sizeof(info));
        fgets(info,300,cfgfp);
        //fscanf(cfgfp,"%s ",info);
        if(strlen(info) < 5) break;
        //printf(" ip:%s\n",info);
        strcpy(cfg_v.ips[i],info);
        printf("iptable %d:%s\n",i,cfg_v.ips[i]);
        i++;
        if(i==4) break;
    }
     fclose(cfgfp);urls_num = i;
}

void loadconfig_xy()
{
    cfg_v.x = cfg_v.y = 0;

    char name[8];
    char value[8];

    FILE *cfgfp = NULL;
    cfgfp = fopen(CONFIG_FILE_XY,"r");
    if(cfgfp == NULL)
    {printf("open xy config file error\n");return;}
    while(feof(cfgfp) == 0)
    {
        memset(name,0x00,sizeof(name));
        memset(value,0x00,sizeof(value));
        fscanf(cfgfp,"%s %s",name,value);
        if(!strcmp(name,"x"))
        {
            cfg_v.x = atoi(value);
        }
        if(!strcmp(name,"y"))
        {
            cfg_v.y = atoi(value);
        }
    }
    printf("old x:%d\n",cfg_v.x);
    printf("old y:%d\n",cfg_v.y);
    fclose(cfgfp);
}
void loaduserid()
{
    memset(cfg_v.uid,0x00,sizeof(cfg_v.uid));
    char uid[64];
    FILE *cfgfp = NULL;
    cfgfp = fopen(CONFIG_FILE_UID,"r");
    if(cfgfp == NULL)
    {printf("open uid config file error\n");return;}
    while(feof(cfgfp) == 0)
    {
        memset(uid,0x00,sizeof(uid));
        fscanf(cfgfp,"%s",uid);
        if(strlen(uid) < 1) break;
        printf(" uid:%s\n",uid);
        strcpy(cfg_v.uid,uid);
    }
     fclose(cfgfp);
}

//加载IP、MAC信息"/mnt/mtd/if.cfg"
void loadconfig()
{
   // printf("%s\n",CONFIG_FILE_PATH);
    memset(cfg_v.ip,0x00,sizeof(cfg_v.ip));
    memset(cfg_v.mac,0x00,sizeof(cfg_v.mac));
    char name[32];
    char value[32];
    FILE *cfgfp = NULL;
    cfgfp = fopen(CONFIG_FILE_PATH,"r");
    if(cfgfp == NULL)
    {printf("open config file error\n");return;}
    while(feof(cfgfp) == 0)
    {
        memset(name,0x00,sizeof(name));
        memset(value,0x00,sizeof(value));
        fscanf(cfgfp,"%s %s",name,value);
        //printf(" name:%s\n",name);
        //printf(" value:%s\n",value);
        if(!strcmp(name,"ip"))
        {
            strcpy(cfg_v.ip,value);
        }
        if (!strcmp(name, "netmask"))
        {
        	strcpy(cfg_v.netmask, value);
        }
        if(!strcmp(name,"mac"))
        {
            strcpy(cfg_v.mac,value);
        }
    }

     fclose(cfgfp);
}

//加载软件版本"/mnt/mtd/Version"
void loadVersionConfig()
{
    memset(cfg_v.version,0x00,sizeof(cfg_v.version));
    char version[10];
    FILE *cfgfp = NULL;
    cfgfp = fopen(CONFIG_FILE_VERSION,"r");
    if(cfgfp == NULL)
    {printf("open version config file error\n");return;}
    while(feof(cfgfp) == 0)
    {
        memset(version,0x00,sizeof(version));
        fscanf(cfgfp,"%s",version);
        if(strlen(version) < 1) break;
        printf(" version:%s\n",version);
        strcpy(cfg_v.version,version);
    }
     fclose(cfgfp);
}

int main(int argc,char* argv[])
{
    load_iptable();
    loadconfig();
    loadVersionConfig();
    loadconfig_xy();
    loaduserid();
    load_netkeyboard_id();

    //用于组播搜索的，添加一个路由项
    //最早在  IPC上面做得实现，你那边可以考虑关掉
    system("/sbin/route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0");

    main_loop_t* main_loop = (main_loop_t*)malloc(sizeof(main_loop_t));
    main_loop->loop = NULL;
    main_loop->running = 1;
    main_loop->loop = event_loop_create();
    if(NULL ==main_loop->loop ) {
		printf("init loop error\n");
		exit(-1);
    }
    create_boardcast_worker(main_loop);	//s, boardcast_source: bind 224.0.1.2:5001 UDP [group address]
    create_boardcast_worker_1(main_loop);	//s1, boardcast_source_1: bind eth0:6789 UDP
    create_client_timer(main_loop);			//fd, client_check_source: 1s timer
    create_selfserver(main_loop);			//server_fd, server_source: bind INADDR_ANY:16101 TCP,  add client_fd, client_source
    create_com_monior(main_loop);			//com_fd, g_com_source: uart3,9600,no parity
    //create_char_socket();					//g_char_socket: connect 127.0.0.1:20051 TCP

     while(main_loop->running)
         event_loop_dispatch(main_loop->loop, -1);
	 
     stop_boardcast(); 	//free boardcast_source, but not close(s)
     stop_boardcast_1();	//free boardcast_source_1, but not close(s1)
     stop_selfserver();		//free server_source, close(server_fd),  free client_source, close(client_fd)
     stop_checkclient();	//client_check_source, but not close(fd)
     stop_com_monior();	//g_com_source, but not close(com_fd)
     
     close_char_socket();	//close(g_char_socket)
     event_loop_destroy(main_loop->loop);
     free(main_loop);
    return 0;
}
