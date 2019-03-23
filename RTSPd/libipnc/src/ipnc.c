

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "ipnc.h"

char *ptz_command[] = {
	"left",
	"right",
	"up",
	"down",
	"home",
	"zoomin",
	"zoomout",
	"hscan",
	"vscan",
	"stop",
	"focusin",
	"focusout",
	"aperturein",
	"apertureout",
	"auto",
	"brush",
	"light"
};

char *devtype_strings[] = {
//	"CMOS 720P / PTZ_485",
//	"CMOS 1080P / PTZ_485"
		"720P IPCAM",
		"1080P IPCAM"
};

char *SDstatus[] = {
	"Out",
	"Ready",
	"Readonly"
};

time_zone_t Time_Zone[] = {
	{"Etc", -720},
	{"Pacific/Apia", -660},
	{"Pacific/Honolulu", -600},
	{"America/Anchorage", -540},
	{"America/Los_Angeles", -480},
	{"America/Denver", -420},
	{"America/Tegucigalpa", -420},
	{"America/Phoenix", -420},
	{"America/Winnipeg", -360},
	{"America/Mexico_City", -360},
	{"America/Chicago", -360},
	{"America/Costa_Rica", -360},
	{"America/Indianapolis", -300},
	{"America/New_York", -300},
	{"America/Bogota", -300},
	{"America/Santiago", -240},
	{"America/Caracas", -240},
	{"America/Montreal", -240},
	{"America/St_Johns", -210},
	{"America/Thule", -180},
	{"America/Buenos_Aires", -180},
	{"America/Sao_Paulo", -180},
	{"Atlantic/South_Georgia", -120},
	{"Atlantic/Cape_Verde", -60},
	{"Atlantic/Azores", -60},
	{"Europe/Dublin", 0},
	{"Africa/Casablanca", 0},
	{"Europe/Amsterdam", 60},
	{"Europe/Belgrade", 60},
	{"Europe/Brussels", 60},
	{"Europe/Warsaw", 60},
	{"Africa/Lagos", 60},
	{"Europe/Athens", 120},
	{"Europe/Bucharest", 120},
	{"Africa/Cairo", 120},
	{"Africa/Harare", 120},
	{"Europe/Helsinki", 120},
	{"Asia/Jerusalem", 120},
	{"Asia/Baghdad", 180},
	{"Asia/Kuwait", 180},
	{"Europe/Moscow", 180},
	{"Africa/Nairobi", 180},
	{"Asia/Tehran", 210},
	{"Asia/Dubai", 240},
	{"Asia/Baku", 240},
	{"Asia/Kabul", 270},
	{"Asia/Yekaterinburg", 300},
	{"Asia/Karachi", 300},
	{"Asia/Calcutta", 330},
	{"Asia/Katmandu", 345},
	{"Asia/Almaty", 360},
	{"Asia/Dhaka", 360},
	{"Asia/Colombo", 360},
	{"Asia/Rangoon", 390},
	{"Asia/Bangkok", 420},
	{"Asia/Krasnoyarsk", 420},
	{"Asia/Hong_Kong", 480},
	{"Asia/Irkutsk", 480},
	{"Asia/Kuala_Lumpur", 480},
	{"Australia/Perth", 480},
	{"Asia/Taipei", 480},
	{"Asia/Tokyo", 540},
	{"Asia/Seoul", 540},
	{"Asia/Yakutsk", 540},
	{"Australia/Adelaide", 570},
	{"Australia/Brisbane", 600},
	{"Australia/Sydney", 600},
	{"Pacific/Guam", 600},
	{"Australia/Hobart", 600},
	{"Asia/Vladivostok", 600},
	{"Asia/Magadan", 660},
	{"Pacific/Auckland", 720},
	{"Pacific/Fiji", 720},
	{"Pacific/Tongatapu", 780},
	{"end", 0}
};

