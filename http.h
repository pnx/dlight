/* http.h
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
#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>
#include "buffer.h"

struct http_file {
	struct buffer data;
	char *filename;
};

struct buffer* http_fetch_page(const char *url);

struct http_file* http_fetch_file(const char *url);

int http_download_file(const char *url, const char *dir);

void http_free(struct buffer *b);

void http_free_file(struct http_file *file);

#endif
