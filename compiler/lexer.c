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

static int lx_whitespace(int c) {

	return c != '\n' && isspace(c);
}

static void lx_ungetc(int c, FILE *fd) {

	if (c == '\n')
		line_nr--;
	ungetc(c, fd);
}

static int lx_get_ch(FILE *fd) {

	int c = fgetc(fd);

	if (c == '\r') {
		c = fgetc(fd);
		if (c != '\n') {
			ungetc(c, fd);
			c = '\r';
		}
	}
	if (c == '\n')
		line_nr++;
	return c;
}

static int lx_get_ch_tok(FILE *fd) {

	int c, comment = 0;

	while((c = lx_get_ch(fd)) != EOF) {

		if (comment) {
			if (c == '\n') {
				lx_ungetc(c, fd);
				comment = 0;
			}
		} else if (c == '#') {
			comment = 1;
		} else if (!lx_whitespace(c)) {
			break;
		}
	}
	return c;
}

/*
 * Helper function to analyze the DEFINE token.
 * This functions expects that the last character read from
 * the filedescriptor is '<'.
 */
static token_type lx_analyze_define(FILE *fd) {

	int c = lx_get_ch_tok(fd);

	if (c != '-') {
		lx_ungetc(c, fd);
		return NONE;
	}
	return DEFINE;
}

/*
 * Helper function to match variable string tokens.
 * This can be specific strings that has a special meaning to
 * the language like keywords, integers etc.
 */
static token_type lx_analyze_var_type(FILE *fd) {

	int c = lx_get_ch_tok(fd);

	printf("Variable type '%c'\n", c);
	return STRING;
}

token_t lexer_getnext(FILE *fd) {

	token_t t = { NONE, line_nr , NULL };
	int c;

	c = lx_get_ch_tok(fd);

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
	case '<' : t.type = lx_analyze_define(fd);
		break;
	}

	/* Variable type (string, number, bool etc) */
	if (t.type == NONE) {
		lx_ungetc(c, fd);
		t.type = lx_analyze_var_type(fd);
	}

	return t;
}
