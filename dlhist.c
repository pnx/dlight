/* dlhist.c
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "env.h"
#include "error.h"
#include "lockfile.h"
#include "utils.h"
#include "dlhist.h"

/* \195 D L H */
#define SIGNATURE 0xC3444C48
#define STORAGE_FILE "dlhist"

#define HASH_TABLE_LOAD(c, s) ((double) (c) / ((s) ? (s) : 1))

#define TABLE_MIN_SIZE 128

struct header {
	unsigned int signature;
	unsigned int version;
	unsigned int size;
};

struct destination {
	char *path;
	unsigned time;
};

struct hash_entry {
	char *key;
	unsigned dest_nr;
	struct destination *dest;
};

#define he_empty(x) (!(x)->key)

static struct lockfile lock = LOCKFILE_INIT;

static struct hash_entry *table;
static unsigned int table_size;
static unsigned int table_count;

static unsigned hash(const char *s) {

	unsigned h;

	for(h = 0; *s; s++)
		h = ((unsigned)*s) + (h << 6) + (h << 16) - h;
        return h;
}

static struct hash_entry* lookup(const char *key) {

	unsigned index = hash(key) % table_size;

	/* linear probing */
	while(!he_empty(table + index)) {
		if (!strcmp(table[index].key, key))
			break;
		index = (index + 1) % table_size;
	}
	return table + index;
}

static inline void he_set(struct hash_entry *he, const char *key) {

	if (!he_empty(he))
		return;
	he->key = strdup(key);
	table_count++;
}

static int he_insert(struct hash_entry *he) {

	struct hash_entry *dest = lookup(he->key);

	if (he_empty(dest)) {
		memcpy(dest, he, sizeof(*he));
		table_count++;
		return 1;
	}
	return 0;
}

static void he_remove(struct hash_entry *he) {

	if (he->key) {
		free(he->key);
		he->key = NULL;
	}
	if (he->dest) {
		int i;
		for(i=0; i < he->dest_nr; i++)
			free(he->dest[i].path);
		free(he->dest);
		he->dest = NULL;
		he->dest_nr = 0;
	}
	table_count--;
}

static int dest_insert(struct hash_entry *he, const char *path) {

	int i;
	struct destination *dest;

	/* Look if path already exists in entry. */
	for(i=0; i < he->dest_nr; i++) {

		if (!strcmp(he->dest[i].path, path))
			return -1;
	}

	he->dest = realloc(he->dest,
		sizeof(struct destination) * (he->dest_nr + 1));

	dest = he->dest + he->dest_nr++;
	dest->path = strdup(path);
	dest->time = time(NULL);

	return 0;
}

static void dest_remove(struct hash_entry *he, unsigned index) {

	struct destination *dest;

	if (he->dest_nr < index)
		return;

	dest = he->dest + index;
	if (dest->path)
		free(dest->path);

	memcpy(dest, he->dest + (--he->dest_nr), sizeof(*dest));
}

static void resize_table() {

	double load;
	unsigned int i, old_size = table_size;
	struct hash_entry *old = table;

	load = HASH_TABLE_LOAD(table_count, table_size);

	/* check if resize should be done */
	if ((load < 0.5 && table_size <= TABLE_MIN_SIZE) ||
		(load >= 0.5 && load <= 0.75))
		return;

	/*
	 * set size to a load factor that is in the
	 * middle in the valid range.
	 */
	table_size = table_count / 0.625;
	if (table_size < TABLE_MIN_SIZE)
		table_size = TABLE_MIN_SIZE;

	table_count = 0;
	table = calloc(sizeof(*table), table_size);

	for(i=0; i < old_size; i++) {
		struct hash_entry *he = old + i;
		if (!he_empty(he))
			he_insert(he);
	}
	free(old);
}

static void* read_entry_nr(void *buf, unsigned int *out) {

	memcpy(out, buf, sizeof(*out));
	*out = ntohl(*out);
	return buf + sizeof(*out);
}

static size_t parse_destination(char *buf, struct destination *dest) {

	size_t offset;

	buf = read_entry_nr(buf, &dest->time);
	offset = sizeof(dest->time);

	dest->path = strdup(buf);
	return offset + strlen(buf) + 1;
}

static void build_table(char *buf, size_t len) {

	size_t i;
	char *orig = buf;

	table = calloc(sizeof(*table), table_size);

	while(buf - orig < len) {
		unsigned dest_nr;
		struct hash_entry entry;

		entry.key = strdup(buf);

		buf = read_entry_nr(buf + strlen(entry.key) + 1, &dest_nr);

		entry.dest_nr = dest_nr;
		entry.dest = calloc(sizeof(struct destination), dest_nr);
		for(i=0; i < dest_nr; i++)
			buf += parse_destination(buf, entry.dest + i);

		he_insert(&entry);
	}
}

