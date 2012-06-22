/* lexer.c
 *
 *   Copyright (C) 2012   Henrik Hautakoski <henrik@fiktivkod.org>
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

#include <ctype.h>
#include "lexer.h"
#include "buffer.h"

static unsigned line_nr = 1;

static int next_char(FILE *fd) {

	int c, comment = 0;

	do {
		c = fgetc(fd);
		if (c == '\r') {
			c = fgetc(fd);
			if (c != '\n') {
				ungetc(c, fd);
				c = '\r';
			}
		}

		if (c == '\n') {
			line_nr++;
			if (comment)
				comment = 0;
		} else if (c == '#') {
			comment = 1;
		}
	} while(comment);

	return c;
}

enum lexer_state {
	LEX_STATE_NORMAL,
	LEX_STATE_STRING,
};

token_t lexer_getnext(FILE *fd) {

	enum lexer_state state = LEX_STATE_NORMAL;
	struct buffer buf = BUFFER_INIT;
	static token_t prev = { NONE, 0 };
	token_t ret = { NONE, line_nr };

	if (prev.type != NONE) {
		ret = prev;
		prev.type = NONE;
		prev.attr = 0;
		return ret;
	}

	for(;;) {
		token_t t = { NONE, line_nr , NULL };
		int c = next_char(fd);

		switch(c) {
		case EOF : t.type = EOI;
			break;
		case '\n' : t.type = EOL;
			break;
		case '\t' : t.type = TAB;
			break;
		case ',' : t.type = SEP;
			break;
		case '=' : t.type = ASSIGN;
			break;
		case '[' : t.type = LBRACKET;
			break;
		case ']' : t.type = RBRACKET;
			break;
		case '<' :
			c = next_char(fd);
			if (c == '-') {
				t.type = DEFINE;
				break;
			}
			ungetc(c, fd);
			c = '<';
		}

		if (state == LEX_STATE_STRING) {
			if (isspace(c) || t.type != NONE) {
				ret.attr = buffer_cstr_release(&buf);
				prev = t;
				break;
			}
			if (c == '\\')
				c = next_char(fd);
			if (!(isalpha(c) || c == '_'))
				ret.type = STRING;
			buffer_append_ch(&buf, c);
			continue;
		}

		if (t.type == NONE) {
			if (!isspace(c)) {
				ungetc(c, fd);
				ret.type = ID;
				state = LEX_STATE_STRING;
			}
			continue;
		}
		ret = t;
		goto out;
	}

out :	return ret;
}
