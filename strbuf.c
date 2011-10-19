/* strbuf.c
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
#include "error.h"
#include "strbuf.h"

#define CHNK_SIZE 128

char strbuf_null = '\0';

void strbuf_init(strbuf_t *s) {

	s->buf = &strbuf_null;
	s->size = s->len = 0;
}

void strbuf_expand(strbuf_t *s, size_t len) {

	if (s->len + len + 1 < s->size)
		return;
	if (!s->size)
		s->buf = NULL;

	do
		s->size += CHNK_SIZE;
	while(s->len + len + 1 > s->size);

	s->buf = realloc(s->buf, s->size);
}

char* strbuf_release(strbuf_t *s) {

	char *ret;

	if (!s->size)
		ret = malloc(1);
	else if (s->len + 1 != s->size)
		ret = realloc(s->buf, s->len + 1);
	else
		ret = s->buf;

	strbuf_init(s);

	return ret;
}

void strbuf_free(strbuf_t *s) {

	if (!s->size)
		return;

	free(s->buf);
	strbuf_init(s);
}

void strbuf_attach(strbuf_t *s, void *buf, size_t len, size_t size) {

	strbuf_free(s);
	s->buf = buf;
	s->len = len;
	s->size = size;
	strbuf_expand(s, 0);
	s->buf[s->len] = '\0';
}

void strbuf_append(strbuf_t *s, const void *ptr, size_t len) {

	strbuf_expand(s, len);
	memcpy(s->buf + s->len, ptr, len);
	strbuf_setlen(s, s->len + len);
}
