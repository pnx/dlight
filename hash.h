/* hash.h
 *
 *   Copyright (C) 2011-2012  Henrik Hautakoski <henrik@fiktivkod.org>
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
#ifndef HASH_H
#define HASH_H

#include <stddef.h>

struct hash_entry {
	unsigned hash;
};

struct hash_table {
	struct hash_entry **ptr;
	unsigned size;
	unsigned count;
};

#define HASH_TABLE_INIT { NULL, 0, 0 }

#define hash_entry(t, i) \
	((void*) *((char**) (t)->ptr + (i)))

/* general hash functions */
unsigned hash_sdbm(const char *s);

void hash_init(struct hash_table *table);

void hash_free(struct hash_table *table);

void* hash_lookup(const struct hash_table *t, unsigned hash);

void* hash_insert(struct hash_table *t, unsigned hash, void *ptr);

void* hash_remove(struct hash_table *t, unsigned hash);

#endif /* HASH_H */
