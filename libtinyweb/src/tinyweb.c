/* tinyweb - tiny web server library and daemon
 * Author: John Tsiombikas <nuclear@member.fsf.org>
 *
 * This program is placed in the public domain. Feel free to use it any
 * way you like. Mentions and retaining this attribution header will be
 * appreciated, but not required.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "tinyweb.h"
#include "http.h"
#include "mime.h"
#include "logger.h"

/* HTTP version */
#define HTTP_VER_MAJOR	1
#define HTTP_VER_MINOR	1
#define HTTP_VER_STR	"1.1"

/* maximum request length: 64mb */
#define MAX_REQ_LENGTH	(65536 * 1024)

struct client {
	int s;
	char *rcvbuf;
	int bufsz;
	struct client *next;
};

static int accept_conn(int lis);
static void close_conn(struct client *c);
static int handle_client(struct client *c);
static int do_get(struct client *c, const char *uri, int with_body);
static void respond_error(struct client *c, int errcode);

static int lis = -1;
static int maxfd;
static int port = 8080;
static struct client *clist;
static int num_clients;

static const char *indexfiles[] = {
	"index.cgi",
	"index.html",
	"index.htm",
	0
};

void tw_set_port(int p)
{
	port = p;
}

int tw_set_root(const char *path)
{
	return chdir(path);
}

int tw_set_logfile(const char *fname)
{
	return set_log_file(fname);
}

int tw_start(void)
{
	int s;
	struct sockaddr_in sa;

	logmsg("starting server ...\n");

	if(lis != -1) {
		logmsg("can't start tinyweb server: already running!\n");
		return -1;
	}

	if((s = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		logmsg("failed to create listening socket: %s\n", strerror(errno));
		return -1;
	}
	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);

	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(port);

	if(bind(s, (struct sockaddr*)&sa, sizeof sa) == -1) {
		logmsg("failed to bind socket to port %d: %s\n", port, strerror(errno));
		return -1;
	}
	listen(s, 16);

	lis = s;
	return s;
}

int tw_stop(void)
{
	if(lis == -1) {
		return -1;
	}

	logmsg("stopping server...\n");

	close(lis);
	lis = -1;

	while(clist) {
		struct client *c = clist;
		clist = clist->next;
		close_conn(c);
		free(c);
	}
	clist = 0;

	return 0;
}

int tw_get_sockets(int *socks)
{
	struct client *c, dummy;

	/* first cleanup the clients marked for removal */
	dummy.next = clist;
	c = &dummy;

	while(c->next) {
		struct client *n = c->next;

		if(n->s == -1) {
			/* marked for removal */
			c->next = n->next;
			free(n);
			--num_clients;
		} else {
			c = c->next;
		}
	}
	clist = dummy.next;


	if(!socks) {
		/* just return the count */
		return num_clients + 1;	/* +1 for the listening socket */
	}

	/* go through the client list and populate the array */
	maxfd = lis;
	*socks++ = lis;

	c = clist;
	while(c) {
		*socks++ = c->s;
		if(c->s > maxfd) {
			maxfd = c->s;
		}
		c = c->next;
	}
	return num_clients + 1;	/* +1 for the listening socket */
}

int tw_get_maxfd(void)
{
	return maxfd;
}

int tw_handle_socket(int s)
{
	struct client *c;

	if(s == lis) {
		return accept_conn(s);
	}

	/* find which client corresponds to this socket */
	c = clist;
	while(c) {
		if(c->s == s) {
			return handle_client(c);
		}
		c = c->next;
	}

	logmsg("socket %d doesn't correspond to any client\n");
	return -1;
}

static int accept_conn(int lis)
{
	int s;
	struct client *c;
	struct sockaddr_in addr;
	socklen_t addr_sz = sizeof addr;

	if((s = accept(lis, (struct sockaddr*)&addr, &addr_sz)) == -1) {
		logmsg("failed to accept incoming connection: %s\n", strerror(errno));
		return -1;
	}
	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);

	if(!(c = malloc(sizeof *c))) {
		logmsg("failed to allocate memory while accepting connection: %s\n", strerror(errno));
		return -1;
	}
	c->s = s;
	c->rcvbuf = 0;
	c->bufsz = 0;
	c->next = clist;
	clist = c;
	++num_clients;
	return 0;
}

