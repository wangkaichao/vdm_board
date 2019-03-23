
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
			
obj-m := gpio.o
#gpio-y += gpio.o

#EXTRA_CFLAGS += -D$(HI_FPGA) 
 
default:	
	@$(CC) -g -Wall -o io_test io_test.c	
	@make -C $(LINUX_ROOT) M=$(PWD) modules
	#@cp i2c_read i2c_write $(REL_KO)
clean:
	@rm *.o i2c_read i2c_write -rf
	@make -C $(LINUX_ROOT) M=$(PWD) clean
