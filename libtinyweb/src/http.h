/* tinyweb - tiny web server library and daemon
 * Author: John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is placed in the public domain. Feel free to use it any
 * way you like. Mentions and retaining this attribution header will be
 * appreciated, but not required.
 */
#ifndef HTTP_H_
#define HTTP_H_

enum http_method {
	HTTP_UNKNOWN,
	HTTP_OPTIONS,
	HTTP_GET,
	HTTP_HEAD,
	HTTP_POST,
	HTTP_PUT,
	HTTP_DELETE,
	HTTP_TRACE,
	HTTP_CONNECT,

	NUM_HTTP_METHODS
};

struct http_req_header {
	enum http_method method;
	char *uri;
	int ver_major, ver_minor;	/* http version */
	char **hdrfields;
	int num_hdrfields;
	int body_offset;
};

struct http_resp_header {
	int status;
	int ver_major, ver_minor;
	char **fields;
	int num_fields;
};

#define HTTP_HDR_OK			0
#define HTTP_HDR_INVALID	-1
#define HTTP_HDR_NOMEM		-2
#define HTTP_HDR_PARTIAL	-3

int http_parse_request(struct http_req_header *hdr, const char *buf, int bufsz);
void http_log_request(struct http_req_header *hdr);
void http_destroy_request(struct http_req_header *hdr);

int http_init_resp(struct http_resp_header *resp);
int http_add_resp_field(struct http_resp_header *resp, const char *fmt, ...);
void http_destroy_resp(struct http_resp_header *resp);
int http_serialize_resp(struct http_resp_header *resp, char *buf);

const char *http_strmsg(int code);

#endif	/* HTTP_H_ */
