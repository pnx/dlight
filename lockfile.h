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

struct lockfile {
	struct lockfile *next;
	int fd;
	char name[4096];
};

#define LOCKFILE_INIT { 0, -1, { 0 } }

int hold_lock(struct lockfile *lock, const char *filename, int force);

int commit_lock(struct lockfile *lock);

int release_lock(struct lockfile *lock);

#endif /* LOCKFILE_H */