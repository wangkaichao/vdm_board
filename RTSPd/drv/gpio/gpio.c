
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>

#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/io.h>

#include "gpio.h" 

#define GPIO_1_BASE 0x20160000
#define SYSSTAT_SHIFT_NUM	1
#define CHN1_SHIFT_NUM   0
#define SYSSTAT             (0x1 << SYSSTAT_SHIFT_NUM)
#define CHN1                 (0x1 << CHN1_SHIFT_NUM)    /* GPIO7 0_7 */
#define GPIO_SYSSTAT_REG    IO_ADDRESS(GPIO_1_BASE + (0x1<<(SYSSTAT_SHIFT_NUM+2)))  /* 0x200 */
#define GPIO_CHN1_REG    IO_ADDRESS(GPIO_1_BASE + (0x1<<(CHN1_SHIFT_NUM+2)))  /* 0x200 */


#define GPIO_1_DIR IO_ADDRESS(GPIO_1_BASE + 0x400)
#define HW_REG(reg)         *((volatile unsigned int *)(reg))
#define DELAY(us)           time_delay_us(us)

typedef enum {
	E_SYSSTAT = 0,
	E_CHN1,
	E_CHN2
}light_type;

static int get_addr(light_type t, unsigned int *dir, unsigned int *var)
{
	switch (t) {
		default:
		case E_SYSSTAT:
			*dir = GPIO_1_DIR;	
			*var = GPIO_SYSSTAT_REG;	
			return SYSSTAT;

		case E_CHN1:
		case E_CHN2:
			*dir = GPIO_1_DIR;	
			*var = GPIO_CHN1_REG;	
			return CHN1;			
	}
}

static void gpio_set_val(light_type ty, int val)
{
	unsigned int reg = 0;
	unsigned int dir_addr = 0;
	unsigned int var_addr = 0;
	int shift = 0;

	
	shift = get_addr(ty, (unsigned int *)&dir_addr, (unsigned int *)&var_addr);
	
	reg = HW_REG(dir_addr); 
	reg |= shift; 
	HW_REG(dir_addr) = reg;

	if (val)
		HW_REG(var_addr) = reg;
	else
		HW_REG(var_addr) = 0;

	return;
}




/*
 *  delays for a specified number of micro seconds rountine.
 *
 *  @param usec: number of micro seconds to pause for
 *
 */
void time_delay_us(unsigned int usec)
{
	volatile int i,j;

        for(i = 0 ; i< usec * 5 ;i++)
    	{
			for(j=0;j<47;j++)
	    		{;}
    	}
  
}

long gpioi2c_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
//	printk("set cmd %x...\n", cmd);
	switch(cmd)
	{
		case GPIO_SYSSTAT_UP:
//			printk("set sysstat up...\n");
			gpio_set_val(E_SYSSTAT, 0);	
			break;
		
		case GPIO_SYSSTAT_DOWN:
//			printk("set sysstat down...\n");
			gpio_set_val(E_SYSSTAT, 1);	
			break;

		case GPIO_CHN_1_UP:
//			printk("set channel 1 up...\n");
			gpio_set_val(E_CHN1, 0);	
			break;
		case GPIO_CHN_1_DOWN:
//			printk("set channel 1 down...\n");
			gpio_set_val(E_CHN1, 1);	
			break;
		case GPIO_CHN_2_UP:
//			printk("set channel 2 up...\n");
			gpio_set_val(E_CHN2, 0);	
			break;
		case GPIO_CHN_2_DOWN:
//			printk("set channel 2 down...\n");
			gpio_set_val(E_CHN2, 1);	
			break;
	
		default:
			return -1;
	}
    return 0;
}

int gpioi2c_open(struct inode * inode, struct file * file)
{
    return 0;
}
int gpioi2c_close(struct inode * inode, struct file * file)
{
    return 0;
}


static struct file_operations gpioi2c_fops = {
    .owner      = THIS_MODULE,
    //.ioctl      = gpioi2c_ioctl,
    .unlocked_ioctl = gpioi2c_ioctl,
    .open       = gpioi2c_open,
    .release    = gpioi2c_close
};


static struct miscdevice gpio_dev = {
   .minor		= MISC_DYNAMIC_MINOR,
   .name		= "BR_gpio",
   .fops  = &gpioi2c_fops,
};

static int __init gpio_init(void)
{
    int ret;
    //unsigned int reg;

    printk("xxxxxxxxxxxxxxxxxx\n");
    ret = misc_register(&gpio_dev);
    if(0 != ret)
    	return -1;
        
    return 0;    
}

static void __exit gpio_exit(void)
{
    misc_deregister(&gpio_dev);
}


module_init(gpio_init);
module_exit(gpio_exit);

#ifdef MODULE
//#include <linux/compile.h>
#endif
//MODULE_INFO(build, UTS_VERSION);
MODULE_LICENSE("GPL");




