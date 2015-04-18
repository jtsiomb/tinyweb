#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include "logger.h"

static FILE *logfile;

int set_log_file(const char *fname)
{
	FILE *fp;

	if(!(fp = fopen(fname, "w"))) {
		fprintf(stderr, "failed to open logfile: %s: %s\n", fname, strerror(errno));
		return -1;
	}
	setvbuf(fp, 0, _IONBF, 0);
	logfile = fp;
	return 0;
}

void logmsg(const char *fmt, ...)
{
	va_list ap;

	if(!logfile) {
		logfile = stderr;
	}

	va_start(ap, fmt);
	vfprintf(logfile, fmt, ap);
	va_end(ap);
}
