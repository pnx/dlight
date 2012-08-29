/* array.h
 *
 *   Copyright (C) 2010, 2012   Henrik Hautakoski <henrik@fiktivkod.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 */

#ifndef __ARRAY_H
#define __ARRAY_H

#include <stddef.h>

/*
 * This macro should be used to define the array struct:
 * struct myarray ARRAY(struct element);
 */
#define ARRAY(type)	\
	type *__items;	\
	unsigned __nr;

#define ARRAY_INIT { NULL, 0 }

typedef void (array_clear_fn_t)(void*);
typedef int (array_cmp_fn_t)(const void *, const void *b);

/* public API */
void* array_create(void);

#define array_copy(a) __array_copy(a, array_ele_size(a))

int array_destroy(void *arr);

#define array_clear(a) __array_clear_fn(a, NULL, -1);

#define array_clear_fn(a, fn) __array_clear_fn(a, fn, array_ele_size(a));

#define array_insert(a, i) __array_insert(a, i, array_ele_size(a));

#define array_remove(a, i) __array_remove(a, i, array_ele_size(a));

#define array_reduce(a) __array_reduce(a, array_ele_size(a));

#define array_has(a, i, fn) \
	(__array_lookup(a, i, fn, array_ele_size(a)) != NULL)

#define array_size(a) ((a) ? (a)->nr : 0)

#define array_ele_size(a) sizeof(*((a)->__items))

#define array_isempty(a) (!(a) || (a)->nr == 0)

#define array_foreach(a, i)			\
	for(i = (a)->__items;			\
		i < ((a)->__items + (a)->__nr);	\
		i++)

/* internal */

void* __array_copy(void *__arr, unsigned n);

void __array_clear_fn(void *__arr,
			array_clear_fn_t *fn, unsigned n);

int __array_insert(void *__arr,
	const void *item, unsigned n);

void* __array_remove(void *__arr, unsigned i, unsigned n);

void* __array_reduce(void *__arr, unsigned n);

void* __array_lookup(void *__arr, const void *item,
		array_cmp_fn_t *fn, unsigned n);

int __array_indexof(void *__arr,
	const void *item, unsigned n);

#endif /* __ARRAY_H */
