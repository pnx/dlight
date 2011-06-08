
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "env.h"
#include "cconf.h"
#include "dlhist.h"
#include "filter.h"
#include "http.h"
#include "rss.h"

#define error(...) fprintf(stderr, "error: " __VA_ARGS__)

static void process_items(rss_t rss, struct target *t) {

	struct rss_item item;

	while(rss_walk_next(rss, &item)) {

		if (!filter_match_list(t->filter, t->nr, item.title)
			|| dlhist_lookup(item.link)) {
			continue;
		}

		if (http_download_file(item.link, t->dest) < 0 &&
			errno != EEXIST) {
			error("download failed: %s\n", strerror(errno));
			continue;
		}

		dlhist_update(item.link);
	}
}

static void process(struct cconf *config) {

	int i;
	struct http_data *data;

	dlhist_purge(7200);

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
