/* hash.c
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

#include "hash.h"
#include <stdlib.h>

#define TABLE_MIN_SIZE 128

unsigned hash_sdbm(const char *s) {

	unsigned h;

	for(h = 0; *s; s++)
		h = ((unsigned)*s) + (h << 6) + (h << 16) - h;
        return h;
}

void hash_init(struct hash_table *table) {

	table->ptr = NULL;
	table->size = 0;
	table->count = 0;
}

void hash_free(struct hash_table *table) {

	if (table->ptr)
		free(table->ptr);
	hash_init(table);
}

static struct hash_entry** lookup(const struct hash_table *t, unsigned hash) {

	unsigned i = hash % t->size;
	struct hash_entry **table = t->ptr;

	while(table[i]) {
		if (table[i]->hash == hash)
			break;
		i = (i + 1) % t->size;
	}
	return table + i;
}

void* hash_lookup(const struct hash_table *t, unsigned hash) {

	if (t->size < 1)
		return NULL;
	return *lookup(t, hash);
}

static void* insert(struct hash_table *t, unsigned hash, void *ptr) {

	struct hash_entry **dest = lookup(t, hash);

	if (!*dest) {
		*dest = ptr;
		(*dest)->hash = hash;
		t->count++;
		return NULL;
	}
	return *dest;
}

static unsigned needs_resize(const struct hash_table *t) {

	if (t->size) {
		double load = t->count / t->size;
		return ((load >= 0.5 && load <= 0.75) == 0) &&
			(t->size > TABLE_MIN_SIZE);
	}
	return 1;
}

static void resize(struct hash_table *t) {

	unsigned int i, old_size = t->size;
	struct hash_entry **old = t->ptr;

	/*
	 * set size to a load factor that is in the
	 * middle in the valid range.
	 */
	t->size = t->count / 0.625;
	if (t->size < TABLE_MIN_SIZE)
		t->size = TABLE_MIN_SIZE;

	t->count = 0;
	t->ptr = calloc(sizeof(t->ptr), t->size);

	if (old) {
		for(i=0; i < old_size; i++) {
			struct hash_entry *entry = old[i];
			if (entry)
				insert(t, entry->hash, entry);
		}
		free(old);
	}
}

void* hash_insert(struct hash_table *t, unsigned hash, void *ptr) {

	if (needs_resize(t))
		resize(t);
	return insert(t, hash, ptr);
}

void* hash_remove(struct hash_table *t, unsigned hash) {

	struct hash_entry **dest = lookup(t, hash);

	if (*dest) {
		void *ptr = *dest;
		*dest = NULL;
		t->count--;
		if (needs_resize(t))
			resize(t);
		return ptr;
	}
	return NULL;
}
