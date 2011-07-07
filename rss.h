/* rss.h
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
#ifndef RSS_ITEM_H
#define RSS_ITEM_H

#include <stddef.h>

typedef struct __rss* rss_t;
typedef struct __walk_info* rss_walk_info;

struct rss_item {
	const char *title;
	const char *link;
};

rss_t rss_parse(void *buf, size_t size);

void rss_free(rss_t r);

/* walking interface */

int rss_walk_next(rss_t rss, struct rss_item *item);

int rss_walk_reset(rss_t rss);

#endif
