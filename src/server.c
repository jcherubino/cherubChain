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
void *get_in_addr(struct sockaddr *sa);

//Create server data struct instance with appropraite set up and information 
//to begin communicating data via sockets. Returns null pointer on failure
struct PollingData *initialise_server(void) {
    struct PollingData * polling_data = malloc(sizeof(struct PollingData));

    //TODO: dynamically allocate instead of wasting space with MAX_CONN_NUMBER?
    polling_data->pollfds = malloc(sizeof(struct pollfd) * TOTAL_CONNS);

    if (polling_data == NULL) {
        fprintf(stderr, "Failed to malloc memory for PollingData struct\n");
        return NULL;
    }

    // Set up and get a listening socket
    polling_data->listenerfd = get_listener();

    //return NULL ptr to indicate error
    if (polling_data->listenerfd == -1) {
        fprintf(stderr, "error getting listening socket\n");
        free(polling_data->pollfds);
        free(polling_data);
        return NULL;
    }
    
    // Add the listener to the array
    polling_data->fd_count = 1; 
    polling_data->pollfds[0].fd = polling_data->listenerfd;
    polling_data->pollfds[0].events = POLLIN; // Report ready to read on incoming connection

    return polling_data;
}

void deinitialise_server(struct PollingData ** polling_data) {
    //close all sockets
    for (int i = 0; i < (*polling_data)->fd_count; i++) {
        close((*polling_data)->pollfds[i].fd);
    }

    free((*polling_data)->pollfds);
    free(*polling_data);
    *polling_data = NULL;
}

//Attempt to add to server.
int add_fd_to_server(struct PollingData* polling_data, int new_fd) {
    //N.B. +1 to account for listener
    if (polling_data->fd_count == TOTAL_CONNS) {
        fprintf(stderr, "Max connections in server reached\n");
        return -1;
    }
    
    polling_data->pollfds[polling_data->fd_count].fd = new_fd;
    polling_data->pollfds[polling_data->fd_count].events = POLLIN; //ready to read
    polling_data->fd_count++; 

    return 0;
}

//Remove index from the set
void delete_fd_from_server(struct PollingData* polling_data, int fd_index) {
    close(polling_data->pollfds[fd_index].fd); //close sockfd
    //Copy end over sockfd to delete with last entry while also updating count
    polling_data->pollfds[fd_index] = polling_data->pollfds[--polling_data->fd_count];
}

//get client sockfd to communicate on.
//Takes open sockfd to 'accept' with
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


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
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