Cmd_compare cmd_compare_value[] = {
	{"setvideoattr", cmd_av},
	{"setvencattr", cmd_enc},
	{"setimageattr", cmd_imgattr},
	{"setoverlayattr", cmd_overlay},
	{"setmobilesnapattr", cmd_mbsnapattr},
	{"setaencattr", cmd_aencattr},
	{"setaudioinvolume", cmd_audioV},
	{"setinfrared", cmd_infrared},
	{"setnetattr", cmd_netattr},
	{"sethttpport", cmd_httpport},
	{"setrtspport", cmd_rtspport},
	{"setupnpattr", cmd_upnpattr},
	{"setwirelessattr", cmd_wfattr},
	{"searchwireless", cmd_scwf},
	{"chkwirelessattr", cmd_ckwf},
	{"set3thddnsattr", cmd_ddnsattr},
	{"setptzcomattr", cmd_ptzattr},
	{"setmdattr", cmd_mdattr},
	{"setioattr", cmd_ioattr},
	{"setmdalarm", cmd_mdalarm},
	{"setrelayattr", cmd_relayattr},
	{"setschedule", cmd_schedule},
	{"setuserattr", cmd_userattr},
	{"setsnaptimerattr", cmd_snaptimer},
	{"setftpattr", cmd_ftpattr},
	{"setsmtpattr", cmd_smtpattr},
	{"setservertime", cmd_servertime},
	{"setntpattr", cmd_ntpattr},
	{NULL, cmd_end}
};

User_file_t stUser_file_default = {
	3,
	{
		{"admin", "admin", em_Usergroup_admin},
		{"user", "user", em_Usergroup_user},
		{"guest", "guest", em_Usergroup_guest}
	}
};

int iFW_default = 50;

Img_cfg_t stImgattr_file_default = {
	50,
	50,
	50,
	50,
	0,
	50,
	0,
	{10, 30},
	{128, 128, 128}
};

Osd_cfg_t arrstOsdcfg_default[REGION_NUM] = {
	{
		TIMER_REGION,
		1,
		10,
		10,
		100,
		1,
		"time region"
	},
	{
		1,
		0,
		1000,
		10,
		100,
		1,
		"win1"
	},
	{
		2,
		0,
		1000,
		30,
		100,
		1,
		"win2"
	}
};

Infrared_cfg_t stInfrared_default = {
	0
};

Net_cfg_t stNetcfg_default = {
	0,
	"192.168.1.233",
	"255.255.0.0",
	"192.168.1.1",
	0,
	"202.96.134.133",
	"202.96.134.133",
	{0x00,0x50,0x80,0x11,0x22,0x33},
	0
};

Port_cfg_t stPortcfg_default = {
	8001,
	554
};

Upnp_cfg_t stUpnpcfg_default = {
	0
};

Wf_cfg_t stWFcfg_default = {
	0,
	"linksys",
	2,
	"1234567890",
	0,
	0
};

DDNS_cfg_t stDDNScfg_default = {
	0,
	1,
	"simple",
	"12345",
	"simple.3322.org"
};

Ptz_cfg_t stPtzcfg_default = {
	0,
	0x01,
	9600,
	8,
	1,
	0
};

Md_cfg_t stMdcfg_default = {
	{
		{0, 1, 0, 10, 10, 10, 10, 0},
		{0, 1, 1, 20, 20, 20, 20, 0},
		{0, 1, 2, 30, 30, 30, 30, 0},
		{0, 1, 3, 40, 40, 40, 40, 0}
	},

		0,
		0,

		0,
		0,
		0,
		0,

		0,
		10,

	{
		0, 
			0,
			0,
		{0,0},
		{0,0},
		{
			{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}
		}
	}
};

Snaptimer_cfg_t stSnaptimercfg_default = {
	0,
	60,
	1,
};

Ft_cfg_t stFtcfg_default = {
	"192.168.1.2",
	21,
	"aaronqiu",
	"123456",
	1,
	"BR_pic"
};

Smtp_cfg_t stSmtpcfg_default = {
	"smtp.brgq.com.cn",
	25,
	0,
	1,
	"aaronqiu",
	"123456",
	"aaron@brgq.com.cn",
	"simple@brgq.com.cn",
	"pic",
	"snap pic"
};

VidMask_cfg_t stVidMaskcfg_default = {
	0,
{
	{
		0, 10, 10, 100, 100, 1
	},
	{
		0, 100, 100, 100, 100, 1
		},
		{
			0, 200, 200, 100, 100, 1
		}
}
};

