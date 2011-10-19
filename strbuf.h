/* strbuf.h
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

#ifndef __STRBUF_H
#define __STRBUF_H

#include <string.h>
#include <stddef.h>

typedef struct {
	char  *buf;
	size_t len;
	size_t size;
} strbuf_t;

extern char strbuf_null;

#define STRBUF_INIT { &strbuf_null, 0, 0 }

void strbuf_init(strbuf_t *s);

static inline size_t strbuf_avail(strbuf_t *s) {

	return s->size ? s->size - (s->len + 1) : 0;
}

void strbuf_expand(strbuf_t *s, size_t len);

static inline void strbuf_setlen(strbuf_t *s, size_t len) {

	if (!s->size)
		return;

	if (len >= s->size)
		len = s->size - 1;
	s->len = len;
	s->buf[s->len] = '\0';
}

static inline void strbuf_reduce(strbuf_t *s, size_t len) {

	if (len > s->len)
		len = s->len;

	strbuf_setlen(s, s->len - len);
}

char* strbuf_release(strbuf_t *s);

void strbuf_free(strbuf_t *s);

void strbuf_attach(strbuf_t *s, void *str, size_t len, size_t size);

void strbuf_append(strbuf_t *s, const void *ptr, size_t len);

static inline void strbuf_append_str(strbuf_t *s, const char *str) {

	strbuf_append(s, str, strlen(str));
}

static inline void strbuf_append_ch(strbuf_t *s, char ch) {

	strbuf_expand(s, 1);
	s->buf[s->len++] = ch;
	s->buf[s->len] = '\0';
}

static inline void strbuf_append_repeat(strbuf_t *s, char ch, size_t len) {

	strbuf_expand(s, len);
	memset(s->buf + s->len, ch, len);
	strbuf_setlen(s, s->len + len);
}

static inline void strbuf_term(strbuf_t *s, char ch) {

	if (s->buf[s->len-1] != ch)
		strbuf_append_ch(s, ch);
}

#endif /* __STRBUF_H */
