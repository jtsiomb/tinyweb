#ifndef TINYWEB_H_
#define TINYWEB_H_

void tw_set_port(int port);
int tw_set_root(const char *path);
int tw_set_logfile(const char *fname);

int tw_start(void);
int tw_stop(void);

/* tw_get_sockets returns the number of active sockets managed by tinyweb
 * (clients plus the listening socket), and fills in the array sockets
 * passed through the socks pointer, if it's not null.
 *
 * Call it with a null pointer initially to get the number of sockets, then
 * make sure you have enough space in the array and pass it in a second call
 * to fill it.
 */
int tw_get_sockets(int *socks);

/* returns the maximum file descriptor number in the set of sockets managed
 * by the library (useful for calling select).
 */
int tw_get_maxfd(void);

/* call tw_handle_socket to let tinyweb handle incoming traffic to any of the
 * tinyweb managed sockets.
 */
int tw_handle_socket(int s);


#endif	/* TINYWEB_H_ */
