
#ifndef LOCKFILE_H
#define LOCKFILE_H

struct lockfile {
	int fd;
	char name[4096];
};

#define LOCKFILE_INIT { -1, { 0 } }

int hold_lock(struct lockfile *lock, const char *filename, int force);

int commit_lock(struct lockfile *lock);

int release_lock(struct lockfile *lock);

#endif /* LOCKFILE_H */