static void close_conn(struct client *c)
{
	close(c->s);
	c->s = -1;	/* mark it for removal */
	free(c->rcvbuf);
	c->rcvbuf = 0;
}

static int handle_client(struct client *c)
{
	struct http_req_header hdr;
	static char buf[2048];
	int rdsz, status;

	while((rdsz = recv(c->s, buf, sizeof buf, 0)) > 0) {
		char *newbuf;
		int newsz = c->bufsz + rdsz;
		if(newsz > MAX_REQ_LENGTH) {
			respond_error(c, 413);
			return -1;
		}

		if(!(newbuf = realloc(c->rcvbuf, newsz + 1))) {
			logmsg("failed to allocate %d byte buffer\n", newsz);
			respond_error(c, 503);
			return -1;
		}

		memcpy(newbuf + c->bufsz, buf, rdsz);
		newbuf[newsz] = 0;

		c->rcvbuf = newbuf;
		c->bufsz = newsz;
	}

	if((status = http_parse_request(&hdr, c->rcvbuf, c->bufsz)) != HTTP_HDR_OK) {
		http_log_request(&hdr);
		switch(status) {
		case HTTP_HDR_INVALID:
			respond_error(c, 400);
			return -1;

		case HTTP_HDR_NOMEM:
			respond_error(c, 503);
			return -1;

		case HTTP_HDR_PARTIAL:
			return 0;	/* partial header, continue reading */
		}
	}
	http_log_request(&hdr);

	/* we only support GET and HEAD at this point, so freak out on anything else */
	switch(hdr.method) {
	case HTTP_GET:
		if(do_get(c, hdr.uri, 1) == -1) {
			return -1;
		}
		break;

	case HTTP_HEAD:
		if(do_get(c, hdr.uri, 0) == -1) {
			return -1;
		}
		break;

	default:
		respond_error(c, 501);
		return -1;
	}

	close_conn(c);
	return 0;
}

static int do_get(struct client *c, const char *uri, int with_body)
{
	const char *ptr;
	struct http_resp_header resp;

	if((ptr = strstr(uri, "://"))) {
		uri = ptr + 3;
	}

	/* skip the host part and the first slash if it exists */
	if((ptr = strchr(uri, '/'))) {
		uri = ptr + 1;
	}

	if(*uri) {
		struct stat st;
		char *path = 0;
		char *rsphdr;
		const char *type;
		int fd, rspsize;

		if(stat(uri, &st) == -1) {
			respond_error(c, 404);
			return -1;
		}

		if(S_ISDIR(st.st_mode)) {
			int i;
			path = alloca(strlen(uri) + 64);

			for(i=0; indexfiles[i]; i++) {
				sprintf(path, "%s/%s", uri, indexfiles[i]);
				if(stat(path, &st) == 0 && !S_ISDIR(st.st_mode)) {
					break;
				}
			}

			if(indexfiles[i] == 0) {
				respond_error(c, 404);
				return -1;
			}
		} else {
			path = (char*)uri;
		}

		if((fd = open(path, O_RDONLY)) == -1) {
			respond_error(c, 403);
			return -1;
		}

		/* construct response header */
		http_init_resp(&resp);
		http_add_resp_field(&resp, "Content-Length: %d", st.st_size);
		if((type = mime_type(path))) {
			http_add_resp_field(&resp, "Content-Type: %s", type);
		}

		rspsize = http_serialize_resp(&resp, 0);
		rsphdr = alloca(rspsize);
		http_serialize_resp(&resp, rsphdr);

		if(with_body) {
			int cont_left = st.st_size;
			char *cont = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if(cont == (void*)-1) {
				respond_error(c, 503);
				close(fd);
				return -1;
			}
			ptr = cont;

			send(c->s, rsphdr, rspsize, 0);
			while(cont_left > 0) {
				int sz = cont_left < 4096 ? cont_left : 4096;
				send(c->s, ptr, sz, 0);
				ptr += sz;
				cont_left -= sz;
			}

			munmap(cont, st.st_size);
		} else {
			send(c->s, rsphdr, rspsize, 0);
		}

		close(fd);
	}
	return 0;
}

static void respond_error(struct client *c, int errcode)
{
	char buf[512];

	sprintf(buf, "HTTP/" HTTP_VER_STR " %d %s\r\n\r\n", errcode, http_strmsg(errcode));

	send(c->s, buf, strlen(buf), 0);
	close_conn(c);
}

