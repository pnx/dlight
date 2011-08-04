/* filter.c
 *
 *   Copyright (C) 2011       Henrik Hautakoski <henrik@fiktivkod.org>
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
#include <assert.h>
#include <pcre.h>
#include <stdio.h>
#include <string.h>
#include "error.h"
#include "filter.h"

struct __error_info {
	const char *msg;
	int offset;
};

static inline pcre* compile(const char *pattern, struct __error_info *info) {

	const char *err;
	int eoffset;
	pcre *regex;

	regex = pcre_compile(pattern, 0, &err, &eoffset, NULL);
	if (!regex) {
		if (info) {
			info->msg = error;
			info->offset = eoffset;
		}
		return NULL;
	}

	return regex;
}

static inline int match(pcre *pcre, const char *subject) {

	int ovector[1];

	return pcre_exec(pcre, NULL, subject, strlen(subject), 0, 0,
		ovector, sizeof(ovector));
}

int filter_check_syntax(const char *pattern) {

	struct __error_info info;

	if (!compile(pattern, &info)) {
		error("filter: error in expression '%s': %s\n",
			pattern, info.msg);
		return 0;
	}
	return 1;
}

int filter_match(const char *pattern, const char *subject) {

	pcre *regex;
	int rc;

	if (!pattern || !subject)
		return 0;

	regex = compile(pattern, NULL);
	if (!regex)
		return 0;

	rc = match(regex, subject);

	pcre_free(regex);

	return rc > 0;
}

int filter_match_list(char **patterns, unsigned n, const char *subject) {

	int i;

	for(i=0; i < n; i++) {

		/* return true at the first matching pattern */
		if (filter_match(patterns[i], subject))
			return 1;
	}
	return 0;
}