/* read-config.c
 *
 *   Copyright (C) 2011       Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *   MA 02110-1301, USA.
 */
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
