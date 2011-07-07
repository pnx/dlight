/* cconf.h
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
#ifndef CCONF_H
#define CCONF_H

/*
 * data structure for 'Dlight compiled config' file format.
 */

/* \232 D C C */
#define CCONF_SIGNATURE 0xe8444343
struct cconf_header {
	unsigned int signature;
	unsigned int version;
	unsigned char crc[20];
};

struct target {
	char *src; /* source. (url) */
	char *dest; /* destination, path on filesystem */
	char **filter;
	unsigned int nr;
};

struct cconf {
	struct target *target;
	unsigned int nr;
	struct {
		void *buf;
		unsigned long size;
	} map;
};

void cconf_free(struct cconf *c);

struct target* cconf_new_target(struct cconf *c);

void cconf_add_filter(struct target *t, char *filter);

int cconf_write(int fd, struct cconf *c);

struct cconf* cconf_read(const char *filename);

#endif /* CCONF_H */
