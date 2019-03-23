/*
*2012.02.19 	by Aaron
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ipnc.h"

int main(int argc, char *argv[])
{
	init_system();

	boot_other();

	av_server();

	return 0;
}
