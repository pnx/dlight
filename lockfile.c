/* lockfile.c
 *
 *   Copyright (C) 2011       Henrik Hautakoski <henrik@fiktivkod.org>
 *   Copyright (C) 2005       Junio C Hamano
 *
 *   Based on lockfile.c from Git.
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "error.h"
#include "lockfile.h"

#define locked(x) ((x)->fd >= 0)

static struct lockfile *active_locks;

static void release_all_locks() {

	struct lockfile *lock = active_locks;

	while(lock) {
		if (lock->name[0]) {
			if (lock->fd >= 0)
				close(lock->fd);
			unlink(lock->name);
		}
		lock = lock->next;
	}
}

static void remove_from_list(struct lockfile *lock) {

	struct lockfile *it;

	if (active_locks == lock) {
		active_locks = lock->next = NULL;
		return;
	}

	for(it = active_locks; it; it = it->next) {

		if (it->next != lock)
			continue;

		it->next = lock->next;
		lock->next = NULL;
	}
}

static void register_signal_handl(void (*handl)(int)) {

	signal(SIGINT, handl);
	signal(SIGHUP, handl);
	signal(SIGTERM, handl);
	signal(SIGQUIT, handl);
	signal(SIGPIPE, handl);
}

static void sig_remove_from_list(int signo) {

	release_all_locks();
	signal(signo, SIG_DFL);
	raise(signo);
}

static inline void init(void) {

	static int is_init = 0;

	if (is_init)
		return;

	register_signal_handl(sig_remove_from_list);
	atexit(release_all_locks);
	is_init = 1;
}

int hold_lock(struct lockfile *lock, const char *filename, int force) {

	int rc, mask = O_WRONLY | O_CREAT | O_TRUNC;

	init();

	if (locked(lock))
		return -1;

	if (!force)
		mask |= O_EXCL;

	rc = snprintf(lock->name, sizeof(lock->name), "%s.lock", filename);
	if (rc > sizeof(lock->name))
		return -1;

	lock->fd = open(lock->name, mask, 0600);
	if (lock->fd < 0) {
		return error(errno == EEXIST ?
			"'%s' is locked" : "unable to create lockfile '%s'",
			lock->name);
	}
	lock->next = active_locks;
	active_locks = lock;
	return lock->fd;
}

int commit_lock(struct lockfile *lock) {

	char target[4096];
	int len;

	if (!locked(lock))
		return 0;

	len = strlen(lock->name) - 5; /* .lock */

	memcpy(target, lock->name, len);
	target[len] = '\0';

	if (rename(lock->name, target))
		return -1;
	remove_from_list(lock);
	lock->name[0] = '\0';
	close(lock->fd);
	lock->fd = -1;
	return 0;
}

int release_lock(struct lockfile *lock) {

	int rc;

	if (!locked(lock))
		return 0;
	rc = unlink(lock->name);
	if (rc == 0) {
		remove_from_list(lock);
		lock->name[0] = '\0';
		close(lock->fd);
		lock->fd = -1;
	}
	return rc;
}