Sys_cfg_t stSyscfg_default = {
	0,
	"0",
	"1.0.0.0",
	"2.0.0.0",
	"ipcam",
	"2012-4-25 00:00:00",
	0,
	0,
	0,
	0
};

Time_cfg_t stTimecfg_default = {
	56,//HK
	0,
	0,
	"192.168.1.2",
	60
};

Ptz_preset_t stPtz_preset_defualt = {
		{{1, 0, ""},{2, 0, ""}},
		{
			{0}, {0}, {0}
		}
};

Cloud_cfg_t stCloud_cfg = {
	0,
	"st_ipc",
	"admin",
	8117,
	"115.28.238.224",
	"ST_IPCAM"
};

Rec_cfg_t stRec_cfg_default = {
	0,
	5,
	0
};

static pthread_mutex_t wr_mutex;

void libipc_init()
{
	pthread_mutex_init(&wr_mutex, NULL);
}

int read_cfg_file(char *filename, void *buf, const int buflen)
{
	FILE *fp;
	int readlen = -1;

	if(filename==NULL||buf==NULL){
		return PARAM_ERROR;
	}

	printf("read cfg %s.\n", filename);

	if(access(filename, F_OK)!=0){
		default_setting(filename);
	}

	fp = fopen(filename, "r");
	if(fp==NULL){
		return CFG_FILE_OPEN_ERROR;
	}
	readlen = fread(buf, 1, buflen, fp);
/*	if(readlen!=buflen){
		fclose(fp);
		return CFG_FILE_READ_ERROR;
	}
*/
	fclose(fp);

	return READ_CFG_FILE_OK;
}

int write_cfg_file(char *filename, void *buf, const int buflen)
{
	FILE *fp;
	int writeLen = -1;
	char cmd[128];

	pthread_mutex_lock(&wr_mutex);

	printf("----------------------------------write cfg file: %s.\n", filename);

	fp = fopen("/tmp/writeconf.tmp", "w");
	if(fp==NULL){
		pthread_mutex_unlock(&wr_mutex);
		return CFG_FILE_WRITE_ERROR;
	}

	writeLen = fwrite(buf, 1, buflen, fp);
/*	
	if(writeLen!=buflen){
		fclose(fp);
		return CFG_FILE_WRITE_ERROR;
	}
*/
	fflush(fp);
	fclose(fp);

	sprintf(cmd, "mv /tmp/writeconf.tmp %s", filename);
	system(cmd);
	usleep(200);

	pthread_mutex_unlock(&wr_mutex);

	return WRITE_CFG_FILE_OK;
}

static int setUserDefualt()
{
	FILE *fp;
	User_file_t stUser_file;
	char cmd[100];
	int i;

	fp = fopen("/tmp/user.tmp", "w");
	if(fp==NULL){
		DBG_ERR("setUserDefualt: fopen failed.\n");
		return -1;
	}

	memset(&stUser_file, 0, sizeof(User_file_t));
	stUser_file.user_num = stUser_file_default.user_num;
	for(i=0; i<stUser_file.user_num; i++){
		memcpy(&(stUser_file.stUser[i].username[0]), &(stUser_file_default.stUser[i].username[0]), sizeof(stUser_file_default.stUser[i].username));
		memcpy(&(stUser_file.stUser[i].password[0]), &(stUser_file_default.stUser[i].password[0]), sizeof(stUser_file_default.stUser[i].password));
		stUser_file.stUser[i].group = stUser_file_default.stUser[i].group;
	}

	fwrite(&stUser_file, 1, sizeof(stUser_file), fp);
	fflush(fp);
	fclose(fp);

	sprintf(cmd, "mv /tmp/user.tmp %s", USER_FILE);
	system(cmd);

	return 0;
}

static int setDefault(char *filename, void *buf, int len)
{
	return write_cfg_file(filename, buf, len);
}

