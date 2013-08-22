/* filter-check.c
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
#include <stdio.h>
#include "error.h"
#include "filter.h"
#include "version.h"

const char *usagestr =
	"dlight-filter-check [ --help | -h | --version | -v ] "
	"| [ <pattern> <subject> ]";

int main(int argc, char **argv) {

	if (argc > 1) {
		if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help")) {
			usage(usagestr);
		} else if (!strcmp(argv[1], "-v")
			|| !strcmp(argv[1], "--version")) {
			printf("Version: %s\n",
				dlight_version_str);
			return 0;
		}
	} else if (argc < 3) {
		usage(usagestr);
	}

	if (!filter_check_syntax(argv[1]))
		return 0;

	if (filter_match(argv[1], argv[2])) {
		puts("match");
	} else {
		puts("nomatch");
	}
	return 0;
}
