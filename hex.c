/* hex.c
 *
 *   Copyright (C) 2013   Henrik Hautakoski <henrik@fiktivkod.org>
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
#include "hex.h"

/* Giant table to lookup hex character values */
const char hex_value_table[256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, /* 0-7 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 8-15 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 16-23 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 24-31 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 32-39 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 40-47 */
	 0,  1,  2,  3,  4,  5,  6,  7, /* 48-55 */
	 8,  9, -1, -1, -1, -1, -1, -1, /* 56-63 */
	-1, 10, 11, 12, 13, 14, 15, -1, /* 64-71 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 72-79 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 80-87 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 88-95 */
	-1, 10, 11, 12, 13, 14, 15, -1, /* 96-103 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 104-111 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 112-119 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 120-127 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 128-135 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 136-143 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 144-151 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 152-159 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 160-167 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 168-175 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 176-183 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 184-191 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 192-199 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 200-207 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 208-215 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 216-223 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 224-231 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 232-239 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 240-247 */
	-1, -1, -1, -1, -1, -1, -1, -1, /* 248-255 */
};

int strhextodec(const char *str) {

	int v = 0;

	while(*str && v >= 0) {
		unsigned char dec = *str++;
		v = (v << 4) | hex_value_table[dec];
	}
	return v;
}
