/* xalloc.c - stricter memory allocation.
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#include <string.h>
#include <errno.h>
#include "error.h"
#include "xalloc.h"

#ifdef __DEBUG__
# define CHECK_INPUT(s, prefix) \
	if (!(s))		\
		fatal(prefix ": Invalid argument '%s'\n", #s)
#else
# define CHECK_INPUT(s, prefix)
#endif

void* xmalloc(size_t size) {

	void *ptr;

	CHECK_INPUT(size, "xmalloc");

	ptr = malloc(size);
	if (!ptr)
		fatal("xmalloc: %s, tried to allocate %lu bytes",
			strerror(errno), (unsigned long) size);
	return ptr;
}

void* xmallocz(size_t size) {

	void *ptr;

	CHECK_INPUT(size, "xmallocz");

	ptr = malloc(size);
	if (!ptr)
		fatal("xmallocz: %s, tried to allocate %lu bytes",
			strerror(errno), (unsigned long) size);
	memset(ptr, 0, size);
	return ptr;
}

void* xrealloc(void *ptr, size_t size) {

	void *new;
	CHECK_INPUT(size, "xrealloc");

	new = realloc(ptr, size);
	if (!new)
		fatal("xrealloc: Can't resize memory block (%s) on '%p' with size '%lu'",
			strerror(errno), ptr, (unsigned long) size);
	return new;
}

char* xstrdup(const char *s) {

	size_t len;
	char *dest;

	CHECK_INPUT(s, "xstrdup");

	len = strlen(s) + 1;
	dest = xmalloc(len);
	memcpy(dest, s, len);
	return dest;
}

void* xmemdup(const void *src, size_t size) {

	void *dest;

	CHECK_INPUT(src, "xmemdup");

	dest = xmalloc(size);
	memcpy(dest, src, size);
	return dest;
}

#ifdef __DEBUG__
void xfree(void *ptr) {

	CHECK_INPUT(ptr, "xfree");
	free(ptr);
}
#endif /* __DEBUG__ */
