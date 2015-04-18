/* tinyweb - tiny web server library and daemon
 * Author: John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is placed in the public domain. Feel free to use it any
 * way you like. Mentions and retaining this attribution header will be
 * appreciated, but not required.
 */
#ifndef MIME_H_
#define MIME_H_

int add_mime_type(const char *suffix, const char *type);
const char *mime_type(const char *path);

#endif	/* MIME_H_ */
