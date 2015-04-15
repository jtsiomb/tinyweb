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

const char *http_method_str[] = {
	"<unknown>",
	"OPTIONS",
	"GET",
	"HEAD",
	"POST",
	"PUT",
	"DELETE",
	"TRACE",
	"CONNECT",
	0
};


struct http_req_header {
	enum http_method method;
	char *uri;
	int ver_major, ver_minor;	/* http version */
	char **hdrfields;
	int num_hdrfields;
};


int http_parse_header(struct http_req_header *hdr, const char *buf, int bufsz);

const char *http_strmsg(int code);


/* HTTP 1xx message strings */
const char *http_msg1xx[] = {
	"Continue",					/* 100 */
	"Switching Protocols"		/* 101 */
};

/* HTTP 2xx message strings */
const char *http_msg2xx[] = {
	"OK",						/* 200 */
	"Created",					/* 201 */
	"Accepted",					/* 202 */
	"Non-Authoritative Information",	/* 203 */
	"No Content",				/* 204 */
	"Reset Content",			/* 205 */
	"Partial Content"			/* 206 */
};

/* HTTP 3xx message strings */
const char *http_msg3xx[] = {
	"Multiple Choices",			/* 300 */
	"Moved Permanently",		/* 301 */
	"Found",					/* 302 */
	"See Other",				/* 303 */
	"Not Modified",				/* 304 */
	"Use Proxy",				/* 305 */
	"<unknown>",				/* 306 is undefined? */
	"Temporary Redirect"		/* 307 */
};

/* HTTP 4xx error strings */
const char *http_msg4xx[] = {
	"Bad Request",				/* 400 */
	"Unauthorized",				/* 401 */
	"What the Fuck?",			/* 402 */
	"Forbidden",				/* 403 */
	"Not Found",				/* 404 */
	"Method Not Allowed",		/* 405 */
	"Not Acceptable",			/* 406 */
	"Proxy Authentication Required",	/* 407 */
	"Request Time-out",			/* 408 */
	"Conflict",					/* 409 */
	"Gone",						/* 410 */
	"Length Required",			/* 411 */
	"Precondition Failed",		/* 412 */
	"Request Entity Too Large", /* 413 */
	"Request-URI Too Large",	/* 414 */
	"Unsupported Media Type",	/* 415 */
	"Request range not satisfiable", /* 416 */
	"Expectation Failed"		/* 417 */
};

/* HTTP 5xx error strings */
const char *http_msg5xx[] = {
	"Internal Server Error",	/* 500 */
	"Not Implemented",			/* 501 */
	"Bad Gateway",				/* 502 */
	"Service Unavailable",		/* 503 */
	"Gateway Time-out",			/* 504 */
	"HTTP Version not supported"	/* 505 */
};

#endif	/* HTTP_H_ */
