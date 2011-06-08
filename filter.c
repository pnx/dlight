
#include <assert.h>
#include <pcre.h>
#include <stdio.h>
#include <string.h>
#include "filter.h"

static inline pcre* compile(const char *pattern) {

	const char *error;
	int eoffset;
	pcre *regex;

	regex = pcre_compile(pattern, 0, &error, &eoffset, NULL);
	if (!regex) {
		fprintf(stderr, "Error compiling expression\n");
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

	const char *error;
	int eoffset;
	pcre *regex;

	regex = pcre_compile(pattern, 0, &error, &eoffset, NULL);
	if (!regex) {
		fprintf(stderr, "filter: error in expression '%s': %s\n",
			pattern, error);
		return 0;
	}
	return 1;
}

int filter_match(const char *pattern, const char *subject) {

	pcre *regex;
	int rc;

	if (!pattern || !subject)
		return 0;

	regex = compile(pattern);
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