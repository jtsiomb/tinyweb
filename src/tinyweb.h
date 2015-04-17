#ifndef TINYWEB_H_
#define TINYWEB_H_

void tw_set_port(int port);
int tw_set_root(const char *path);
int tw_set_logfile(const char *fname);

int tw_start(void);
int tw_stop(void);

int tw_get_sockets(int *socks);
int tw_get_maxfd(void);
int tw_handle_socket(int s);


#endif	/* TINYWEB_H_ */
