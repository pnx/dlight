/* sha1_io.h - sha1 file I/O wrappers.
 *
 *   Copyright (C) 2011-2012  Henrik Hautakoski <henrik@fiktivkod.org>
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
#ifndef SHA1_IO_H
#define SHA1_IO_H

#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/sha.h>

inline int sha1_write(SHA_CTX *ctx, int fd, void *buf, size_t size) {

	SHA1_Update(ctx, buf, size);
	return write(fd, buf, size);
}

/* This function makes sure that the integer is in
   network byte order before it is written to disk
   by 'sha1_write'. */
inline int sha1_write_int(SHA_CTX *ctx, int fd, int val) {

	val = htonl(val);
	return sha1_write(ctx, fd, &val, sizeof val);
}

#endif /* SHA1_IO_H */