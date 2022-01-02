#ifndef _SERVER_H
#define _SERVER_H

//includes that any 'user' program will need
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>

struct PollingData {
    struct pollfd* pollfds; //array of pollfd objects for poll call
    int fd_count; //How many pollfd objects are being used
    int listenerfd; //pointer to listener object so we can tell when to 'accept' new conn
};

struct PollingData* initialise_server(void);
void deinitialise_server(struct PollingData ** polling_data);
int add_fd_to_server(struct PollingData* server_data, int new_fd);
void delete_fd_from_server(struct PollingData* server_data, int fd_index);
int32_t send_buf(int fd_index, uint8_t * buf, size_t len);

#endif /*_SERVER_H*/
