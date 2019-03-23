
include ../makefile.rule

INCLUDE += -I../inc

LIBDIR=../lib

USE_FILE = test.c

EXEFILE = test

LIBFILE = -lpthread -L$(LIBDIR) -lraj  -L$(LIBDIR) -lrtsp  -lstdc++  $(MPI_LIBS) $(AUDIO_LIBA) $(JPEGD_LIBA)



all:
	$(CC)  -o $(EXEFILE) $(USE_FILE) $(LIBFILE) $(INCLUDE) $(LIBS) 	
	cp -f $(EXEFILE) ../../build/bin/
clean:
	rm -f  $(EXEFILE)

