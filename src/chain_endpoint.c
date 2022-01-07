#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "block.h"
#include "server.h"

//Request chain endpoint on node at specified IP
int main(int argc, char * argv[]) {
     
    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        return 1;
    }

    //connect to node with given hostname
    int node_fd = connect_to_node(argv[1]);

    if (node_fd == -1) {
        return 2;
    }
    
    //Get chain

    close(node_fd);
    return 0;
}

