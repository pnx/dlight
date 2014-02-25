/* llist.h
 *
 *   Copyright (C) 2011   Henrik Hautakoski <henrik@fiktivkod.org>
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
 *
 *   Inspired by include/linux/list.h in Linux Kernel.
 */

#ifndef LLIST_H
#define LLIST_H

#include <stddef.h>

struct llist {
	struct llist *next;
};

#define LLIST_INIT { NULL }

#define llist_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

static inline void llist_add(struct llist *list, struct llist *new) {

	for(; list->next; list = list->next);

	list->next = new;
	new->next = NULL;
}

static inline void llist_unlink(struct llist *prev, struct llist *entry) {

	if (prev != entry) {
		prev->next = entry->next;
		entry->next = NULL;
	}
}

static inline void llist_del(struct llist *list, struct llist *entry) {

	for(; list->next; list = list->next) {

		if (list->next == entry) {
			llist_unlink(list, entry);
			break;
		}
	}
}

static inline int llist_empty(struct llist *list) {

	return !list || list->next == NULL;
}

#define llist_foreach(pos, head) \
	for(pos = (head); pos; pos = pos->next)

#define llist_foreach_entry(pos, head, member) \
	for(pos = llist_entry(head, typeof(*pos), member); \
		pos->member; pos = llist_entry(pos->member.next, \
		typeof(*pos), member))

#define llist_foreach_safe(it, n, head) \
	for(it = (head), n = it->next; it; \
		it = n, n = (n) ? it->next : NULL)

#endif /* LLIST_H */