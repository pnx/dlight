
CC = gcc
LDFLAGS = -lxml2 -lcurl -lpcre
CFLAGS = -g -Wall -I/usr/include/libxml2

PROGRAMS = dlight dlight-compile dlight-read-config dlight-filter-check

ifeq ($(DEBUG), 1)
	CFLAGS +=-D__DEBUG__
endif

all : $(PROGRAMS)

install : $(PROGRAMS)
	cp $^ $(HOME)/bin/

dlight : dlight.o buffer.o env.o http.o rss.o lockfile.o filter.o cconf.o \
	proc-cache.o dlhist.o hash.o xalloc.o error.o utils.o
dlight-compile : compile.o buffer.o env.o lockfile.o filter.o cconf.o \
	error.o
dlight-read-config : read-config.o buffer.o env.o cconf.o error.o
dlight-filter-check: filter-check.o filter.o error.o

dlight-% : %.o
	$(CC) $(LDFLAGS) -o $@ $^

clean :
	$(RM) *.o $(PROGRAMS)
