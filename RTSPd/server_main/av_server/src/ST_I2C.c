#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>

#include "gpio_i2c.h"
#include "aes.h"

#define	GROUPS				(1)
#define 	GROUPS_LEN		(16)

extern void Stop_stat_thread(void) ;

static long getval(long seed);
static void filled(unsigned char *buf, unsigned char len, unsigned char v) ;
static void printHex(unsigned char *buf, int len);

/*
这个算法保证所产生的值不会超过(2^31 - 1)
这里(2^31 - 1)就是 0x7FFFFFFF。而 0x7FFFFFFF
等于127773 * (7^5) + 2836,7^5 = 16807。
整个算法是通过：t = (7^5 * t) mod (2^31 - 1)
这个公式来计算随机值，并且把这次得到的值，作为
下次计算的随机种子值。
*/
static long getval(long seed) {
    long quotient, remainder, t;
    static long next;

    if (seed >= 0) next = seed;

    quotient  = next / 127773L;
    remainder = next % 127773L;
    t = 16807L * remainder - 2836L * quotient;

    if (t <= 0) t += 0x7FFFFFFFL;

    return ((unsigned long)(next = t) % (unsigned long)0x80000000);
}

static void filled(unsigned char *buf, unsigned char len, unsigned char v) 
{
	int i;
	buf[0] = v; buf[1] = v + 1;

	for (i = 2; i < len; i++) {
		buf[i] = buf[i - 1] + buf[i - 2];
	}
}

static void printHex(unsigned char *buf, int len) 
{
	int i;
	for (i = 0; i < len; i++) {
		printf("%02x", &buf[i]);
	}
	printf(" | ");
}

extern void Stop_rtspclient(int signo) ;

void *i2c_loop(void *pArgs)
{
	int fd = -1;
	st_stm32_i2c st_i2c;
	unsigned char buf[GROUPS * GROUPS_LEN];
	unsigned int cnt = 0;
	
	fd = open("/dev/gpioi2c", 0);
	if (fd < 0) {
		printf("Open gpioi2c error!\n");
		return (void *)-1;
	}
	st_i2c.devaddr = 0x30;
	st_i2c.len = sizeof(buf);
	st_i2c.pbuf = buf;
	
	void *p = malloc(10);
	free(p);
	long seed = (long)p;
	seed = seed < 0 ? -seed : (seed == 0 ? 1 : seed);
	getval(seed);
	
	while (1) {
		int i;
		unsigned char srcbuf[GROUPS * GROUPS_LEN];
		unsigned char desbuf[GROUPS * GROUPS_LEN];
	
		for (i = 0; i < st_i2c.len / 4; i++) {
			*(long *)&srcbuf[i * 4] = getval(-1);
		}
		//printHex(srcbuf, sizeof(srcbuf));
		
		for (i = 0; i < GROUPS; i++) {
			aes_context aes_ctx;
			unsigned char key[32];
			
			filled(key, sizeof(key), 3);
			aes_set_key(&aes_ctx, key);
			memset(key, 0, sizeof(key));
			aes_encrypt(&aes_ctx, &srcbuf[i*GROUPS_LEN], &st_i2c.pbuf[i*GROUPS_LEN]);
			memset(&aes_ctx, 0, sizeof(aes_ctx));
		}
		//printHex(st_i2c.pbuf, st_i2c.len);
		
		ioctl(fd, STM32_I2C_WRITE, &st_i2c);
		
		sleep(5);
		ioctl(fd, STM32_I2C_READ, &st_i2c);
		
		for (i = 0; i < GROUPS; i++) {
			aes_context aes_ctx;
			unsigned char key[32];
			
			filled(key, sizeof(key), 5);
			aes_set_key(&aes_ctx, key);
			memset(key, 0, sizeof(key));
			aes_decrypt(&aes_ctx, &st_i2c.pbuf[i*GROUPS_LEN], &desbuf[i*GROUPS_LEN]);
			memset(&aes_ctx, 0, sizeof(aes_ctx));
		}
		//printHex(st_i2c.pbuf, st_i2c.len);
		//printHex(desbuf, sizeof(desbuf));
		//printf("\n");
		
		if (memcmp(srcbuf, desbuf, sizeof(srcbuf))) {
			if (++cnt >= 3) {
				Stop_stat_thread();
				Stop_rtspclient(0);
			}
		} else {
			cnt = 0;
		}
		
		memset(srcbuf, 0, sizeof(srcbuf));
		memset(srcbuf, 0, sizeof(desbuf));
	}
	
	close(fd);
    	   
	return NULL;
}

void i2c_get_init_data(unsigned short cmd, unsigned char *pbuf)
{
	int fd = -1;
	st_stm32_i2c st_i2c;
	aes_context aes_ctx;
	unsigned char key[32];
	unsigned char buf[256];
	unsigned char src [GROUPS_LEN * 2];

	st_i2c.devaddr = 0x30;
	st_i2c.len = GROUPS_LEN * 2;
	st_i2c.pbuf = buf;
	
	void *p = malloc(10);
	free(p);
	long seed = (long)p;
	seed = seed < 0 ? -seed : (seed == 0 ? 1 : seed);
	getval(seed);

	int i;
	for (i = 0; i < sizeof(src); ++i) {
		*(src + i) = getval(-1);
	}
	src[0] = (cmd >> 8) & 0xff;
	src[1] = cmd & 0xff;
	
	filled(key, sizeof(key), 1);
	aes_set_key(&aes_ctx, key);
	memset(key, 0, sizeof(key));
	
	aes_encrypt(&aes_ctx, src, st_i2c.pbuf);
	aes_encrypt(&aes_ctx, src + GROUPS_LEN, st_i2c.pbuf + GROUPS_LEN);
	memset(&aes_ctx, 0, sizeof(aes_ctx));

	//printHex(src, sizeof(src));
	//printHex(buf, GROUPS_LEN*2);
		
	fd = open("/dev/gpioi2c", 0);
	if (fd < 0) {
		printf("open /dev/gpioi2c error!\n");
		return ;
	}
	
	ioctl(fd, STM32_I2C_WRITE, &st_i2c);
	sleep(1);

	if (src[0] == 0) {
		switch (src[1]) {
		case 0:
			// int32_t init_data1[5] = {1, 14, (1<<2) | (1<<5), 0, 0};
			st_i2c.len = 5 * 4;
			break;
			
		default:
			break;
		}
	} else if (src[0] == 1) {
		switch (src[1]) {
		case 0:
		case 1:
		case 2:
		case 3:
			// int32_t init_data2[19] = {0, 0, -1, 0, 0, 4096, 30, 30, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0, 1, 4, 1, 0};
			st_i2c.len = 19 * 4;
			break;
			
		default:
			break;
		}
	}
	st_i2c.len = (st_i2c.len +  GROUPS_LEN - 1) /GROUPS_LEN * GROUPS_LEN;
	ioctl(fd, STM32_I2C_READ, &st_i2c);
	close(fd);
	
	//filled(key, sizeof(key), 1);
	memmove(key, src, sizeof(key));
	aes_set_key(&aes_ctx, key);
	memset(key, 0, sizeof(key));

	int loop = st_i2c.len / GROUPS_LEN;
	for (i = 0; i < loop; i++)
		aes_decrypt(&aes_ctx, buf + i*GROUPS_LEN, pbuf + i*GROUPS_LEN);
	memset(&aes_ctx, 0, sizeof(aes_ctx));
}

