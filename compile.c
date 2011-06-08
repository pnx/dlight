
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
#include "cconf.h"

#define error(...) fprintf(stderr, "error: " __VA_ARGS__)

#define isalias(x) (isalnum(x) || (x) == '-')

#define MAXNAME 1024

static int dest_table_nr;

static struct dest_table {
	char *key;
	char *value;
} *dest_table;

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
		index = 0;
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


static int parse_alias() {

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

static int parse_filter(struct target *target) {

	char *value = parse_value();
	if (!value)
		return -1;
	cconf_add_filter(target, strdup(value));
	return 0;
}

static int parse_target(struct target *target) {

	char src[4096], alias[4096];
	int c, len = 0, trailing_space = 0;

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
	len = 0;
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
			return -1;
		}
		if (trailing_space) {
			error("Space not allowed in alias\n");
			return -1;
		}
		if (len >= sizeof(alias))
			return -1;
		alias[len++] = tolower(c);
	}
	alias[len] = '\0';

	if (!len && !dest_table_nr) {
		error("No destination found for target '%s'\n", src);
		return -1;
	}

	target->src = strdup(src);
	target->dest = strdup(len ? fetch_destination(alias) :
		dest_table[0].value);

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
			if (parse_alias() < 0)
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

static int commit_lock(const char *file) {

	char target[4096];
	int len;

	len = strlen(file) - 5; /* .lock */

	memcpy(target, file, len);
	target[len] = '\0';

	return rename(file, target);
}

int main(int argc, char **argv) {

	int lockfd;
	char lockfile[4096];

	snprintf(lockfile, sizeof(lockfile), "%s/%s",
		env_get_dir(), "config.lock");

	/* Remove lockfile if forced */
	if (argc > 1 && !strcmp(argv[1], "-f"))
		unlink(lockfile);

	lockfd = open(lockfile, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (lockfd < 0) {
		if (errno == EEXIST) {
			error("config is locked\n");
		} else {
			perror("unable to create new configfile");
		}
		return 1;
	}

	if (parse_config_file("./config") < 0)
		goto error;

	if (!cconf_write(lockfd, &cconf) &&
		!commit_lock(lockfile))
		return 0;
error:
	unlink(lockfile);
	return 1;
}
