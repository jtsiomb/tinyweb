/* tinyweb - tiny web server library and daemon
 * Author: John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is placed in the public domain. Feel free to use it any
 * way you like. Mentions and retaining this attribution header will be
 * appreciated, but not required.
 */
#ifndef LOGGER_H_
#define LOGGER_H_

int set_log_file(const char *fname);

void logmsg(const char *fname, ...);

#endif	/* LOGGER_H_ */
