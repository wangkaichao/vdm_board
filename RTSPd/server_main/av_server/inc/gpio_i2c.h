
#ifndef _GPIO_I2C_H_
#define _GPIO_I2C_H_


#define GPIO_I2C_READ   0x01
#define GPIO_I2C_WRITE  0x03

#define STM32_I2C_WRITE	(0x04)
#define STM32_I2C_READ		(0x05)

typedef struct st_stm32_i2c {
	unsigned char devaddr;
	unsigned char *pbuf;
	unsigned long len;
} st_stm32_i2c;

unsigned char gpio_i2c_read(unsigned char devaddress, unsigned char address);
void gpio_i2c_write(unsigned char devaddress, unsigned char address, unsigned char value);

// add by wangkaichao
void stm32_i2c_write(const st_stm32_i2c *pstm32);
void stm32_i2c_read(st_stm32_i2c *pstm32);

#endif

