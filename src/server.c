/**
 * Implementation of tools to initialise and communicate
 * data via sockets. Provides an interface to the `poll` syscall
 * to handle multiple connections to a single device
 * Written by Josh Cherubino (josh.cherubino@gmail.com)
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include "server.h"

#define MAX_CONN_NUMBER 255 //number of sockfds connections we communicate with
#define SEND_TIMEOUT_S 1 //number of seconds until timeout on sends
#define RECV_TIMEOUT_S SEND_TIMEOUT_S //number of seconds until timeout on recvs

#define TOTAL_CONNS MAX_CONN_NUMBER + 1 //number of sockfds in polling data (+1 for listener fd)

//Internal functions
static int get_listener(const char * servname);
static void *get_in_addr(struct sockaddr *sa);

/**
 * Create server data struct. Populate fields as required to begin communicating
 * @param servname port number or service name 
 * @return populated server data struct. pollfds field will be NULL on failure
 */
struct ServerData initialise_server(const char * servname) {
    struct ServerData server_data;

    server_data.pollfds = malloc(sizeof(struct pollfd));

    if (server_data.pollfds == NULL) {
        fprintf(stderr, "Server: Failed to malloc memory for pollfds\n");
        return server_data;
    }

    server_data.fd_size = 1;
    
    // Set up and get a listening socket
    server_data.listenerfd = get_listener(servname);

    if (server_data.listenerfd == -1) {
        fprintf(stderr, "Server: error getting listening socket\n");
        free(server_data.pollfds);
        server_data.pollfds = NULL;
        server_data.fd_size = 0;
        return server_data;
    }
    
    // Add the listener to the array
    server_data.fd_count = 1; 
    server_data.pollfds[0].fd = server_data.listenerfd;
    server_data.pollfds[0].events = POLLIN; // Report ready to read on incoming connection

    return server_data;
}

/**
 * Deinitialise server. Close all socket connections and free allocated buffers
 * @param pserver_data pointer to server data object to deinit
 * @return void
 */
void deinitialise_server(struct ServerData * pserver_data) {
    //close all sockets
    for (int i = 0; i < pserver_data->fd_count; i++) {
        close(pserver_data->pollfds[i].fd);
    }

    free(pserver_data->pollfds);

    pserver_data->pollfds = NULL;
    pserver_data->fd_count = 0;
    pserver_data->listenerfd = -1;
}

/**
 * Add open socket fd to server.
 * @param pserver_data server data struct to add sock to
 * @param new_fd connected fd to add
 * @return 0 on success or -1 on failure
 */
int add_fd_to_server(struct ServerData* pserver_data, int new_fd) {
    if (pserver_data->fd_count == pserver_data->fd_size) {
        //double allocated space
        pserver_data->fd_size *= 2;
        pserver_data->pollfds = realloc(pserver_data->pollfds, 
                sizeof(struct pollfd)*pserver_data->fd_count);
        
        //realloc error
        if (pserver_data->pollfds == NULL) {
            perror("Server: realloc");
            return -1;
        }
    }
    
    pserver_data->pollfds[pserver_data->fd_count].fd = new_fd;
    pserver_data->pollfds[pserver_data->fd_count].events = POLLIN; //ready to read
    pserver_data->fd_count++; 

    return 0;
}

/**
 * Remove socket from list using index in pollfds array.
 * @param pserver_data server data struct to remove sock from
 * @return void
 */
void delete_fd_from_server(struct ServerData* pserver_data, int fd_index) {
    //Invalid index
    if (fd_index >= pserver_data->fd_count) {
        fprintf(stderr, "Server: invalid index requested to delete");
        return;
    }
    close(pserver_data->pollfds[fd_index].fd); //close sockfd
    //Copy end over sockfd to delete with last entry while also updating count
    pserver_data->pollfds[fd_index] = pserver_data->pollfds[--pserver_data->fd_count];
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
        perror("Server: accept");
        close(new_fd);
        return -1;
    }
        
    //Set send and receive timeouts
    struct timeval tv;
    tv.tv_sec = SEND_TIMEOUT_S;
    tv.tv_usec = 0;
    
    if (setsockopt(new_fd, SOL_SOCKET, SO_SNDTIMEO, (void *)&tv, sizeof(tv)) != 0) {
        fprintf(stderr, "Server: Failed to set send timeout\n");
        close(new_fd);
        return -1; 
    }
    tv.tv_sec = RECV_TIMEOUT_S;
    if (setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&tv, sizeof(tv)) != 0) {
        fprintf(stderr, "Server: Failed to set receive timeout\n");
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
 * @param servname port number or service name 
 * @return sockfd of connected node or -1 on failure
 */
int connect_to_node(const char *node_address, const char *servname) {
    struct addrinfo hints, *node_info, *p;
    int ret, sockfd;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; //IPV4 or IPV6
    hints.ai_socktype = SOCK_STREAM; //TCP stream

    if ((ret = getaddrinfo(node_address, servname, &hints, &node_info)) != 0) {
        fprintf(stderr, "Server: getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
    }	

    //loop through all the results and connect to the first we can
	for(p = node_info; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("Server: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("Server: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "Server: failed to connect\n");
		return -1;
	}

    //report client information
    char remote_ip[INET6_ADDRSTRLEN];

    printf("Server: connected to node %s on service %s\n",
            inet_ntop(p->ai_family,
                get_in_addr((struct sockaddr*)p->ai_addr), remote_ip, INET6_ADDRSTRLEN),
            servname);

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

//Code mostly from from Beej's Guide to Network Programming
/**
 * Get sock fd to listen for new connections
 * @param servname port number or service name 
 * @return connected listener sockfd or -1 on error
 */
static int get_listener(const char * servname) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, servname, &hints, &servinfo)) != 0) {
        fprintf(stderr, "Server: getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("Server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("Server: setsockopt");
            return -1;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server: bind");
            continue;
        }

        //Successfully bound so we may exit
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "Server: failed to bind\n");
        return -1;
    }

    if (listen(sockfd, MAX_CONN_NUMBER) == -1) {
        perror("Server: listen");
        close(sockfd);
        return -1;
    }

    //if we have reached this point. Then we may return our open sockfd to
    //accept new cons from
    return sockfd;
}

/**
 * Send buffer to given socket. Will continue to `send` until appropriate amount of 
 * bytes transmitted or error occurs.
 * @param sockfd connected socket to send buffer to
 * @param buf pointer to databuffer to transmit
 * @param len number of bytes to send
 * @return number of bytes sent or -1 on failure
 */
int send_buf(int sockfd, const void * buf, size_t len) {
    size_t sent = 0;
    int n; 
    while (sent < len) {
        n = send(sockfd, buf+sent, len-sent, 0); 
        if (n == -1) {
            perror("Server: send");
            return -1;
        }
        sent += n;
    }

    return sent;
}


/**
 * Receive buffer from a specified socket.
 * @param sockfd socket to receive from
 * @param buf buffer to populate
 * @param len number of bytes to receive
 * @return number of bytes received, -1 on error and 0 on socket close
 */
int receive_buf(int sockfd, void * buf, size_t len) {
    size_t recvd = 0;
    ssize_t n; 
    while (recvd < len) {
        n = recv(sockfd, buf+recvd, len-recvd, 0); 
        //error or socket closed
        if (n == -1) {
            perror("Server: recv");
            return -1;
        }
        else if( n == 0) {
            fprintf(stderr, "Server: sockfd %d closed while receiving\n", sockfd);
            return 0;
        }
        recvd += n;
    }

    //return number of bytes recvd
    return recvd;
}

