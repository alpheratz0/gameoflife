#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "debug.h"

extern void
die(const char *err) {
	fprintf(stderr, "gameoflife: %s\n", err);
	exit(1);
}

extern void
dief(const char *err, ...) {
	va_list list;
	fputs("gameoflife: ", stderr);
	va_start(list, err);
	vfprintf(stderr, err, list);
	va_end(list);
	fputc('\n', stderr);
	exit(1);
}