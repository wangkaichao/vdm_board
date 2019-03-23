
#include "br_keyboard.h"

#include <stdlib.h>
#include <stdio.h>
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

#define VERSION 1

int g_debug_or_release = 1;

callback_key_fun g_key_fun = NULL;

int g_device_id = 1;
