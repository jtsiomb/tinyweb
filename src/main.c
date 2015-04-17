#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "tinyweb.h"

int parse_args(int argc, char **argv);
void sighandler(int s);


int main(int argc, char **argv)
{
	int *sockets, num_sockets, sockets_arr_size = 0;

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGQUIT, sighandler);

	tw_start();

	for(;;) {
		int i;
		fd_set rdset;

		num_sockets = tw_get_sockets(0);
		if(num_sockets > sockets_arr_size) {
			int newsz = sockets_arr_size ? sockets_arr_size * 2 : 16;
			int *newarr = realloc(sockets, newsz * sizeof *sockets);
			if(!newarr) {
				fprintf(stderr, "failed to allocate sockets array\n");
				tw_stop();
				return 1;
			}
			sockets = newarr;
			sockets_arr_size = newsz;
		}
		tw_get_sockets(sockets);

		FD_ZERO(&rdset);
		for(i=0; i<num_sockets; i++) {
			FD_SET(sockets[i], &rdset);
		}

		while(select(tw_get_maxfd() + 1, &rdset, 0, 0, 0) == -1 && errno == EINTR);

		for(i=0; i<num_sockets; i++) {
			if(FD_ISSET(sockets[i], &rdset)) {
				tw_handle_socket(sockets[i]);
			}
		}
	}

	return 0;	/* unreachable */
}

void sighandler(int s)
{
	if(s == SIGINT || s == SIGTERM || s == SIGQUIT) {
		tw_stop();
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
				{
					int port = atoi(argv[++i]);
					if(!port) {
						fprintf(stderr, "-p must be followed by a valid port number\n");
						return -1;
					}
					tw_set_port(port);
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
