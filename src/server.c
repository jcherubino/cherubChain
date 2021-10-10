#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include "server.h"

#define PORT "33333"  //port users connect to

#define CON_BACKLOG 1   // only hold 1 pending connection at a time

//Get 'listening' sockfd 
//Return -1 on error
//Code mostly from from Beej's Guide to Network Programming
int get_listener(void) {
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

    if (listen(sockfd, CON_BACKLOG) == -1) {
        perror("listen");
        close(sockfd);
        return -1;
    }

    //if we have reached this point. Then we may return our open sockfd to
    //accept new cons from
    return sockfd;
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
    
    return new_fd;
}


//send buffer to given connected sockfd
//return -1 on failure otherwise return number of bytes written
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

