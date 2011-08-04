/* error.c - Error handling helper routines
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
#include <stdarg.h>
#include "error.h"

static void print_e(FILE *fd, const char *prefix, const char *fmt, va_list va) {

	static char msg[4096];
	vsnprintf(msg, sizeof(msg), fmt, va);
	fprintf(fd, "%s: %s\n", prefix, msg);
}

void usage(const char *msg, ...) {

	va_list vl;
	va_start(vl, msg);
	print_e(stdout, "usage", msg, vl);
	va_end(vl);
	exit(EXIT_FAILURE);
}

void warn(const char *msg, ...) {

	va_list vl;
	va_start(vl, msg);
	print_e(stderr, "warning", msg, vl);
	va_end(vl);
}

void fatal(const char *msg, ...) {

	va_list vl;
	va_start(vl, msg);
	print_e(stderr, "fatal", msg, vl);
	va_end(vl);
	exit(EXIT_FAILURE);
}

int error(const char *msg, ...) {

	va_list vl;
	va_start(vl, msg);
	print_e(stderr, "error", msg, vl);
	va_end(vl);
	return -1;
}