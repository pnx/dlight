
#ifndef CCONF_H
#define CCONF_H

/*
 * data structure for 'Dlight compiled config' file format.
 */

/* \232 D C C */
#define CCONF_SIGNATURE 0xe8444343
struct cconf_header {
	unsigned int signature;
	unsigned int version;
	unsigned char crc[20];
};

struct target {
	char *src; /* source. (url) */
	char *dest; /* destination, path on filesystem */
	char **filter;
	unsigned int nr;
};

struct cconf {
	struct target *target;
	unsigned int nr;
	struct {
		void *buf;
		unsigned long size;
	} map;
};

void cconf_free(struct cconf *c);

struct target* cconf_new_target(struct cconf *c);

void cconf_add_filter(struct target *t, char *filter);

int cconf_write(int fd, struct cconf *c);

struct cconf* cconf_read(const char *filename);

#endif /* CCONF_H */
