#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

struct client {
	int s;
	char *rcvbuf;
	int bufsz;
	struct client *next;
};

int start_server(void);
int accept_conn(int lis);
int handle_client(struct client *c);

static int lis;
static int port = 8080;
static struct client *clist;

int main(int argc, char **argv)
{
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
	}
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

int handle_client(struct client *c)
{
	static char buf[2048];
	int rdsz;

	while((rdsz = recv(c->s, buf, sizeof buf, 0)) > 0) {
		if(c->rcvbuf) {
			int newsz = c->bufsz + rdsz;
			char *newbuf = realloc(buf, newsz);
			if(!newbuf) {
				fprintf(stderr, "failed to allocate %d byte buffer\n", newsz);
				/* TODO http error */
				goto drop;
			}

			memcpy(newbuf + c->bufsz, buf, rdsz);

			c->rcvbuf = newbuf;
			c->bufsz = newsz;
		}
	}

	/* TODO: parse header, drop on invalid, determine if the request
	 * is complete and process
	 */

	if(rdsz == -1 && errno != EAGAIN) {
drop:
		close(c->s);
		c->s = -1;
		free(c->rcvbuf);
	}
	return 0;
}
