/* compiler.c
 *
 *   Copyright (C) 2012       Henrik Hautakoski <henrik@fiktivkod.org>
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
#include <errno.h>
#include "lexer.h"

FILE* sfopen(const char *file, const char *mode) {

	FILE *fd = fopen(file, mode);
	if (fd == NULL)
		perror("Unable to open file");
	return fd;
}

static void usage(const char *name) {

	fprintf(stderr, "usage: %s <sourcefile> [ <outfile> ]\n", name);
}

int main(int argc, char **argv) {

	FILE *fd, *outfd = stdout;

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	} else if (argc >= 3) {
		outfd = sfopen(argv[2], "w");
		if (outfd == NULL)
			return -1;
	}

	fd = sfopen(argv[1], "r");
	if (fd == NULL)
		return -1;

	/* WIP: currently just output the tokens. */
	for(;;) {
		token_t t = lexer_getnext(fd);

		if (t.type == TOKEN_EOI)
			break;
		token_print(t);
	}

	fclose(fd);
	if (outfd != stdout)
		fclose(outfd);
	return 0;
}
