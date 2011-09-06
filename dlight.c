
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "env.h"
#include "error.h"
#include "cconf.h"
#include "dlhist.h"
#include "filter.h"
#include "http.h"
#include "rss.h"

#define DLHIST_PURGE_INTERVAL (60*60*6) /* 6 hours (in seconds) */

static void process_items(rss_t rss, struct target *t) {

	int i;
	struct rss_item item;

	while(rss_walk_next(rss, &item)) {

		if (dlhist_lookup(item.link))
			continue;

		for(i=0; i < t->nr; i++) {
			struct filter *filter = &t->filter[i];

			if (!filter_match(filter->pattern, item.title))
				continue;

			if (http_download_file(item.link, filter->dest) < 0 &&
				errno != EEXIST) {
				error("download failed: %s", strerror(errno));
				continue;
			}

			printf("Downloaded: %s\n", item.title);

			dlhist_update(item.link);
		}
	}
}

static void process(struct cconf *config) {

	int i;
	struct http_data *data;

	dlhist_purge(DLHIST_PURGE_INTERVAL);

	for(i=0; i < config->nr; i++) {
		struct target *t = config->target + i;
		rss_t rss;

		data = http_fetch_page(t->src);
		if (!data)
			continue;

		rss = rss_parse(data->block, data->len);
		if (!rss) {
			error("failed to parse rss: %s\n", t->src);
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

	if (dlhist_open() < 0) {
		perror("dlhist");
		return 1;
	}

	process(config);

	dlhist_close();
	cconf_free(config);

	return 0;
}
