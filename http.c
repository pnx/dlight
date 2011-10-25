/* http.c
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
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "error.h"
#include "http.h"

struct http_file_req {
	char *filename;
	FILE *fd;
};

static char* strnstrr(const char *str, const char *needle, size_t size) {

	char *ptr;
	size_t len, pos;

	if (!needle || !*needle)
		return (char *) str;

	len = strlen(needle);
	pos = size;
	for(ptr=(char*)str; *ptr; ptr = memchr(ptr+1, *needle, pos-1)) {
		pos = size - (ptr - str);
		if (pos < len)
			break;
		if (!strncmp(ptr, needle, len))
			return ptr + len;
	}
	return NULL;
}

static const char* url_filename(const char *url) {

	const char *start = url;

	for(; *url; url++) {
		if (*url != '/')
			continue;
		if (*(url+1) == 0)
			break;
		start = url + 1;
	}
	return start;
}

#define HDR_CONDISP "Content-Disposition:"

static size_t hdr_fname_cb(void *src, size_t smemb, size_t nmemb, void *data) {

	int pos, size = smemb * nmemb;
	char *ptr = (char *) src;
	char **filename = (char**) data;

	if (*filename || size < sizeof(HDR_CONDISP)-1 ||
		memcmp(ptr, HDR_CONDISP, sizeof(HDR_CONDISP)-1))
		return size;

	pos = sizeof(HDR_CONDISP)-1;
	ptr = strnstrr(ptr + pos, "filename=\"", size);
	if (ptr) {
		int start, len;
		start = pos = ptr - ((char*) src);
		ptr = (char *) src;
		for(len=0;;len++) {
			if (ptr[pos] == '"' && ptr[pos-1] != '\\')
				break;
			if (++pos > size)
				return 0;
		}
		if (len)
			*filename = strndup(ptr + start, len);
	}
	return size;
}

static size_t write_cb(void *src, size_t smemb, size_t nmemb, void *data) {

	size_t size = smemb * nmemb;

	buffer_append(data, src, size);

	return size;
}

#define HTTPREQ_MEM	0
#define HTTPREQ_FILE	1
#define HTTPREQ_FILEMEM 2

static int http_request(const char *url, void *req, int mode) {

	CURL *handle;
	CURLcode res;
	int ret = 0;
	void *headerdata = NULL;

	handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(handle, CURLOPT_TIMEOUT, 10);

	if (mode == HTTPREQ_FILE) {
		struct http_file_req *freq = req;

		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, fwrite);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, freq->fd);

		headerdata = &freq->filename;
	} else {
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_cb);
		if (mode == HTTPREQ_FILEMEM) {
			struct http_file *file = req;

			req = &file->data;
			headerdata = &file->filename;
		}
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, req);
	}

	if (headerdata) {
		curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, hdr_fname_cb);
		curl_easy_setopt(handle, CURLOPT_HEADERDATA, headerdata);
	}

	res = curl_easy_perform(handle);
	if (res != CURLE_OK) {
		error("curl: (%s) %s", url, curl_easy_strerror(res));
		ret = -1;
	}

	curl_easy_cleanup(handle);

	return ret;
}

struct buffer* http_fetch_page(const char *url) {

	struct buffer *buf = malloc(sizeof(struct buffer));

	buffer_init(buf);

	if (http_request(url, buf, HTTPREQ_MEM) < 0) {
		buffer_free(buf);
		free(buf);
		return NULL;
	}
	return buf;
}

struct http_file* http_fetch_file(const char *url) {

	struct http_file *file = malloc(sizeof(struct http_file));

	file->filename = NULL;
	buffer_init(&file->data);

	if (http_request(url, file, HTTPREQ_FILEMEM) < 0) {
		http_free_file(file);
		return NULL;
	}

	if (!file->filename)
		file->filename = strdup(url_filename(url));

	return file;
}

int http_download_file(const char *url, const char *dir) {

	int err;
	char tmpfile[4096];
	struct http_file_req req = { 0 };

	/* Construct an filename from url. */
	snprintf(tmpfile, sizeof(tmpfile), "%s/%s", dir, url_filename(url));

	req.fd = fopen(tmpfile, "w");
	if (!req.fd)
		goto error;

	if (http_request(url, &req, HTTPREQ_FILE) < 0)
		goto error;

	if (req.filename) {
		/* found the real file in http header.
		   move the old file. */
		char realfile[4096];

		snprintf(realfile, sizeof(realfile), "%s/%s",
			dir, req.filename);

		if (rename(tmpfile, realfile) < 0)
			goto error;
		free(req.filename);
	} else {
		fclose(req.fd);
	}

	return 0;
error:
	err = errno;
	if (req.filename)
		free(req.filename);
	fclose(req.fd);
	errno = err;
	return -1;
}

void http_free(struct buffer *b) {

	if (b) {
		buffer_free(b);
		free(b);
	}
}

void http_free_file(struct http_file *file) {

	if (file) {
		buffer_free(&file->data);
		if (file->filename)
			free(file->filename);
		free(file);
	}
}
