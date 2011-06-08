
#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

struct http_data {
	void *block;
	size_t len;
};

struct http_data* http_fetch_page(const char *url);

int http_download_page(const char *url, const char *file);

int http_download_file(const char *url, const char *dir);

void http_free(struct http_data *data);

#endif
