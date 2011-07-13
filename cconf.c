/* cconf.c
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/sha.h>
#include <arpa/inet.h>
#include "cconf.h"

/* we count NULL as part of the string ondisk */
#define strsize(str) (strlen(str) + 1)

static int sha1_write(SHA_CTX *ctx, int fd, void *buf, size_t size) {

	SHA1_Update(ctx, buf, size);
	return write(fd, buf, size);
}

static void write_int(SHA_CTX *ctx, int fd, int val) {

	val = htonl(val);
	sha1_write(ctx, fd, &val, sizeof val);
}

static void* read_entry_nr(void *buf, unsigned int *out) {

	memcpy(out, buf, sizeof(*out));
	*out = ntohl(*out);
	return buf + sizeof(*out);
}

void cconf_free(struct cconf *c) {

	int i, j;

	if (!c)
		return;

	if (c->map.buf) {
		free(c->target);
		munmap(c->map.buf, c->map.size);
	} else if (c->nr) {
		for(i=0; i < c->nr; i++) {
			struct target *t = c->target + i;
			free(t->src);
			for(j=0; j < t->nr; j++) {
				struct filter *f = t->filter + i;
				free(f->pattern);
				free(f->dest);
			}
			free(t->filter);
		}
		free(c->target);
	}
}

struct target* cconf_new_target(struct cconf *c) {

	struct target *t;

	c->target = realloc(c->target, (sizeof(struct target) * (c->nr + 1)));

	t = c->target + (c->nr++);
	memset(t, 0, sizeof(*t));

	return t;
}

void cconf_add_filter(struct target *t, struct filter *filter) {

	if (!filter || !filter->dest)
		return;

	t->filter = realloc(t->filter, sizeof(*t->filter) * (t->nr + 1));
	memcpy(&t->filter[t->nr++], filter, sizeof(*filter));
}

static size_t parse_filter(void *buf, struct target *target) {

	size_t offset = read_entry_nr(buf, &target->nr) - buf;

	if (target->nr) {
		int i;

		target->filter = malloc(sizeof(*target->filter) * target->nr);

		for(i=0; i < target->nr; i++) {
			struct filter *filter = &target->filter[i];

			filter->pattern = (char *) buf + offset;
			offset += strsize(buf + offset);

			filter->dest = (char *) buf + offset;
			offset += strsize(buf + offset);
		}
	}
	return offset;
}

static size_t parse_target(void *buf, struct target *target) {

	size_t offset;

	target->src = (char *) buf;
	offset = strsize(buf);

	return offset;
}

static struct cconf* parse(void *buf, size_t size) {

	struct cconf *c = calloc(1, sizeof(struct cconf));
	int i;

	/* move! */
	c->map.buf = buf;
	c->map.size = size;

	buf = read_entry_nr(buf + sizeof(struct cconf_header), &c->nr);

	c->target = calloc(sizeof(struct target), c->nr);

	for(i=0; i < c->nr; i++) {
		struct target *target = c->target + i;

		buf += parse_target(buf, target);
		buf += parse_filter(buf, target);
	}
	return c;
}

static int validate_hdr(struct cconf_header *hdr, size_t size) {

	SHA_CTX ctx;
	unsigned char sha1[20];

	if (hdr->signature != htonl(CCONF_SIGNATURE) ||
		hdr->version != htonl(1))
		return -1;
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, hdr, offsetof(struct cconf_header, crc));
	SHA1_Update(&ctx, hdr + 1, size - sizeof(*hdr));
	SHA1_Final(sha1, &ctx);
	if (memcmp(sha1, hdr->crc, sizeof(hdr->crc)))
		return -1;
	return 0;
}

struct cconf* cconf_read(const char *file) {

	int fd;
	struct stat st;
	void *buf;

	fd = open(file, O_RDONLY);
	if (fd < 0)
		return NULL;
	if (fstat(fd, &st) < 0) {
		close(fd);
		return NULL;
	}

	buf = MAP_FAILED;
	if (!fstat(fd, &st) && st.st_size > sizeof(struct cconf_header)) {
		buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	}
	close(fd);

	if (buf == MAP_FAILED)
		return NULL;

	if (validate_hdr(buf, st.st_size) < 0)
		goto error;

	return parse(buf, st.st_size);
error:
	munmap(buf, st.st_size);
	return NULL;
}

int cconf_write(int fd, struct cconf *c) {

	int i;
	SHA_CTX ctx;
	struct cconf_header hdr;

	hdr.signature = htonl(CCONF_SIGNATURE);
	hdr.version = htonl(1);

	SHA1_Init(&ctx);
	SHA1_Update(&ctx, &hdr, offsetof(struct cconf_header, crc));

	/* leave room for the header to be written later as CRC
	   will be calculated as we write the rest of the data */
	lseek(fd, sizeof(hdr), SEEK_SET);

	/* put number of targets */
	write_int(&ctx, fd, c->nr);

	for(i = 0; i < c->nr; i++) {
		int j;
		struct target *target = c->target + i;

		if (!target->src)
			return -1;

		sha1_write(&ctx, fd, target->src, strsize(target->src));

		/* write number of filters */
		write_int(&ctx, fd, target->nr);

		for(j=0; j < target->nr; j++) {
			struct filter *f = &target->filter[j];

			sha1_write(&ctx, fd, f->pattern, strsize(f->pattern));
			sha1_write(&ctx, fd, f->dest, strsize(f->dest));
		}
	}

	SHA1_Final(hdr.crc, &ctx);

	/* write header */
	lseek(fd, 0, SEEK_SET);
	sha1_write(&ctx, fd, &hdr, sizeof(hdr));

	return 0;
}
