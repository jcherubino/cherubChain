#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "block.h"
#include "server.h"

int main(int argc, char * argv[]) {
     
    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        return 1;
    }

    int node_fd = connect_to_node(argv[1]);

    close(node_fd);
    return 0;
}

