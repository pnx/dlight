/* compile.c
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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include "env.h"
#include "error.h"
#include "cconf.h"
#include "lockfile.h"
#include "filter.h"
#include "version.h"

#define isalias(x) (isalnum(x) || (x) == '-')

#define MAXNAME 1024

static int dest_table_nr;

static struct dest_table {
	char *key;
	char *value;
} *dest_table;

/* index to the default destination */
static unsigned default_dest;

static struct cconf cconf;
static int config_lineno = 1;
static FILE *config_fd;

static int get_next_ch(void) {

	int c = getc(config_fd);
	if (c == '\n')
		config_lineno++;
	return c;
}

static int find_destination(const char *key) {

	int i;
	for(i=0; i < dest_table_nr; i++)
		if (!strcmp(dest_table[i].key, key))
			return i;
	return -1;
}

static char* fetch_destination(char *key) {

	int index = find_destination(key);
	if (index < 0)
		index = default_dest;
	return dest_table[index].value;
}

static void free_destination(struct dest_table *entry) {

	if (entry->key)
		free(entry->key);
	if (entry->value)
		free(entry->value);
}


static void insert_destination(const char *key, const char *value) {


	int index = find_destination(key);

	if (index < 0) {
		dest_table = realloc(dest_table,
			sizeof(struct dest_table) * (dest_table_nr + 1));
		index = dest_table_nr++;
	} else {
		free_destination(&dest_table[index]);
	}

	if (!value)
		value = "";

	dest_table[index].key = strdup(key);
	dest_table[index].value = strdup(value);
}

static char* parse_value() {

	static char value[1024];
	int c, len = 0, space = 0;

	for(;;) {
		c = get_next_ch();
		if (c == EOF || c == '\n')
			break;
		if (isspace(c)) {
			if (len)
				space++;
			continue;
		}
		for(; space; space--)
			value[len++] = ' ';
		value[len++] = c;
	}
	value[len] = '\0';

	return value;
}

static int parse_alias_definition() {

	static char name[MAXNAME];
	const char *value;
	int c, len = 0;

	for(;;) {
		c = get_next_ch();
		if (c == EOF || isspace(c))
			break;
		if (!isalias(c)) {
			error("Invalid character '%c' in alias\n", c);
			return -1;
		}
		if (len >= sizeof(name))
			return -1;
		name[len++] = tolower(c);
	}
	name[len] = '\0';

	value = NULL;
	if (c != '\n') {
		value = parse_value();
		if (!value)
			return -1;
	}

	insert_destination(name, value);

	return 0;
}

static char* parse_alias() {

	static char buf[MAXNAME];
	int c, len = 0, trailing_space = 0;

	for(;;) {
		c = get_next_ch();
		if (c == EOF || c == '\n')
			break;
		if (isspace(c)) {
			if (len)
				trailing_space = 1;
			continue;
		}
		if (!isalias(c)) {
			error("Invalid character '%c' in alias\n", c);
			return NULL;
		}
		if (trailing_space) {
			error("Space not allowed in alias\n");
			return NULL;
		}
		if (len >= sizeof(buf))
			return NULL;
		buf[len++] = tolower(c);
	}
	buf[len] = '\0';
	return buf;
}

static int parse_filter(struct target *target) {

	struct filter filter;
	char pattern[1024];
	char *alias = NULL;
	int c, len = 0;

	for(;;) {
		c = get_next_ch();
		if (c == EOF || isspace(c))
			break;
		if (c == '\\') {
			c = get_next_ch();
			if (c != ' ') {
				ungetc(c, config_fd);
				c = '\\';
			}
		}
		if (len >= sizeof(pattern))
			return -1;
		pattern[len++] = c;
	}
	pattern[len] = '\0';

	if (!pattern[0] || !filter_check_syntax(pattern))
		return -1;

	if (c == ' ' || c == '\t') {
		alias = parse_alias();
		if (!alias)
			return -1;
	}

	filter.pattern = strdup(pattern);
	if (!alias || !alias[0])
		alias = dest_table[default_dest].key;
	filter.dest = strdup(fetch_destination(alias));

	cconf_add_filter(target, &filter);
	return 0;
}

static int parse_target(struct target *target) {

	char src[4096], *alias;
	int c, len = 0;

	for(;;) {
		c = get_next_ch();
		if (c == EOF || isspace(c))
			break;
		if (len >= sizeof(src))
			return -1;
		src[len++] = c;
	}
	src[len] = '\0';

	/* next, get alias */
	alias = parse_alias();
	if (!alias)
		return -1;
	if (!alias[0] && !dest_table_nr) {
		return error("No destination found for target '%s'\n", src);
	}

	target->src = strdup(src);
	default_dest = len ? find_destination(alias) : 0;

	return 0;
}

static int parse_config_file(const char *file) {

	struct target *target = NULL;

	config_fd = fopen(file, "r");
	if (!config_fd) {
		perror(file);
		return -1;
	}

	for(;;) {
		int c = get_next_ch();
		if (c == EOF)
			return 0;
		if (c == ':') {
			if (parse_alias_definition() < 0)
				break;
			continue;
		}
		if (target && c == '\t') {
			if (parse_filter(target) < 0)
				break;
			continue;
		}
		if (isspace(c))
			continue;
		target = cconf_new_target(&cconf);
		ungetc(c, config_fd);
		if (parse_target(target) < 0)
			break;
	}
	error("failed to parse line %i in %s\n", config_lineno, file);

	fclose(config_fd);
	return -1;
}

int main(int argc, char **argv) {

	int lockfd;
	struct lockfile lock = LOCKFILE_INIT;
	char filename[4096];

	if (argc > 1) {
		if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help")) {
			printf("dlight compiler\n");
			printf("%s [--help|-h|--version|-v]\n", argv[0]);
		} else if (!strcmp(argv[1], "-v")
			|| !strcmp(argv[1], "--version")) {
			printf("%s\n", dlight_version_str);
		}
		return 0;
	}

	snprintf(filename, sizeof(filename), "%s/%s",
		env_get_dir(), "config");

	lockfd = hold_lock(&lock, filename);
	if (lockfd < 0)
		return 1;

	if (parse_config_file("./config") < 0)
		goto error;

	if (!cconf_write(lockfd, &cconf) && !commit_lock(&lock)) {
		printf("Compilation complete.\n");
		return 0;
	}
error:
	release_lock(&lock);
	return 1;
}
