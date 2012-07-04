/* buffer.c
 *
 *   Copyright (C) 2010-2011   Henrik Hautakoski <henrik@fiktivkod.org>
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "error.h"
#include "buffer.h"

#define CHNK_SIZE 128

unsigned char buffer_null = '\0';

void buffer_init(struct buffer *b) {

	b->block = &buffer_null;
	b->size = b->len = 0;
}

void buffer_expand(struct buffer *b, size_t len) {

	if (b->len + len < b->size)
		return;
	if (!b->size)
		b->block = NULL;

	do
		b->size += CHNK_SIZE;
	while(b->len + len > b->size);

	b->block = realloc(b->block, b->size);
}

char* buffer_cstr(struct buffer *b) {

	buffer_expand(b, 1);
	b->block[b->len + 1] = '\0';

	return (char*) b->block;
}

char* buffer_cstr_release(struct buffer *b) {

	char *ret;

	if (b->size) {
		if (b->len + 1 != b->size) {
			ret = realloc(b->block, b->len + 1);
		} else {
			ret = (char *) b->block;
		}
		ret[b->len + 1] = '\0';
	} else {
		ret = calloc(1, 1);
	}

	buffer_init(b);

	return ret;
}

void buffer_free(struct buffer *b) {

	if (!b->size)
		return;

	free(b->block);
	buffer_init(b);
}

void buffer_attach(struct buffer *b, void *ptr, size_t len, size_t size) {

	buffer_free(b);
	b->block = ptr;
	b->len = len;
	b->size = size;
	buffer_expand(b, 0);
}

void buffer_append(struct buffer *b, const void *ptr, size_t len) {

	buffer_expand(b, len);
	memcpy(b->block + b->len, ptr, len);
	buffer_setlen(b, b->len + len);
}

int buffer_write(struct buffer *b, const char *filename) {

	int fd, rc;

	fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0664);
	if (fd < 0)
		return error("%s: %s", filename, strerror(errno));

	rc = write(fd, b->block, b->len);
	close(fd);

	return rc;
}