int default_setting(char *conf_path)
{
	if(!strcmp(conf_path, USER_FILE)){
		return setUserDefualt();
	}/*else if(!strcmp(conf_path, )){
		return setDefault(ENC_FW_FILE, &, sizeof(iFW_default));

	}else if(!strcmp(conf_path, )){
		return setDefault(ENC_FW_FILE, &, sizeof(iFW_default));

	}*/else if(!strcmp(conf_path, ENC_FW_FILE)){
		return setDefault(ENC_FW_FILE, &iFW_default, sizeof(iFW_default));
	}else if(!strcmp(conf_path, IMAGE_ATTR_FILE)){
		return setDefault(IMAGE_ATTR_FILE, &stImgattr_file_default, sizeof(stImgattr_file_default));
	}else if(!strcmp(conf_path, OVERLAY_CFG_FILE)){
		return setDefault(OVERLAY_CFG_FILE, arrstOsdcfg_default, sizeof(arrstOsdcfg_default));
	}else if(!strcmp(conf_path, INFRARED_CFG_FILE)){
		return setDefault(INFRARED_CFG_FILE, &stInfrared_default, sizeof(stInfrared_default));
	}else if(!strcmp(conf_path, NET_CFG_FILE)){
		return setDefault(NET_CFG_FILE, &stNetcfg_default, sizeof(stNetcfg_default));
	}else if(!strcmp(conf_path, PORT_CFG_FILE)){
		return setDefault(PORT_CFG_FILE, &stPortcfg_default, sizeof(stPortcfg_default));
	}else if(!strcmp(conf_path, UPNP_CFG_FILE)){
		return setDefault(UPNP_CFG_FILE, &stUpnpcfg_default, sizeof(stUpnpcfg_default));
	}else if(!strcmp(conf_path, WF_CFG_FILE)){
		return setDefault(WF_CFG_FILE, &stWFcfg_default, sizeof(stWFcfg_default));
	}else if(!strcmp(conf_path, DDNS_CFG_FILE)){
		return setDefault(DDNS_CFG_FILE, &stDDNScfg_default, sizeof(stDDNScfg_default));
	}else if(!strcmp(conf_path, PTZCOM_CFG_FILE)){
		return setDefault(PTZCOM_CFG_FILE, &stPtzcfg_default, sizeof(stPtzcfg_default));
	}else if(!strcmp(conf_path, MD_CFG_FILE)){
		return setDefault(MD_CFG_FILE, &stMdcfg_default, sizeof(stMdcfg_default));
	}else if(!strcmp(conf_path, USER_FILE)){
		return setUserDefualt();
	}else if(!strcmp(conf_path, SNAPTIMER_CFG_FILE)){
		return setDefault(SNAPTIMER_CFG_FILE, &stSnaptimercfg_default, sizeof(stSnaptimercfg_default));
	}else if(!strcmp(conf_path, FTP_CFG_FILE)){
		return setDefault(FTP_CFG_FILE, &stFtcfg_default, sizeof(stFtcfg_default));
	}else if(!strcmp(conf_path, SMTP_CFG_FILE)){
		return setDefault(SMTP_CFG_FILE, &stSmtpcfg_default, sizeof(stSmtpcfg_default));
	}else if(!strcmp(conf_path, VIDMASK_CFG_FILE)){
		return setDefault(VIDMASK_CFG_FILE, &stVidMaskcfg_default, sizeof(stVidMaskcfg_default));
	}else if(!strcmp(conf_path, SYS_CFG_FILE)){
		return setDefault(SYS_CFG_FILE, &stSyscfg_default, sizeof(stSyscfg_default));
	}else if(!strcmp(conf_path, PTZ_PRESET_CFG_FILE)){
		return setDefault(PTZ_PRESET_CFG_FILE, &stPtz_preset_defualt, sizeof(stPtz_preset_defualt));
	}else if(!strcmp(conf_path, SYS_TIMER_FILE)){
		return setDefault(SYS_TIMER_FILE, &stTimecfg_default, sizeof(stTimecfg_default));
	}else if(!strcmp(conf_path, CLOUD_CFG_FILE)){
		return setDefault(CLOUD_CFG_FILE, &stCloud_cfg, sizeof(stCloud_cfg));
	}else if(!strcmp(conf_path, REC_CFG_FILE)){
		return setDefault(REC_CFG_FILE, &stRec_cfg_default, sizeof(stRec_cfg_default));
	}else{
		unlink(conf_path);
		return 0;
	}

	return -1;
}






