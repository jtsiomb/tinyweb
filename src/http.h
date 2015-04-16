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

#define HTTP_HDR_OK			0
#define HTTP_HDR_INVALID	-1
#define HTTP_HDR_NOMEM		-2
#define HTTP_HDR_PARTIAL	-3

int http_parse_header(struct http_req_header *hdr, const char *buf, int bufsz);
void http_print_header(struct http_req_header *hdr);
void http_destroy_header(struct http_req_header *hdr);

const char *http_strmsg(int code);

#endif	/* HTTP_H_ */