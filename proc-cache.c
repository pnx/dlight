/* proc-cache.c
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
#include "llist.h"
#include "hash.h"
#include "lockfile.h"
#include "proc-cache.h"

/* \175 D P C */
#define SIGNATURE 0xAF445043
#define STORAGE_FILE "proc-cache"

struct header {
	unsigned int signature;
	unsigned int version;
	unsigned int entries;
};

union hash {
	unsigned int  index;
	unsigned char sha1[20];
};

/* hash entry flags */
#define HE_FLAG_VALID (1 << 0)

/*
 * NOTE: be sure to change this constant if the struct's size changes.
 */
#define HE_SZ (sizeof(union hash) + sizeof(unsigned))
struct proc_cache_entry {
	union hash   hash;
	unsigned int time;
	unsigned int flags;
	struct llist list;
};

#define he_empty(x) (!(x) || !((x)->flags & HE_FLAG_VALID))

static struct lockfile lock = LOCKFILE_INIT;
static struct hash_table table = HASH_TABLE_INIT;

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

static struct proc_cache_entry* lookup(const char *key) {

	struct proc_cache_entry *entry;
	union hash h;

	hash(&h, key);

	entry = hash_lookup(&table, h.index);
	if (entry) {
		struct llist *it;
		struct proc_cache_entry *e;

		llist_foreach(it, entry->list.next) {
			e = llist_entry(it, struct proc_cache_entry, list);

			if (!he_empty(e) && !memcmp(e->hash.sha1, h.sha1, 20))
				return e;
		}
	}
	return NULL;
}

static void he_insert(struct proc_cache_entry *entry) {

	struct proc_cache_entry *dest;

	dest = hash_insert(&table, entry->hash.index, entry);
	if (dest) {
		struct llist *it;
		struct proc_cache_entry *e;

		llist_foreach(it, &dest->list) {
			e = llist_entry(it, struct proc_cache_entry, list);

			if (!he_empty(e) &&
				!memcmp(e->hash.sha1, entry->hash.sha1, 20)) {
				free(entry);
				return;
			}
		}
		llist_add(&dest->list, &entry->list);
	}

	entry->flags |= HE_FLAG_VALID;
}

static void build_table(const char *buf, size_t entries) {

	size_t i, offset = 0;

	for(i=0; i < entries; i++) {
		struct proc_cache_entry *entry = calloc(1, sizeof(*entry));

		memcpy(&entry->hash, buf + offset, sizeof(entry->hash));
		offset += sizeof(entry->hash);

		memcpy(&entry->time, buf + offset, sizeof(entry->time));
		offset += sizeof(entry->time);

		entry->hash.index = ntohl(entry->hash.index);
		entry->time = ntohl(entry->time);

		he_insert(entry);
	}
}

int proc_cache_open() {

	char filename[4096], *buf = NULL;
	int ret = -1, fd = -1;
	size_t entries = 0, offset = 0;
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
		perror("proc_cache_open");
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
			fprintf(stderr, "proc_cache_open: Invalid header\n");
			goto error;
		}

		entries = htonl(hdr->entries);

		offset = sizeof(*hdr);
	}

	if (entries * HE_SZ > st.st_size - offset) {
		fprintf(stderr,
			"proc_cache_open: file truncated. "
			"expected atleast '%lu' bytes, got '%lu'\n",
			entries * HE_SZ, st.st_size - offset);
		goto error;
	}

	build_table(buf + offset, entries);

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

int proc_cache_lookup(const char *url) {

	return lookup(url) != NULL;
}

void proc_cache_update(const char *url) {

	struct proc_cache_entry *entry = lookup(url);

	if (!entry) {
		entry = calloc(1, sizeof(*entry));
		hash(&entry->hash, url);
		he_insert(entry);
	}
	entry->time = time(NULL);
}

void proc_cache_purge(unsigned int timestamp) {

	unsigned int i, t, now = time(NULL);

	if (now < timestamp)
		return;

	t = now - timestamp;
	for(i=0; i < table.size; i++) {
		struct llist *it;
		struct proc_cache_entry *e, *entry = hash_entry(&table, i);

		if (!entry)
			continue;

		llist_foreach(it, &entry->list) {
			e = llist_entry(it, struct proc_cache_entry, list);
			if (!he_empty(e) && e->time <= t)
				e->flags &= ~HE_FLAG_VALID;
		}
	}
}

static void write_entry(int fd, struct proc_cache_entry *entry) {

	struct proc_cache_entry ondisk;

	memcpy(&ondisk.hash, &entry->hash, 20);
	ondisk.hash.index = htonl(entry->hash.index);
	ondisk.time = htonl(entry->time);

	write(fd, &ondisk.hash, 20);
	write(fd, &ondisk.time, sizeof(ondisk.time));
}

void proc_cache_close() {

	int i;
	struct header hdr;
	int fd = lock.fd;

	if (table.size < 1)
		return;

	hdr.signature = htonl(SIGNATURE);
	hdr.version = htonl(1);
	hdr.entries = 0;

	ftruncate(fd, 0);
	lseek(fd, sizeof(hdr), SEEK_SET);

	/* Write hash entries */
	for(i=0; i < table.size; i++) {
		struct llist *it;
		struct proc_cache_entry *entry = hash_entry(&table, i);

		if (!entry)
			continue;

		llist_foreach(it, &entry->list) {
			struct proc_cache_entry *e;

			e = llist_entry(it, struct proc_cache_entry, list);
			if (!he_empty(e)) {
				write_entry(fd, e);
				hdr.entries++;
			}
		}
	}

	hdr.entries = htonl(hdr.entries);

	/* Write header */
	lseek(fd, 0, SEEK_SET);
	write(fd, &hdr, sizeof(hdr));

	/* Flush it to the real file */
	commit_lock(&lock);

	release_lock(&lock);

	hash_free(&table);
}