int dlhist_open() {

	char filename[4096], *buf = NULL;
	int ret = -1, fd = -1, offset = 0;
	struct stat st;
	struct header *hdr;

	snprintf(filename, sizeof(filename),
		"%s/%s", env_get_dir(), STORAGE_FILE);

	/* try lockin the file */
	if (hold_lock(&lock, filename) < 0)
		goto error;

	fd = open(filename, O_CREAT | O_RDONLY, 0600);
	if (fd < 0 || fstat(fd, &st) < 0) {
		error("dlhist_open: %s", strerror(errno));
		goto error;
	}

	if (st.st_size >= sizeof(*hdr)) {

		buf = malloc(st.st_size);
		if (!buf)
			goto error;

		read(fd, buf, st.st_size);

		/* Validate header */
		hdr = (struct header *) buf;
		if (hdr->signature != htonl(SIGNATURE) ||
			hdr->version != htonl(1)) {
			error("dlhist_open: Invalid header");
			goto error;
		}

		/* Get current table size */
		table_size = htonl(hdr->size);

		offset = sizeof(*hdr);
	}

	if (table_size < 1)
		table_size = TABLE_MIN_SIZE;

	build_table(buf + offset, st.st_size - offset);

	ret = 0;
error:
	if (ret)
		release_lock(&lock);
	if (buf)
		free(buf);
	if (fd >= 0)
		close(fd);
	return ret;
}

int dlhist_lookup(const char *title, const char *dest) {

	struct hash_entry *he;

	if (table_size < 1)
		return 0;

	he = lookup(title);
	if (!he_empty(he)) {
		int i;
		for(i=0; i < he->dest_nr; i++) {

			if (file_cmp(he->dest[i].path, dest))
				return 1;
		}
	}
	return 0;
}

void dlhist_mark(const char *title, const char *dest) {

	struct hash_entry *he;

	if (table_size < 1)
		return;

	/* lookup a entry in the hashtable
	   and insert the destination. */
	he = lookup(title);
	dest_insert(he, dest);

	if (he_empty(he)) {
		he_set(he, title);
		resize_table();
	}
}

void dlhist_purge(unsigned int interval) {

	unsigned int i, t = time(NULL);
	int j;

	if (t < interval)
		return;

	t -= interval;
	for(i=0; i < table_size; i++) {
		struct hash_entry *entry = table + i;

		if (he_empty(entry))
			continue;

		for(j=entry->dest_nr-1; j >= 0; j--) {

			if (entry->dest[j].time <= t)
				dest_remove(entry, j);
		}

		if (entry->dest_nr < 1)
			he_remove(entry);
	}
	resize_table();
}

static int write_dest(int fd, struct destination *dest) {

	unsigned time;
	int rc, len = 0;

	time = htonl(dest->time);
	rc = write(fd, &time, sizeof(time));
	if (rc < 0)
		return -1;
	len += rc;

	rc = write(fd, dest->path, strlen(dest->path) + 1);
	if (rc < 0)
		return -1;

	return len + rc;
}

static inline void write_int(int fd, int val) {

	val = htonl(val);
	write(fd, &val, sizeof val);
}

void dlhist_flush() {

	int i;
	struct header hdr;
	int fd = lock.fd;

	if (table_size < 1)
		return;

	ftruncate(fd, 0);
	lseek(fd, 0, SEEK_SET);

	/* Write header */
	hdr.signature = htonl(SIGNATURE);
	hdr.version = htonl(1);
	hdr.size = htonl(table_size);

	write(fd, &hdr, sizeof(hdr));

	/* Write hash entries */
	for(i=0; i < table_size; i++) {
		int j;
		struct hash_entry *entry = table + i;

		if (he_empty(entry))
			continue;

		/* write key and the number of destinations. */
		write(fd, entry->key, strlen(entry->key) + 1);

		write_int(fd, entry->dest_nr);

		/* write destinations for this title. */
		for(j=0; j < entry->dest_nr; j++) {

			if (write_dest(fd, entry->dest + j) < 0)
				goto error;
		}
	}

	/* Flush it to the real file */
	commit_lock(&lock);
	return;
error:
	error("dlhist_flush: partial write");
}

void dlhist_close() {

	int i;

	dlhist_flush();

	release_lock(&lock);

	for(i=0; i < table_count; i++)
		he_remove(table + i);

	if (table)
		free(table);
	table = NULL;
	table_count = table_size = 0;
}


static char* strtime(time_t *time) {

	static char buf[64];

	struct tm *tm = gmtime(time);
	if (strftime(buf, sizeof(buf), "%F %T", tm) < 1)
		buf[0] = '\0';
	return buf;
}

void dlhist_print() {

	int i, j;

	for(i=0; i < table_size; i++) {
		struct hash_entry *entry = table + i;

		if (he_empty(entry))
			continue;

		printf("%s\n", entry->key);

		for(j=0; j < entry->dest_nr; j++) {
			struct destination *dest = entry->dest + j;

			printf("\t%s | %s\n",
				strtime((time_t*) &dest->time), dest->path);
		}
		printf("\n");
	}
}
