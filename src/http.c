#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <alloca.h>
#include "http.h"


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


static enum http_method parse_method(const char *s);


int http_parse_header(struct http_req_header *hdr, const char *buf, int bufsz)
{
	int i, nlines = 0;
	char *rqline = 0;
	char *method, *uri, *version, *ptr;
	const char *startln, *endln;

	memset(hdr, 0, sizeof *hdr);

	for(i=1; i<bufsz; i++) {
		if(buf[i] == '\n' && buf[i - 1] == '\r') {
			if(!rqline) {
				rqline = alloca(i);
				memcpy(rqline, buf, i - 1);
				rqline[i - 1] = 0;
			}
			++nlines;

			if(i > 4 && buf[i - 2] == '\n' && buf[i - 3] == '\r') {
				hdr->body_offset = i + 1;
				break;
			}
		}
	}

	if(!rqline) {
		return HTTP_HDR_PARTIAL;
	}

	ptr = rqline;
	while(*ptr && isspace(*ptr)) ++ptr;
	method = ptr;

	/* parse the request line */
	while(*ptr && !isspace(*ptr)) ++ptr;
	while(*ptr && isspace(*ptr)) *ptr++ = 0;

	uri = ptr;
	while(*ptr && !isspace(*ptr)) ++ptr;
	while(*ptr && isspace(*ptr)) *ptr++ = 0;

	version = ptr;
	while(*ptr && !isspace(*ptr)) ++ptr;
	while(*ptr && isspace(*ptr)) *ptr++ = 0;

	hdr->method = parse_method(method);
	hdr->uri = strdup(uri);
	if(sscanf(version, "HTTP/%d.%d", &hdr->ver_major, &hdr->ver_minor) != 2) {
		fprintf(stderr, "warning: failed to parse HTTP version \"%s\"\n", version);
		hdr->ver_major = 1;
		hdr->ver_minor = 1;
	}

	if(!(hdr->hdrfields = malloc(nlines * sizeof *hdr->hdrfields))) {
		perror("failed to allocate memory for the header fields");
		return HTTP_HDR_NOMEM;
	}
	hdr->num_hdrfields = 0;

	startln = buf;
	endln = buf;
	for(i=1; i<hdr->body_offset; i++) {
		if(buf[i] == '\n' && buf[i - 1] == '\r') {
			int linesz;
			endln = buf + i - 1;
			linesz = endln - startln - 1;

			if(startln > buf) {	/* skip first line */
				int idx = hdr->num_hdrfields++;
				hdr->hdrfields[idx] = malloc(linesz + 1);
				memcpy(hdr->hdrfields[idx], startln, linesz);
				hdr->hdrfields[idx][linesz] = 0;
			}
			startln = endln = buf + i + 1;
		}
	}

	return HTTP_HDR_OK;
}

void http_print_header(struct http_req_header *hdr)
{
	int i;

	printf("HTTP request header\n");
	printf(" method: %s\n", http_method_str[hdr->method]);
	printf(" uri: %s\n", hdr->uri);
	printf(" version: %d.%d\n", hdr->ver_major, hdr->ver_minor);
	printf(" fields (%d):\n", hdr->num_hdrfields);

	for(i=0; i<hdr->num_hdrfields; i++) {
		printf("   %s\n", hdr->hdrfields[i]);
	}
	putchar('\n');
}

void http_destroy_header(struct http_req_header *hdr)
{
	int i;

	if(hdr->hdrfields) {
		for(i=0; i<hdr->num_hdrfields; i++) {
			free(hdr->hdrfields[i]);
		}
		free(hdr->hdrfields);
	}
	free(hdr->uri);
}

const char *http_strmsg(int code)
{
	static const char **msgxxx[] = {
		0, http_msg1xx, http_msg2xx, http_msg3xx, http_msg4xx, http_msg5xx
	};
	static int msgcount[] = {
		0,
		sizeof http_msg1xx / sizeof *http_msg1xx,
		sizeof http_msg2xx / sizeof *http_msg2xx,
		sizeof http_msg3xx / sizeof *http_msg3xx,
		sizeof http_msg4xx / sizeof *http_msg4xx,
		sizeof http_msg5xx / sizeof *http_msg5xx
	};

	int type = code / 100;
	int idx = code % 100;

	if(type < 1 || type >= sizeof msgxxx / sizeof *msgxxx) {
		return "Invalid HTTP Status";
	}

	if(idx < 0 || idx >= msgcount[type]) {
		return "Unknown HTTP Status";
	}

	return msgxxx[type][idx];
}

static enum http_method parse_method(const char *s)
{
	int i;
	for(i=0; http_method_str[i]; i++) {
		if(strcmp(s, http_method_str[i]) == 0) {
			return (enum http_method)i;
		}
	}
	return HTTP_UNKNOWN;
}
