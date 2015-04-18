#ifndef MIME_H_
#define MIME_H_

int add_mime_type(const char *suffix, const char *type);
const char *mime_type(const char *path);

#endif	/* MIME_H_ */
