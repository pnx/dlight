
#ifndef FILTER_H
#define FILTER_H

int filter_check_syntax(const char *pattern);

int filter_match(const char *pattern, const char *subject);

int filter_match_list(char **patterns, unsigned n, const char *subject);

#endif /* FILTER_H */
