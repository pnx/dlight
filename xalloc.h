/* xalloc.h - stricter memory allocation.
 *
 *   Copyright (C) 2010   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#ifndef __XALLOC_H
#define __XALLOC_H

#include <stdlib.h>

void* xmalloc(size_t);

void* xmallocz(size_t);

void* xrealloc(void *, size_t);

char* xstrdup(const char *);

void* xmemdup(const void *, size_t);

#ifdef __DEBUG__
void xfree(void *);
#else
#define xfree free
#endif /* __DEBUG__ */

#endif /* __XALLOC_H */
