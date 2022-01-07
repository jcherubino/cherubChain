#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "server.h"
#include <stdlib.h>

#define PORT "33333"  //port users connect to

#define MAX_CONN_NUMBER 255 //number of sockfds connections we communicate with
#define TOTAL_CONNS MAX_CONN_NUMBER + 1 //number of sockfds in polling data (+1 for listener fd)

//Internal functions
static int get_listener(void);
static void *get_in_addr(struct sockaddr *sa);

//Create server data struct instance with appropraite set up and information 
//to begin communicating data via sockets. Returns null pointer on failure
struct ServerData *initialise_server(void) {
    struct ServerData * server_data = malloc(sizeof(struct ServerData));

    //TODO: dynamically allocate instead of wasting space with MAX_CONN_NUMBER?
    server_data->pollfds = malloc(sizeof(struct pollfd) * TOTAL_CONNS);

    if (server_data == NULL) {
        fprintf(stderr, "Failed to malloc memory for ServerData struct\n");
        return NULL;
    }

    // Set up and get a listening socket
    server_data->listenerfd = get_listener();

    //return NULL ptr to indicate error
    if (server_data->listenerfd == -1) {
        fprintf(stderr, "error getting listening socket\n");
        free(server_data->pollfds);
        free(server_data);
        return NULL;
    }
    
    // Add the listener to the array
    server_data->fd_count = 1; 
    server_data->pollfds[0].fd = server_data->listenerfd;
    server_data->pollfds[0].events = POLLIN; // Report ready to read on incoming connection

    return server_data;
}

void deinitialise_server(struct ServerData ** server_data) {
    //close all sockets
    for (int i = 0; i < (*server_data)->fd_count; i++) {
        close((*server_data)->pollfds[i].fd);
    }

    free((*server_data)->pollfds);
    free(*server_data);
    *server_data = NULL;
}

//Attempt to add to server.
int add_fd_to_server(struct ServerData* server_data, int new_fd) {
    //N.B. +1 to account for listener
    if (server_data->fd_count == TOTAL_CONNS) {
        fprintf(stderr, "Max connections in server reached\n");
        return -1;
    }
    
    server_data->pollfds[server_data->fd_count].fd = new_fd;
    server_data->pollfds[server_data->fd_count].events = POLLIN; //ready to read
    server_data->fd_count++; 

    return 0;
}

//Remove index from the set
void delete_fd_from_server(struct ServerData* server_data, int fd_index) {
    close(server_data->pollfds[fd_index].fd); //close sockfd
    //Copy end over sockfd to delete with last entry while also updating count
    server_data->pollfds[fd_index] = server_data->pollfds[--server_data->fd_count];
}

/**
 * Get client sockfd to commmunicate on
 * @param listenfd open listening socket to communicate on
 * @return returns connected socket or -1 on failure
 */
int get_client(const int listenfd) {
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size = sizeof(their_addr);

    int new_fd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);

    if (new_fd == -1) {
        perror("accept");
        close(new_fd);
        return -1;
    }
        
    //report client information
    
    char remote_ip[INET6_ADDRSTRLEN];

    printf("Server: new connection from %s on socket %d\n",
            inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr*)&their_addr), remote_ip, INET6_ADDRSTRLEN),
                            new_fd);
    return new_fd;
}

/**
 * Connect to node
 * @param node_address IP of node to connect to
 * @return sockfd of connected node or -1 on failure
 */
int connect_to_node(const char *node_address) {
    struct addrinfo hints, *node_info, *p;
    int ret, sockfd;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; //IPV4 or IPV6
    hints.ai_socktype = SOCK_STREAM; //TCP stream

    if ((ret = getaddrinfo(node_address, PORT, &hints, &node_info)) != 0) {
        fprintf(stderr, "server: %s\n", gai_strerror(ret));
        return -1;
    }	

    //loop through all the results and connect to the first we can
	for(p = node_info; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("Server connect_to_node: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("Server connect_to_node: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "Server connect_to_node: failed to connect\n");
		return -1;
	}

    //report client information
    char remote_ip[INET6_ADDRSTRLEN];

    printf("Server: connected to node %s\n",
            inet_ntop(p->ai_family,
                get_in_addr((struct sockaddr*)p->ai_addr), remote_ip, INET6_ADDRSTRLEN));

	freeaddrinfo(node_info); // all done with this structure
    return sockfd;
}

/**
 * Get IP address from given sockaddr struct. Ip version agnostic
 * @param populated sockaddr struct
 * @return void
 */
static void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//Get 'listening' sockfd 
//Return -1 on error
//Code mostly from from Beej's Guide to Network Programming
static int get_listener(void) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            return -1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        //Successfully bound so we may exit
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return -1;
    }

    if (listen(sockfd, MAX_CONN_NUMBER) == -1) {
        perror("listen");
        close(sockfd);
        return -1;
    }

    //if we have reached this point. Then we may return our open sockfd to
    //accept new cons from
    return sockfd;
}


//send buffer to given connected sockfd
//return -1 on failure otherwise return number of bytes written
//TODO: Update with new struct int fd_index
int32_t send_buf(int sockfd, uint8_t * buf, size_t len) {
    size_t sent = 0;
    int n; 
    while (sent < len) {
        n = send(sockfd, buf+sent, len-sent, 0); 
        if (n == -1) break;
        sent += n;
    }

    //-1 on failure otherwise number sent
    return n == -1 ? -1: sent;
}

