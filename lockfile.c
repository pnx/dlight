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
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "error.h"
#include "lockfile.h"

/* Maximum time a lockfile can be uhm.. locked (in seconds)
   Used to prevent deadlocks when processes "forgets" to unlock. */

#define MAX_LOCK_TIME (15 * 60) /* 15 min is fine. */

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

	struct lockfile *it = active_locks, *prev = NULL;

	while(it) {

		if (it == lock) {
			if (prev) {
				prev->next = it->next;
			} else {
				active_locks = it->next;
			}

			lock->flags &= ~__LOCK_ON_LIST;
			lock->next = NULL;
			break;
		}
		prev = it;
		it = it->next;
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

static double get_lock_time(const char *path) {

	struct stat st;

	if (stat(path, &st) < 0)
		return -1;
	return difftime(time(NULL), st.st_ctime);
}

static int open_lock(const char *path) {

	int fd, tried_force = 0;

start:	fd = open(path, O_WRONLY | O_TRUNC | O_CREAT | O_EXCL, 0600);
	if (fd < 0 && !tried_force && errno == EEXIST
		&& get_lock_time(path) > MAX_LOCK_TIME) {

		/* Force open if lockfile exists
		and MAX_LOCK_TIME is exceeded */

		tried_force = 1;
		unlink(path);
		goto start;
	}
	return fd;
}

int hold_lock(struct lockfile *lock, const char *filename) {

	if (is_locked(lock))
		return -1;

	if (snprintf(lock->name, sizeof(lock->name),
		"%s.lock", filename) > sizeof(lock->name))
		return -1;

	lock->fd = open_lock(lock->name);
	if (lock->fd < 0) {
		lock->name[0] = '\0';

		if (errno == EEXIST)
			return error("'%s' is locked", filename);

		return error("unable to create lockfile '%s'", lock->name);
	}

	/* Add the lock to the list if needed. */
	if (!(lock->flags & __LOCK_ON_LIST)) {

		/* If the lock list is empty.
		   Need to register signal handler and atexit. */
		if (!active_locks) {
			register_signal_handl(sig_remove_from_list);
			atexit(release_all_locks);
		}

		lock->next = active_locks;
		active_locks = lock;
		lock->flags |= __LOCK_ON_LIST;
	}
	return lock->fd;
}

int commit_lock(struct lockfile *lock) {

	char target[4096];
	int len, fd;

	if (!is_locked(lock))
		return 0;

	if (lock->flags & __LOCK_LOCAL_STORAGE)
		remove_from_list(lock);

	fd = lock->fd;
	lock->fd = -1;
	if (close(fd) < 0)
		return -1;

	len = strlen(lock->name) - 5; /* .lock */

	memcpy(target, lock->name, len);
	target[len] = '\0';

	if (rename(lock->name, target))
		return -1;
	lock->name[0] = '\0';
	return 0;
}

int release_lock(struct lockfile *lock) {

	if (lock->flags & __LOCK_LOCAL_STORAGE)
		remove_from_list(lock);

	if (lock->name[0]) {

		int rc;

		if (lock->fd >= 0) {
			rc = close(lock->fd);
			lock->fd = -1;
			if (rc < 0)
				return -1;
		}
		rc = unlink(lock->name);
		lock->name[0] = '\0';
		if (rc < 0)
			return -1;
	}
	return 0;
}
