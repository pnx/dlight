
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "env.h"
#include "error.h"
#include "cconf.h"
#include "dlhist.h"
#include "proc-cache.h"
#include "filter.h"
#include "http.h"
#include "rss.h"

#define PROC_CACHE_PURGE_INTERVAL (60*60*6) /* 6 hours (in seconds) */
#define DLHIST_PURGE_INTERVAL (60*60*24) /* 1 day */

static int write_http_file(struct http_file *file, const char *dest) {

	char path[4096];
	snprintf(path, sizeof(path), "%s/%s", dest, file->filename);
	return buffer_write(&file->data, path);
}

static void process_items(rss_t rss, struct target *t) {

	int i;
	struct rss_item item;

	while(rss_walk_next(rss, &item)) {

		struct http_file *file = NULL;

		if (proc_cache_lookup(item.link))
			continue;

		for(i=0; i < t->nr; i++) {
			struct filter *filter = &t->filter[i];

			if (dlhist_lookup(item.title, filter->dest) ||
				!filter_match(filter->pattern, item.title))
				continue;

			/* fetch the file if we haven't already. */
			if (file == NULL) {
				file = http_fetch_file(item.link);
				if (file == NULL) {
					error("download failed");
					continue;
				}
			}

			/* At this point, mark the item as downloaded.
			   Even if we encounter an error while saving to disk. */
			dlhist_mark(item.title, filter->dest);

			if (write_http_file(file, filter->dest) < 0)
				continue;

			printf("Downloaded: %s (%s) to %s\n",
				item.title, item.link, filter->dest);

			proc_cache_update(item.link);
		}

		http_free_file(file);
	}
}

static void process(struct cconf *config) {

	int i;
	struct buffer *data;

	proc_cache_purge(PROC_CACHE_PURGE_INTERVAL);
	dlhist_purge(DLHIST_PURGE_INTERVAL);

	for(i=0; i < config->nr; i++) {
		struct target *t = config->target + i;
		rss_t rss;

		data = http_fetch_page(t->src);
		if (!data)
			continue;

		rss = rss_parse(data->block, data->len);
		if (!rss) {
			error("failed to parse rss: %s", t->src);
			continue;
		}

		process_items(rss, t);
		rss_free(rss);
		http_free(data);
	}
}

int main(int argc, char *argv[]) {

	struct cconf *config;
	char configfile[4096];

	snprintf(configfile, sizeof(configfile), "%s/%s",
		env_get_dir(), "config");

	config = cconf_read(configfile);
	if (!config) {
		perror(configfile);
		return 1;
	}

	/* open process cache and download history. */
	if (proc_cache_open() < 0 || dlhist_open() < 0)
		return 1;

	process(config);

	proc_cache_close();
	dlhist_close();
	cconf_free(config);

	return 0;
}
