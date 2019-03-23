# Make file
ifndef ARCH
ARCH=hisi
endif
 
GV_CC_PATH=/gvos/$(ARCH)/crosstool/bin/
## platform:c2
ifeq ($(ARCH),c2)
CROSS_COMPILE = $(GV_CC_PATH)c2-linux-
CFLAGS= -I/gvos/$(ARCH)/include 
endif
## platform:telechips
ifeq ($(ARCH),telechips)
CROSS_COMPILE = $(GV_CC_PATH)arm-none-linux-gnueabi-
CFLAGS= 
endif
## platform:x86
ifeq ($(ARCH),x86)
CROSS_COMPILE = 
CFLAGS= -fPIC
LDLIBS = -lpthread
endif

ifeq ($(ARCH),hisi)
CROSS_COMPILE = arm-hisiv100nptl-linux-
CFLAGS= 
LDLIBS = -lpthread
endif
#INCLUDE=-I./
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

#BINNAME=/opt/lampp/htdocs/xampp/test/hotplug
LIBNAME=libbrkeyboard

#LIBOBJS = eventloop.o  util.o 
# Common rules that everyone uses 

%.o: %.c
	@ echo "Compiling $<..."
	@ $(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

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
	#@ $(LD) -shared -o $(LIBNAME).so $(OBJS)

	@ mv libbrkeyboard.a ./build/libs
	@ cp br_keyboard.h ./build/include
	

$(BINNAME):$(OBJS)
	@ echo "Building $(BINNAME)..."
	@ $(CC) -o $(BINNAME) $(OBJS) $(LDLIBS)
	@ rm *.o

install:all
	cp ./build/libs/$(LIBNAME).a ../build/libs/
