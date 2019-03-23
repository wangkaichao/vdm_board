#ifndef SELFSERVER_H
#define SELFSERVER_H

#include "eventloop.h"
#include "util.h"
#include "boardcast.h"
#include "cmd_type.h"
#define	UN_AVSERVER_DOMAIN	"/tmp/avserver.domain"
#define CONFIG_FILE_PATH   "/mnt/mtd/if.cfg"
#define CONFIG_FILE_VERSION "/mnt/mtd/Version"
#define CONFIG_FILE_PATH_TMP   "/mnt/mtd/if_tmp.cfg"
#define CONFIG_FILE_XY "/mnt/mtd/xy.cfg"
#define CONFIG_FILE_XY_tmp "/mnt/mtd/xy_tmp.cfg"

#define CONFIG_FILE_UID "/mnt/mtd/uid.cfg"
#define CONFIG_FILE_UID_TMP "/mnt/mtd/uid_tmp.cfg"


#define CONFIG_FILE_NETKEYBOARDID "/mnt/mtd/netkeyboard_id.cfg"
#define CONFIG_FILE_NETKEYBOARDID_TMP "/mnt/mtd/netkeyboard_id_tmp.cfg"

#define CONFIG_FILE_BOARD_IPS "/mnt/mtd/iptable.txt"
#define CONFIG_FILE_BOARD_IPS_TMP "/mnt/mtd/iptable_tmp.txt"
typedef struct config_value_st
{
    char mac[32];
    char ip[32];
    char netmask[32];
    char version[10];

    int x;
    int y;

    char uid[64];
	int netkeyboard_id;

    char ips[4][300];
}config_value;

#define SELF_SERVER_PORT 16101

void stop_selfserver();
int create_selfserver(main_loop_t *main_loop);

void create_client_timer(main_loop_t *main_loop);
void stop_checkclient();

int create_insocket(main_loop_t *main_loop);
void close_insocket();
int senddata_to_inserver(void *data, int len);
int parser_cmd_from_server(char* info);


extern int com_fd;
int com_init();
int com_close();
void create_com_monior(main_loop_t *main_loop);
void stop_com_monior();
int create_char_socket();
int close_char_socket();
#endif
