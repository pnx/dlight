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
#include <openssl/sha.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "env.h"
#include "lockfile.h"
#include "dlhist.h"

/* \195 D L H */
#define SIGNATURE 0xC3444C48
#define STORAGE_FILE "dlhist"

#define TABLE_MIN_SIZE 128

#define HASH_TABLE_LOAD(c, s) ((double) (c) / ((s) ? (s) : 1))

struct header {
	unsigned int signature;
	unsigned int version;
	unsigned int size;
};

union hash {
	unsigned int  index;
	unsigned char sha1[20];
};

struct hash_entry {
	union hash   hash;
	unsigned int time;
};

#define he_empty(x) (!(x) || (x)->hash.index == 0)

static struct lockfile lock = LOCKFILE_INIT;

static struct hash_entry *table;
static unsigned int table_size;
static unsigned int table_count;

static void hash(union hash *h, const char *s) {

	unsigned n = 0;
	const char *ptr;

	for(ptr = s; *ptr; ptr++) {
		if (!strncmp(ptr, "://", 3)) {
			n = 0;
			s = ptr;
		} else if (!strncmp(ptr, "/", 2)) {
			break;
		}
		n++;
	}
	SHA1((unsigned char *)s, n, h->sha1);
}

static struct hash_entry* lookup(const char *key) {

	unsigned int index;
	union hash h;

	hash(&h, key);

	index = h.index % table_size;

	/* linear probing */
	while(!he_empty(table + index)) {
		if (!memcmp(table[index].hash.sha1, h.sha1, 20))
			break;
		index = (index + 1) % table_size;
	}
	return table + index;
}

static inline void he_set(struct hash_entry *he, const char *key) {

	if (!he_empty(he))
		return;
	hash(&he->hash, key);
	table_count++;
}

static int he_insert(struct hash_entry *he) {

	struct hash_entry *dest = table + (he->hash.index % table_size);

	if (he_empty(dest)) {
		memcpy(dest, he, sizeof(*he));
		table_count++;
		return 1;
	}
	return 0;
}

static void he_remove(struct hash_entry *he) {

	memset(he, 0, sizeof(*he));
	table_count--;
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

static void build_table(const char *buf, size_t len) {

	size_t offset = 0;

	table = calloc(sizeof(*table), table_size);

	while(offset < len) {
		struct hash_entry entry;

		memcpy(&entry.hash, buf + offset, sizeof(entry.hash));
		offset += sizeof(entry.hash);

		memcpy(&entry.time, buf + offset, sizeof(entry.time));
		offset += sizeof(entry.time);

		entry.hash.index = ntohl(entry.hash.index);
		entry.time = ntohl(entry.time);

		he_insert(&entry);
	}
}

int dlhist_open() {

	char filename[4096], *buf = NULL;
	int ret = -1, fd = -1, offset = 0;
	struct stat st;
	struct header *hdr;

	/* Open file */
	snprintf(filename, sizeof(filename),
		"%s/%s", env_get_dir(), STORAGE_FILE);

	/* try lockin the file */
	if (hold_lock(&lock, filename, 0) < 0)
		goto error;

	fd = open(filename, O_CREAT | O_RDONLY, 0600);
	if (fd < 0 || fstat(fd, &st) < 0) {
		perror("dlhist_open");
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
			fprintf(stderr, "dlhist_open: Invalid header\n");
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

int dlhist_lookup(const char *url) {

	if (table_size < 1)
		return 0;
	return !he_empty(lookup(url));
}

void dlhist_update(const char *url) {

	struct hash_entry *he;

	if (table_size < 1)
		return;

	/*
	 * set time and key before resize,
	 * hash_entry pointer is invalid after that operation.
	 */
	he = lookup(url);
	he->time = time(NULL);
	if (he_empty(he)) {
		he_set(he, url);
		resize_table();
	}
}

void dlhist_purge(unsigned int timestamp) {

	unsigned int i, t = 0, now = time(NULL);

	if (now < timestamp)
		return;

	t = now - timestamp;
	for(i=0; i < table_size; i++) {
		struct hash_entry *entry = table + i;

		if (!he_empty(entry) && entry->time <= t)
			he_remove(entry);
	}

	resize_table();
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
		struct hash_entry ondisk, *entry = table + i;

		if (he_empty(entry))
			continue;

		memcpy(&ondisk.hash, &entry->hash, 20);
		ondisk.hash.index = htonl(entry->hash.index);
		ondisk.time = htonl(entry->time);

		write(fd, &ondisk.hash, 20);
		write(fd, &ondisk.time, sizeof(ondisk.time));
	}

	/* Flush it to the real file */
	commit_lock(&lock);
}

void dlhist_close() {

	dlhist_flush();

	release_lock(&lock);

	if (table)
		free(table);
	table = NULL;
	table_count = table_size = 0;
}
