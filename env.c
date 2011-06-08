
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "env.h"

static char base[4096];

static void get_base() {

	char *ptr;
	int len;

	ptr = getenv("HOME");
	if (!ptr)
		ptr = ".";

	len = strlen(ptr);
	if (len < sizeof(base) - 9) {
		memcpy(base, ptr, len);
		memcpy(base+len, "/.dlight", 9);
	}
}

const char* env_get_dir() {

	if (!*base) {
		get_base();
		if (mkdir(base, 0700) < 0 && errno != EEXIST) {
			fprintf(stderr, "unable to create '%s': %s\n",
				base, strerror(errno));
			exit(1);
		}
	}
	return base;
}
