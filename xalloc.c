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
#include "std_compat.h"
#include "xalloc.h"

#ifdef __DEBUG__
# define CHECK_INPUT(s) \
	if (!(s))	\
		fatal("%s: Invalid argument '%s'\n", __FUNC__, #s)
#else
# define CHECK_INPUT(s)
#endif


static inline void* __alloc(size_t s, const char *func) {

	void *p = malloc(s);
	if (!p)
		fatal("%s: %s, tried to allocate %lu bytes",
			func, strerror(errno), (unsigned long) (s));
	return p;
}

#define alloc(size) __alloc(size, __FUNC__)


void* xmalloc(size_t size) {

	void *ptr;

	CHECK_INPUT(size);
	ptr = alloc(size);
	return ptr;
}

void* xmallocz(size_t size) {

	void *ptr;

	CHECK_INPUT(size);

	ptr = alloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void* xrealloc(void *ptr, size_t size) {

	void *new;
	CHECK_INPUT(size);

	new = realloc(ptr, size);
	if (!new)
		fatal("xrealloc: Can't resize memory block (%s) on '%p' with size '%lu'",
			strerror(errno), ptr, (unsigned long) size);
	return new;
}

char* xstrdup(const char *s) {

	size_t len;
	char *dest;

	CHECK_INPUT(s);

	len = strlen(s) + 1;
	dest = alloc(len);
	memcpy(dest, s, len);
	return dest;
}

void* xmemdup(const void *src, size_t size) {

	void *dest;

	CHECK_INPUT(src);

	dest = alloc(size);
	memcpy(dest, src, size);
	return dest;
}

#ifdef __DEBUG__
void xfree(void *ptr) {

	CHECK_INPUT(ptr);
	free(ptr);
}
#endif /* __DEBUG__ */
