#ifndef _SERVER_H
#define _SERVER_H

#include <stdlib.h>
#include <unistd.h>

int get_listener(void);
int get_client(int listenfd);

#endif /*_SERVER_H*/
