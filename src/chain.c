#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "block.h"
#include "server.h"
#include "requests.h"

//Request chain endpoint on node at specified IP
int main(int argc, char * argv[]) {
     
    if (argc != 2) {
        fprintf(stderr, "usage: chain hostname\n");
        return 1;
    }

    //connect to node with given hostname
    int node_fd = connect_to_node(argv[1]);

    if (node_fd == -1) {
        return 2;
    }
    
    //Get chain
    struct BlockChain block_chain = initialise_chain();

    int ret = request_chain_endpoint(node_fd, &block_chain);

    print_chain(&block_chain);

    close(node_fd);
    deinitialise_chain(&block_chain);
    
    //failed
    if (ret == -1) {
        return 3;
    }
    return 0;
}

