
#include <stdio.h>
#include "dlhist.h"

int cmd_dlhist(int argc, char **argv) {

	if (dlhist_open() < 0)
		return 1;

	dlhist_print();

	dlhist_close();

	return 0;
}
