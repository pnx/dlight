/* buffer.h
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

#ifndef BUFFER_H
#define BUFFER_H

#include <string.h>
#include <stddef.h>

struct buffer {
	unsigned char *block;
	size_t len;
	size_t size;
};

extern unsigned char buffer_null;

#define BUFFER_INIT { &buffer_null, 0, 0 }

void buffer_init(struct buffer *b);

static inline size_t buffer_avail(struct buffer *b) {

	return b->size ? b->size - (b->len + 1) : 0;
}

void buffer_expand(struct buffer *b, size_t len);

static inline void buffer_setlen(struct buffer *b, size_t len) {

	if (!b->size)
		return;

	if (len >= b->size)
		len = b->size - 1;
	b->len = len;
}

static inline void buffer_reduce(struct buffer *b, size_t len) {

	if (len > b->len)
		len = b->len;

	buffer_setlen(b, b->len - len);
}

char* buffer_cstr(struct buffer *b);

char* buffer_cstr_release(struct buffer *b);

void buffer_free(struct buffer *b);

void buffer_attach(struct buffer *b, void *str, size_t len, size_t size);

void buffer_append(struct buffer *b, const void *ptr, size_t len);

static inline void buffer_append_str(struct buffer *b, const char *str) {

	buffer_append(b, str, strlen(str));
}

static inline void buffer_append_ch(struct buffer *b, char ch) {

	buffer_expand(b, 1);
	b->block[b->len++] = ch;
}

static inline void buffer_append_repeat(struct buffer *b, char ch, size_t len) {

	buffer_expand(b, len);
	memset(b->block + b->len, ch, len);
	buffer_setlen(b, b->len + len);
}

static inline void buffer_str_term(struct buffer *b, char ch) {

	if (b->block[b->len - 1] != ch)
		buffer_append_ch(b, ch);
}

int buffer_write(struct buffer *b, const char *filename);

#endif /* BUFFER_H */
