
#include <stdio.h>
#include "version.h"

int cmd_version(int argc, char **argv) {

	printf("%s\n", dlight_version_str);

	return 0;
}
