
all::

VERSION_FILE : FORCE
	@$(SHELL) VERSION-GEN $@
-include VERSION_FILE

CC = gcc
LD = $(CC)
CFLAGS =
LDFLAGS =
LDLIBS = -lxml2 -lcurl -lcrypto -lpcre

PROGRAMS = dlight

CMD = cmd_compile.o		\
	cmd_dlhist.o 		\
	cmd_filter_check.o 	\
	cmd_read_config.o	\
	cmd_run.o 		\
	cmd_version.o

ifeq ($(DEBUG), 1)
	CFLAGS += -g -Wall -D__DEBUG__
else
	CFLAGS +=-Werror
endif

all:: $(PROGRAMS)

install : $(PROGRAMS)
	cp $^ $(HOME)/bin/

dlight : dlight.o $(CMD) buffer.o env.o http.o rss.o lockfile.o filter.o cconf.o \
	proc-cache.o dlhist.o hash.o xalloc.o error.o utils.o version.o
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

version.o : VERSION_FILE FORCE
version.o : EXTRA_CFLAGS = -DDLIGHT_VERSION=\"$(VERSION)\"

rss.o : EXTRA_CFLAGS = -I/usr/include/libxml2

%.o : %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -o $@ $<

clean :
	$(RM) *.o
	$(RM) VERSION_FILE

distclean : clean
	$(RM) $(PROGRAMS)

FORCE:
