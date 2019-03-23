
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "gpio.h"

#define IO_FILE 	"/dev/BR_gpio"
static int sys_st = 0;
static int chn1_st = 0;
static int chn2_st = 0;

static int thread_flag = 1;
/*
void set_sys_up(void)
{
	sys_st = 1;
}
void set_sys_down(void)
{
	sys_st = 0;
}
*/
void set_chn1_up(void)
{
	chn1_st = 1;
}
void set_chn1_down(void)
{
	chn1_st = 0;
}
void set_chn2_up(void)
{
	chn2_st = 1;
}
void set_chn2_down(void)
{
	chn2_st = 0;
}

void Stop_stat_thread(void) 
{
	thread_flag = 0;
}

//int main(int argc, char *argv[])
void *stat_set(void *pArgs)
{
	int cmd = 0;
	int fd = 0;
    unsigned int step = 1;
    unsigned int count = 0;

	fd = open(IO_FILE, 0);
	if (fd < 0) {
		printf("open %s fail\n", IO_FILE);
		return 1;
	}

	while (thread_flag) {
		count++;
		if (count % 2 == 0) {
			if (sys_st) {
				ioctl(fd, GPIO_SYSSTAT_UP, 0);
				sys_st = 0;
	       } else {
	       	ioctl(fd, GPIO_SYSSTAT_DOWN, 2);
	           sys_st = 1;
	       }
		}

		if (chn1_st && chn2_st) {
			ioctl(fd, GPIO_CHN_1_UP, 0);
		} else if (chn1_st || chn2_st) {
			if (count % 2 == 0)
				ioctl(fd, GPIO_CHN_1_UP, 0);
			else
				ioctl(fd, GPIO_CHN_1_DOWN, 0);
		} else {
			ioctl(fd, GPIO_CHN_1_DOWN, 0);
		}
		
		usleep(500000);
	}

	if (fd >= 0)
		close(fd);

	return 0;	
}
