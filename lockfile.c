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

/* Flags */
#define LOCK_ON_LIST (1<<0)

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

static double get_lock_time(const char *path) {

	struct stat st;

	if (stat(path, &st) < 0)
		return -1;
	return difftime(time(NULL), st.st_ctime);
}

static int open_lock(const char *path) {

	int fd, mask = O_WRONLY | O_TRUNC | O_CREAT | O_EXCL;

	fd = open(path, mask, 0600);
	if (fd < 0) {
		/* Force open if lockfile exists
		   and MAX_LOCK_TIME is exceeded */
		if (errno == EEXIST
			&& get_lock_time(path) > MAX_LOCK_TIME) {

			mask &= ~O_EXCL;
			return open(path, mask, 0600);
		}
	}
	return fd;
}

int hold_lock(struct lockfile *lock, const char *filename) {

	int rc;

	if (is_locked(lock))
		return -1;

	rc = snprintf(lock->name, sizeof(lock->name), "%s.lock", filename);
	if (rc > sizeof(lock->name))
		return -1;

	lock->fd = open_lock(lock->name);
	if (lock->fd < 0) {
		lock->name[0] = '\0';

		if (errno == EEXIST)
			return error("'%s' is locked", filename);

		return error("unable to create lockfile '%s'", lock->name);
	}

	/* Add the lock to the list if needed. */
	if (!(lock->flags & LOCK_ON_LIST)) {

		/* If the lock list is empty.
		   Need to register signal handler and atexit. */
		if (!active_locks) {
			register_signal_handl(sig_remove_from_list);
			atexit(release_all_locks);
		}

		lock->next = active_locks;
		active_locks = lock;
		lock->flags |= LOCK_ON_LIST;
	}
	return lock->fd;
}

int commit_lock(struct lockfile *lock) {

	char target[4096];
	int len;

	if (!is_locked(lock))
		return 0;

	len = strlen(lock->name) - 5; /* .lock */

	memcpy(target, lock->name, len);
	target[len] = '\0';

	if (rename(lock->name, target))
		return -1;
	lock->name[0] = '\0';
	close(lock->fd);
	lock->fd = -1;
	return 0;
}

int release_lock(struct lockfile *lock) {

	int rc;

	if (!is_locked(lock))
		return 0;
	rc = unlink(lock->name);
	if (rc == 0) {
		lock->name[0] = '\0';
		close(lock->fd);
		lock->fd = -1;
	}
	return rc;
}
