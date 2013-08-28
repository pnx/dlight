/* integer.c
 *
 *   Copyright (C) 2012-2013   Henrik Hautakoski <henrik@fiktivkod.org>
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
#include "../hex.h"

#define isdigit(x) ((x) >= '0' && (x) <= '9')

enum state { START, DIGIT, ZERO, HEX_START, HEX, REJECT, ACCEPT };

/* Define End of Input, must not be part of the alphabet. */
#define EOI '$'

int int_parse(int *out, char *buf, unsigned len) {

	int val = 0, pos;
	enum state state = START;

	for(pos=0;;pos++) {
		int c = (pos < len) ? buf[pos] : EOI;

		switch(state) {
		case START :
			if (c == '0') {
				state = ZERO;
			} else if (c >= '1' && c <= '9') {
				val = c - '0';
				state = DIGIT;
			} else {
				state = REJECT;
			}
			break;
		case DIGIT :
			if (isdigit(c)) {
				val = (val * 10) + (c - '0');
				state = DIGIT;
			} else {
				state = ACCEPT;
			}
			break;
		case HEX_START:
			c = hextodec(c);
			if (c < 0) {
				state = REJECT;
			} else {
				val = c;
				state = HEX;
			}
			break;
		case HEX :
			c = hextodec(c);
			if (c < 0) {
				state = ACCEPT;
			} else {
				val = (val << 4) | c;
			}
			break;
		case ZERO :
			if (c == 'x') {
				state = HEX_START;
			} else {
				state = ACCEPT;
			}
			break;
		case REJECT : return -1;
		case ACCEPT : goto end;
		}
	}

end:	*out = val;

	/* return the number of characters accepted. */
	return pos - 1;
}
