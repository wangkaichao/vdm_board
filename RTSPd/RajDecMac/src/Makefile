

include ../makefile.rule


INC_PATH += -I../inc

CFLAGS += $(INC_PATH) 

BINDIR=../../bin

target=libraj.so
STATICLIB = libraj.a

SRCS    :=  $(wildcard *.c)
OBJS    :=  $(SRCS:%.c=%.o)


$(target):$(OBJS)
	$(CC) -fPIC -shared  $(CFLAGS)  $(MPP_CFLAGS)  -o $(target) $(OBJS)
	$(AR) -r $(STATICLIB) $(OBJS) 
	@rm -rf ../lib/$(target)
	@rm -rf ../lib/$(STATICLIB)
	@cp -rf $(STATICLIB) ../lib
	@cp -rf $(STATICLIB) ../../build/libs

%.o:%.c 
	$(CC)   $(CFLAGS)  $(MPP_CFLAGS)  -c $< -o $@ 	

clean:	
	@rm -f $(OBJS)
	@rm -f $(target)
	@rm -f $(STATICLIB)

.PHONY : clean all