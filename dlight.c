
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
#include "filter.h"
#include "http.h"
#include "rss.h"

#define DLHIST_PURGE_INTERVAL (60*60*6) /* 6 hours (in seconds) */

static int write_http_file(struct http_file *file, const char *dest) {

	char path[4096];
	int rc, fd;

	snprintf(path, sizeof(path), "%s/%s",
		dest, file->filename);

	fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0664);
	if (fd < 0 && errno != EEXIST) {
		error("failed to write file: %s\n", path);
		return -1;
	}

	rc = write(fd, file->data.block, file->data.len);
	close(fd);

	return rc;
}

static void process_items(rss_t rss, struct target *t) {

	int i;
	struct rss_item item;

	while(rss_walk_next(rss, &item)) {

		struct http_file *file = NULL;

		if (dlhist_lookup(item.link))
			continue;

		for(i=0; i < t->nr; i++) {
			struct filter *filter = &t->filter[i];

			if (!filter_match(filter->pattern, item.title))
				continue;

			/* fetch the file if we haven't already. */
			if (file == NULL) {
				file = http_fetch_file(item.link);
				if (file == NULL) {
					error("download failed: %s",
						strerror(errno));
					continue;
				}
			}

			/* save file to disk. */
			if (write_http_file(file, filter->dest) < 0)
				continue;

			printf("Downloaded: %s (%s) to %s\n",
				item.title, item.link, filter->dest);

			dlhist_update(item.link);
		}

		http_free_file(file);
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
