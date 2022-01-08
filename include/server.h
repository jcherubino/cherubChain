#ifndef _SERVER_H
#define _SERVER_H

//includes that any 'user' program will need
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>

struct ServerData {
    struct pollfd* pollfds; //array of pollfd objects for poll call
    int fd_count; //How many pollfd objects are being used
    int listenerfd; //pointer to listener object so we can tell when to 'accept' new conn
};

struct ServerData initialise_server(void);
void deinitialise_server(struct ServerData * pserver_data);
int add_fd_to_server(struct ServerData* server_data, int new_fd);
int get_client(const int listenfd);
int connect_to_node(const char *node_address);
void delete_fd_from_server(struct ServerData* server_data, int fd_index);
int send_buf(int sockfd, const void * buf, size_t len);
int receive_buf(int sockfd, void * buf, size_t len);

#endif /*_SERVER_H*/
