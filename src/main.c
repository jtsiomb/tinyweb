#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "http.h"

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

int start_server(void);
int accept_conn(int lis);
void close_conn(struct client *c);
int handle_client(struct client *c);
void do_get_head(struct client *c);
void respond_error(struct client *c, int errcode);
void sighandler(int s);
int parse_args(int argc, char **argv);

static int lis;
static int port = 8080;
static struct client *clist;

int main(int argc, char **argv)
{
	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGQUIT, sighandler);

	if((lis = start_server()) == -1) {
		return 1;
	}

	for(;;) {
		struct client *c, dummy;
		int maxfd = lis;
		fd_set rdset;

		FD_ZERO(&rdset);
		FD_SET(lis, &rdset);

		c = clist;
		while(c) {
			if(c->s > maxfd) {
				maxfd = c->s;
			}
			FD_SET(c->s, &rdset);
			c = c->next;
		}

		while(select(maxfd + 1, &rdset, 0, 0, 0) == -1 && errno == EINTR);

		c = clist;
		while(c) {
			if(FD_ISSET(c->s, &rdset)) {
				handle_client(c);
			}
			c = c->next;
		}

		if(FD_ISSET(lis, &rdset)) {
			accept_conn(lis);
		}

		dummy.next = clist;
		c = &dummy;

		while(c->next) {
			struct client *n = c->next;

			if(n->s == -1) {
				/* marked for removal */
				c->next = n->next;
				free(n);
			} else {
				c = c->next;
			}
		}
		clist = dummy.next;
	}

	return 0;	/* unreachable */
}

int start_server(void)
{
	int s;
	struct sockaddr_in sa;

	if((s = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		perror("failed to create listening socket");
		return -1;
	}
	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);

	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(port);

	if(bind(s, (struct sockaddr*)&sa, sizeof sa) == -1) {
		fprintf(stderr, "failed to bind socket to port %d: %s\n", port, strerror(errno));
		return -1;
	}
	listen(s, 16);

	return s;
}

int accept_conn(int lis)
{
	int s;
	struct client *c;
	struct sockaddr_in addr;
	socklen_t addr_sz = sizeof addr;

	if((s = accept(lis, (struct sockaddr*)&addr, &addr_sz)) == -1) {
		perror("failed to accept incoming connection");
		return -1;
	}
	fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK);

	if(!(c = malloc(sizeof *c))) {
		perror("failed to allocate memory while accepting connection");
		return -1;
	}
	c->s = s;
	c->rcvbuf = 0;
	c->bufsz = 0;
	c->next = clist;
	clist = c;
	return 0;
}

void close_conn(struct client *c)
{
	close(c->s);
	c->s = -1;	/* mark it for removal */
	free(c->rcvbuf);
}

int handle_client(struct client *c)
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
			fprintf(stderr, "failed to allocate %d byte buffer\n", newsz);
			respond_error(c, 503);
			return -1;
		}

		memcpy(newbuf + c->bufsz, buf, rdsz);
		newbuf[newsz] = 0;

		c->rcvbuf = newbuf;
		c->bufsz = newsz;
	}

	if((status = http_parse_header(&hdr, c->rcvbuf, c->bufsz)) != HTTP_HDR_OK) {
		http_print_header(&hdr);
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
	http_print_header(&hdr);

	/* we only support GET and HEAD at this point, so freak out on anything else */
	switch(hdr.method) {
	case HTTP_GET:
	case HTTP_HEAD:
		do_get_head(c);
		break;

	default:
		respond_error(c, 501);
		return -1;
	}

	close_conn(c);
	return 0;
}

void do_get_head(struct client *c)
{
}

void respond_error(struct client *c, int errcode)
{
	char buf[512];

	sprintf(buf, HTTP_VER_STR " %d %s\r\n\r\n", errcode, http_strmsg(errcode));

	send(c->s, buf, strlen(buf), 0);
	close_conn(c);
}

void sighandler(int s)
{
	if(s == SIGINT || s == SIGTERM || s == SIGQUIT) {
		close(lis);
		while(clist) {
			struct client *c = clist;
			clist = clist->next;
			close_conn(c);
			free(c);
		}
		clist = 0;

		printf("bye!\n");
		exit(0);
	}
}


static void print_help(const char *argv0)
{
	printf("Usage: %s [options]\n", argv0);
	printf("Options:\n");
	printf(" -p <port>  set the TCP/IP port number to use\n");
	printf(" -h         print usage help and exit\n");
}

int parse_args(int argc, char **argv)
{
	int i;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-' && argv[i][2] == 0) {
			switch(argv[i][1]) {
			case 'p':
				if((port = atoi(argv[++i])) == 0) {
					fprintf(stderr, "-p must be followed by a valid port number\n");
					return -1;
				}
				break;

			case 'h':
				print_help(argv[0]);
				exit(0);

			default:
				fprintf(stderr, "unrecognized option: %s\n", argv[i]);
				return -1;
			}
		} else {
			fprintf(stderr, "unexpected argument: %s\n", argv[i]);
			return -1;
		}
	}
	return 0;
}
