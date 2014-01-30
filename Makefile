
all::

VERSION_FILE : FORCE
	@$(SHELL) VERSION-GEN $@
-include VERSION_FILE

CC = gcc
LDFLAGS = -lxml2 -lcurl -lcrypto -lpcre
CFLAGS = -g -Wall -I/usr/include/libxml2


PROGRAMS = dlight dlight-compile dlight-read-config dlight-filter-check \
	dlight-dlhist-read

ifeq ($(DEBUG), 1)
	CFLAGS +=-D__DEBUG__
endif

all:: $(PROGRAMS)

install : $(PROGRAMS)
	cp $^ $(HOME)/bin/

dlight : dlight.o buffer.o env.o http.o rss.o lockfile.o filter.o cconf.o \
	proc-cache.o dlhist.o hash.o xalloc.o error.o utils.o version.o
dlight-compile : compile.o buffer.o env.o lockfile.o filter.o cconf.o \
	error.o version.o
dlight-read-config : read-config.o buffer.o env.o cconf.o error.o version.o
dlight-filter-check: filter-check.o filter.o error.o version.o
dlight-dlhist-read : dlhist-read.o buffer.o utils.o env.o error.o lockfile.o dlhist.o

dlight-% : %.o
	$(CC) $(LDFLAGS) -o $@ $^

version.o : VERSION_FILE FORCE
version.o : EXTRA_CFLAGS = -DDLIGHT_VERSION=\"$(VERSION)\"

%.o : %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

clean :
	$(RM) *.o
	$(RM) VERSION_FILE

distclean : clean
	$(RM) $(PROGRAMS)

FORCE:
