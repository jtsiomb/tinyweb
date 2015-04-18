#ifndef LOGGER_H_
#define LOGGER_H_

int set_log_file(const char *fname);

void logmsg(const char *fname, ...);

#endif	/* LOGGER_H_ */
