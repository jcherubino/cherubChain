#ifndef _SERVER_H
#define _SERVER_H

//includes that any 'user' program will need
#include <unistd.h>
#include <sys/socket.h>

int get_listener(void);
int get_client(int listenfd);

#endif /*_SERVER_H*/
