/**
 * Simple program to add a block to a running node
 * Written by Josh Cherubino (josh.cherubino@gmail.com)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "block.h"
#include "server.h"
#include "requests.h"

//Request chain endpoint on node at specified IP
int main(int argc, char * argv[]) {
     
    if (argc != 4) {
        fprintf(stderr, "usage: add_block hostname servname payload\n");
        return 1;
    }

    //connect to node with given hostname
    int node_fd = connect_to_node(argv[1], argv[2]);

    if (node_fd == -1) {
        return 2;
    }
    
    int ret = request_add_block_endpoint(node_fd, argv[3]);

    close(node_fd);
    
    //failed
    if (ret == -1) {
        return 3;
    }

    printf("Add block: Block successfully added with payload %s\n", argv[3]);
    return 0;
}

