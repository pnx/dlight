/* array.c
 *
 *   Copyright (C) 2010, 2012  Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#include <stdlib.h>
#include <string.h>
#include "xalloc.h"
#include "array.h"

struct __array {
	void	*items;
	unsigned nr;
};

static void resize(struct __array *arr, unsigned n) {

	if (!arr->nr) {
		xfree(arr->items);
		arr->items = NULL;
		return;
	}
	arr->items = xrealloc(arr->items, n * arr->nr);
}

void* array_create(void) {

	struct __array *arr = xmalloc(sizeof(struct __array));
	arr->items = NULL;
	arr->nr = 0;
	return arr;
}

void* __array_copy(void *__arr, unsigned n) {

	struct __array *arr = __arr, *copy;

	if (arr && arr->nr && n) {
		copy = xmalloc(sizeof(*arr));
		copy->items = xmemdup(arr->items, n * arr->nr);
		copy->nr = arr->nr;
		return copy;
	}
	return NULL;
}

int array_destroy(void *arr) {

	array_clear(arr);
	xfree(arr);
	return 0;
}

void __array_clear_fn(void *__arr, array_clear_fn_t *fn,
		unsigned n) {

	struct __array *arr = __arr;

	if (arr->items) {
		if (fn) {
			int i;
			for(i=0; i < arr->nr; i++)
				fn(arr->items + (i * n));
		}
		xfree(arr->items);
	}
	arr->items = NULL;
	arr->nr = 0;
}

int __array_insert(void *__arr, const void *item, unsigned n) {

	struct __array *arr = __arr;
	if (arr) {
		arr->items = xrealloc(arr->items, n * (++arr->nr));
		memcpy(arr->items + ((arr->nr - 1) * n), item, n);
		return arr->nr;
	}
	return -1;
}

void* __array_remove(void *__arr, unsigned i, unsigned n) {

	struct __array *arr = __arr;
	void *item = NULL;

	if (i < arr->nr) {
		/* I bet this will come back and bite me in the ass. */
		item = arr->items + (i * n);
		if (i < --arr->nr) {
			memmove(arr->items + (i * n),
			arr->items + ((i + 1) * n),
			(arr->nr - i) * n);
		}
		resize(arr, n);
	}
	return item;
}

void* __array_reduce(void *__arr, unsigned n) {

	struct __array *arr = __arr;
	void *item = NULL;

	if (arr->nr) {
		item = arr->items + ((--arr->nr) * n);
		resize(arr, n);
	}
	return item;
}

int __array_indexof(void *__arr, const void *item, unsigned n) {

	int i;
	struct __array *arr = __arr;

	for(i=0; i < arr->nr; i++) {

		if (!memcmp(arr->items + (i * n), item, n))
			return i;
	}
	return -1;
}

void* __array_lookup(void *__arr, const void *item,
		array_cmp_fn_t fn, unsigned n) {

	int i;
	struct __array *arr = __arr;

	if (fn) {
		for(i=0; i < arr->nr; i++) {
			void *cmp = arr->items + (i * n);
			if (fn(cmp, item) == 0)
				return cmp;
		}
	} else {
		i = __array_indexof(arr, item, n);
		if (i >= 0)
			return arr->items + (i * n);
	}
	return NULL;
}
