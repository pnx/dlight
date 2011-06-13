
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "lockfile.h"

#define locked(x) ((x)->fd >= 0)

int hold_lock(struct lockfile *lock, const char *filename, int force) {

	int rc, mask = O_WRONLY | O_CREAT | O_TRUNC;

	if (locked(lock))
		return -1;

	if (!force)
		mask |= O_EXCL;

	rc = snprintf(lock->name, sizeof(lock->name), "%s.lock", filename);
	if (rc > sizeof(lock->name))
		return -1;

	lock->fd = open(lock->name, mask, 0600);
	if (lock->fd < 0) {
		if (errno == EEXIST) {
			fprintf(stderr, "'%s' is locked\n", lock->name);
		} else {
			fprintf(stderr, "unable to create lockfile '%s'",
				lock->name);
		}
		return -1;
	}
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
		lock->name[0] = '\0';
		close(lock->fd);
		lock->fd = -1;
	}
	return rc;
}
