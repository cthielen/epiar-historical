#include "includes.h"
#include "system/debug.h"

void debug_message(char *message) {
#ifndef NDEBUG
	fprintf(stdout, "dbg: %s", message);
#endif
}
