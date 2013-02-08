
#include <sys/stat.h>
#include "utils.h"

int file_cmp(const char *a, const char *b) {

	struct stat sa, sb;

	if (stat(a, &sa) < 0 || stat(b, &sb) < 0)
		return 0;
	return  sa.st_dev == sb.st_dev &&
		sa.st_ino == sb.st_ino;
}
