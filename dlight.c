
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static struct command commands[] = {
	{ "run", cmd_run },
	{ "compile", cmd_compile },
	{ "read-config", cmd_read_config },
	{ "dlhist", cmd_dlhist },
	{ "filter-check", cmd_filter_check },
	{ "version", cmd_version }
};

struct command* find_command(const char *cmd) {

	int i;
	for(i=0; i < ARRAY_SIZE(commands); i++) {
		struct command *c = commands + i;
		if (!strcmp(cmd, c->cmd))
			return c;
	}
	return NULL;
}

static void dlight_usage() {

	int i;

	printf("usage: dlight [-v|--version] [-h|--help] <command> [<args>]\n");

	printf("\ncommands:\n");
	for(i = 0; i < ARRAY_SIZE(commands); i++) {
		printf("   %s\n", commands[i].cmd);
	}

	exit(1);
}

void handle_options(int *argc, char ***argv) {

	char* opt = (*argv)[0];

	if (*argc < 1 || opt[0] != '-')
		return;

	/* Call the version command. */
	if (!strcmp(opt, "-v") || !strcmp(opt, "--version")) {
		strcpy(opt, "version");
		return;
	}

	if (!strcmp(opt, "-h") || !strcmp(opt, "--help")) {
		dlight_usage();
	}

	printf("Unknown option '%s'\n", opt);
	exit(1);
}

int main(int argc, char **argv) {

	argc--;
	argv++;

	handle_options(&argc, &argv);

	if (argc) {
		struct command *cmd = find_command(argv[0]);
		if (!cmd) {
			printf("Unknown command '%s'\n", argv[0]);
			return 1;
		}

		return cmd->fn(argc - 1, argv + 1);
	} else {
		dlight_usage();
	}

	return 0;
}
