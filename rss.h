
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
