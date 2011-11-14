
CC = gcc
LDFLAGS = -lxml2 -lcurl -lpcre
CFLAGS = -g -Wall -O2 -I/usr/include/libxml2

PROGRAMS = dlight dlight-compile dlight-read-config dlight-filter-check

all : $(PROGRAMS)

install : $(PROGRAMS)
	cp $^ $(HOME)/bin/

dlight : dlight.o buffer.o env.o http.o rss.o lockfile.o filter.o cconf.o \
	proc-cache.o error.o
dlight-compile : compile.o buffer.o env.o lockfile.o filter.o cconf.o error.o
dlight-read-config : read-config.o buffer.o env.o cconf.o error.o
dlight-filter-check: filter-check.o filter.o error.o

dlight-% : %.o
	$(CC) $(LDFLAGS) -o $@ $^

clean :
	$(RM) *.o $(PROGRAMS)
