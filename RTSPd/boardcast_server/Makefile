# Make file
ifndef ARCH
ARCH=arm
endif
 
GV_CC_PATH=/gvos/$(ARCH)/crosstool/bin/
## platform:c2
ifeq ($(ARCH),c2)
CROSS_COMPILE = $(GV_CC_PATH)c2-linux-
CFLAGS= -I/gvos/$(ARCH)/include -I/gvos/$(ARCH)/include
LDLIBS = -L/gvos/$(ARCH)/lib -lgipcclient
endif
## platform:telechips
ifeq ($(ARCH),telechips)
CROSS_COMPILE = $(GV_CC_PATH)arm-none-linux-gnueabi-
CFLAGS= -I/gvos/$(ARCH)/include -I/gvos/$(ARCH)/include/gvos
LDLIBS = -L/gvos/$(ARCH)/lib -lgipcclient 
endif
## platform:x86
ifeq ($(ARCH),x86)
CROSS_COMPILE = 
CFLAGS= -I/usr/include/libxml2  -I/home/lizhan/share/ipnc_app/codes/CmdParser/include
LDLIBS = -lpthread    
endif
ifeq ($(ARCH),arm)
LINUX_SRC=/usr/src/arm-linux-2.6.28/linux-2.6.28-fa
LINUX_INC=$(LINUX_SRC)/include
CROSS_COMPILE = arm-hisiv100nptl-linux-
#CFLAGS=  -I/home/aaron/ipnc_app/codes/CmdParser/include 
LDLIBS = -lpthread
endif

#CFLAGS += -Wall

AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
CPP     = $(CROSS_COMPILE)g++
RANLIB  = $(CROSS_COMPILE)ranlib
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

BINNAME=boardcast_server
#LIBNAME=libcmdreceiver

#LIBOBJS = application.o connection.o eventloop.o server.o util.o remotefunction.o
# Common rules that everyone uses 

%.o: %.c
	@ echo "Compiling $<..."
	@ $(CC) $(CFLAGS) -o $@ -c $<

%.d: %.c
	@ $(CC) -M $(CFLAGS) $< > $@
	@ $(CC) -M $(CFLAGS) $< | sed s/\\.o/.d/ >> $@

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
DEPS := $(patsubst %.o,%.d,$(OBJS))

all: $(OBJS) $(LIBNAME) $(BINNAME)

clean:
	@ rm -f $(OBJS) $(BINNAME) *~ core *.a *.so

$(LIBNAME):$(OBJS)
	@ $(AR) rc $(LIBNAME).a $(OBJS)
	@ $(LD) -shared -o $(LIBNAME).so $(OBJS)
	@ rm *.o

$(BINNAME):$(OBJS)
	@ echo "Building $(BINNAME)..."
	@ $(CC) -o $(BINNAME) $(OBJS) $(LDLIBS)
	@ mv $(BINNAME) ../build/bin/
	@ rm *.o
