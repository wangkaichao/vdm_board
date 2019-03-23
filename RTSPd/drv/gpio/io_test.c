
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "gpio.h"


int main(int argc, char *argv[])
{
	int cmd = 0;
	int fd = 0;

	if (argc < 2)
		printf("usage fail\n");
	cmd = atoi(argv[1]);

	fd = open("/dev/BR_gpio", 0);
	if (fd < 0) {
		printf("open fail\n");
		return 1;
	}

	switch (cmd) {
		default:
		case 0:
			printf("set systat up\n");
			ioctl(fd, GPIO_SYSSTAT_UP, 0);
			break;
		case 1:
			printf("set systat down\n");
			ioctl(fd, GPIO_SYSSTAT_DOWN, 0);
			break;
		case 2:
			printf("set channel 1 up\n");
			ioctl(fd, GPIO_CHN_1_UP, 0);
			break;
		case 3:
			printf("set channel 1 down\n");
			ioctl(fd, GPIO_CHN_1_DOWN, 0);
			break;
		case 4:
			printf("set channel 2 up\n");
			ioctl(fd, GPIO_CHN_2_UP, 0);
			break;
		case 5:
			printf("set channel 2 down\n");
			ioctl(fd, GPIO_CHN_2_UP, 0);
			break;
	}
	
	return 0;	
}
