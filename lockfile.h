/* lockfile.h
 *
 *   Copyright (C) 2011       Henrik Hautakoski <henrik@fiktivkod.org>
 *   Copyright (C) 2005       Junio C Hamano
 *
 *   Based on lockfile.h from Git.
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
#ifndef LOCKFILE_H
#define LOCKFILE_H

 /* Flags */
#define __LOCK_ON_LIST 		(1<<0)
#define __LOCK_LOCAL_STORAGE 	(1<<1)

struct lockfile {
	struct lockfile *next;
	int fd;
	unsigned char flags;
	char name[4096];
};

#define LOCKFILE_INIT 		{ 0, -1, 0, { 0 } }
#define LOCKFILE_INIT_LOCAL 	{ 0, -1, __LOCK_LOCAL_STORAGE, { 0 } }

#define is_locked(x) ((x)->fd >= 0)

int hold_lock(struct lockfile *lock, const char *filename);

int commit_lock(struct lockfile *lock);

int release_lock(struct lockfile *lock);

#endif /* LOCKFILE_H */
