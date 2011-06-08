
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cconf.h"
#include "env.h"

static char *usage = "dlight-read-config [ <file> | -h ]\n";

int main(int argc, char **argv) {

	int i;
	struct cconf *c;
	char file[4096];

	if (argc > 1) {
		if (!strcmp(argv[1], "-h")) {
			fprintf(stderr, usage);
			return 1;
		}
		strncpy(file, argv[1], sizeof(file));
	} else {
		snprintf(file, sizeof(file), "%s/config", env_get_dir());
	}

	c = cconf_read(file);
	if (!c) {
		perror(file);
		return 1;
	}

	printf("--- Config file: %s ---\n", file);
	for(i=0; i < c->nr; i++) {
		int j;
		struct target *t = c->target + i;

		printf("src: %s\n", t->src);
		printf("dest: %s\n", t->dest);

		for(j=0; j < t->nr; j++)
			printf("filter: %s\n", t->filter[j]);

		printf("---\n");
	}

	cconf_free(c);
	free(c);

	return 0;
}
