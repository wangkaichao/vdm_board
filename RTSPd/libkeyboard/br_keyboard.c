#include "br_keyboard.h"
#include "indefine.h"

#define BORADCAST_PORT (8900)

#define CONFIG_FILE_NETKEYBOARD_ID "/mnt/mtd/netkeyboard_id.cfg"
void load_netkeyboard_id()
{
    char name[20];
    char value[20];

    FILE *cfgfp = NULL;
    cfgfp = fopen(CONFIG_FILE_NETKEYBOARD_ID,"r+");
    if(cfgfp == NULL)
    {printf("open netkeyboard_id config file error\n");g_device_id=1;return;}
    while(feof(cfgfp) == 0)
    {
        memset(name,0x00,sizeof(name));
        memset(value,0x00,sizeof(value));
        fscanf(cfgfp,"%s %s",name,value);
        if(!strcmp(name,"netkeyboard_id"))
        {
            g_device_id= atoi(value);
        }
    }
    printf("netkeyboard_id :%d\n",g_device_id);
     fclose(cfgfp);
}

int recv_boradcast_creat()//liz  2015
{
//	char buf[256];
    struct sockaddr_in serv_addr,addr;
    unsigned char net_key_rec_buf[256];
    int bd;
    int ret;
    int len;
    int sockfd;


    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        printf("Error: socket");
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(BORADCAST_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("192.168.2.249");//htonl(INADDR_ANY);//htonl(INADDR_ANY)

    bd = bind(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));


    {
        struct  linger  linger_set;
        struct timeval stTimeoutVal;
        linger_set.l_onoff = 1;
        linger_set.l_linger = 0;

        if (setsockopt( sockfd, SOL_SOCKET, SO_LINGER , (char *)&linger_set, sizeof(struct  linger) ) < 0 )
        {
            printf( "setsockopt SO_LINGER  error\n" );
        }

        stTimeoutVal.tv_sec = 100;
        stTimeoutVal.tv_usec = 0;
        if ( setsockopt( sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&stTimeoutVal, sizeof(struct timeval) ) < 0 )
        {
            printf( "setsockopt SO_SNDTIMEO error\n" );
        }

        if ( setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&stTimeoutVal, sizeof(struct timeval) ) < 0 )
        {
            printf( "setsockopt SO_RCVTIMEO error\n" );
        }
    }


    printf("recv boardcast thread start!!\n");

    while(1)
    {
        usleep(8000);

        len = sizeof(serv_addr);
        memset(&serv_addr, 0, len);
        memset(net_key_rec_buf,0,256);

        ret = recvfrom(sockfd,net_key_rec_buf,sizeof(net_key_rec_buf),0,(struct sockaddr*)&serv_addr,(socklen_t*)&len);

        if( ret > 0 )
        {
            //printf("************** net_key_rec_buf[2] = 0x%x\n",net_key_rec_buf[2]);
            //printf("SysMenu.RemoteID:%d \n",SysMenu.RemoteID);
            if((net_key_rec_buf[0] != 0xa5) ||(net_key_rec_buf[1] != 0x5a))
            {
                printf("other info\n");
                continue;
            }
            if((net_key_rec_buf[2]==(g_device_id))||(net_key_rec_buf[2] == 0xff))
            {
		    switch(net_key_rec_buf[3])
		    {
		    case 0xc1:
			g_key_fun(4);
			break;
		    case 0xc8:
			g_key_fun(net_key_rec_buf[4]-1);
			break;
		    default:
			printf("no define\n");
			break;
		    }
            }

        }else
        {
            //printf("time out!\n");
        }

    }

    if( sockfd > 0)
    {
        close(sockfd);
        sockfd = 0;
    }

    return 1;

}

int BRLib_Keyboard_Version()
{
    return VERSION;
}

void BRLib_Keyboard_Init(int debug_or_release)
{
    g_debug_or_release = debug_or_release;
    load_netkeyboard_id();

}


void BRLib_Keyboard_Start(callback_key_fun f)
{
    g_key_fun = f;
    if(!f) return;

    pthread_t thread;
    int ret = pthread_create(&thread, NULL, (void *)recv_boradcast_creat, NULL);

    if ( ret<0 )
    {
        printf( "recv_boradcast_creat create error\n");
        return;
    }

}

void BRLib_Keyboard_Exit()
{
    if(g_key_fun) g_key_fun = NULL;
}


