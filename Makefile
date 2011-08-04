
CC = gcc
LDFLAGS = -lxml2 -lcurl -lpcre
CFLAGS = -g -Wall -O2 -I/usr/include/libxml2

PROGRAMS = dlight dlight-compile dlight-read-config

all : $(PROGRAMS)

install : $(PROGRAMS)
	cp $^ $(HOME)/bin/

dlight : dlight.o env.o http.o rss.o lockfile.o filter.o cconf.o dlhist.o error.o
dlight-compile : compile.o env.o lockfile.o filter.o cconf.o error.o
dlight-read-config : read-config.o env.o cconf.o error.o

dlight-% : %.o
	$(CC) $(LDFLAGS) -o $@ $^

clean :
	$(RM) *.o $(PROGRAMS)
