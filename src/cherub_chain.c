#include <stdio.h>
#include <string.h>
#include "block.h"
#include "server.h"

int main() {
    /*
    struct Link* head = initialise_chain();
    char payload_buf[MAX_PAYLOAD];
    add_payload(&head->block, "Genesis block choo choo all aboard the cherub chrain");
    hash_block(&head->block);
        
    int i; struct Link* plink;
    for (i = 0, plink = head; i < 10; i++) {
        plink = append_link(plink);
        sprintf(payload_buf, "Block number: %d", i+1);
        add_payload(&plink->block, payload_buf); 
        hash_block(&plink->block);
    }

    plink = append_link(plink);
    add_payload(&plink->block, "End of the chain :)"); 
    hash_block(&plink->block);

    print_chain(head);
    delete_chain(&head);
    */

    int listenerfd = get_listener();
    if (listenerfd == -1) {
        fprintf(stderr, "Failed to get listener\n");
        return 1;
    }
    printf("Got listener %d\n", listenerfd);
    int clientfd = get_client(listenerfd);
    if (clientfd == -1) {
        fprintf(stderr, "Failed to get client\n");
        return 1;
    }
    printf("Got client %d\n", listenerfd);

    //Receive 1 byte from client. Set flags to 0
    uint8_t data; int nbytes;
    if ((nbytes = recv(clientfd, &data, 1, 0)) == -1) {
        perror("recv");
        return 1;
    }
    
    printf("Received byte: %d\n", data);

    data = 133;
    if ((nbytes = send(clientfd, &data, 1, 0)) == -1) {
        perror("send");
        return 1;
    }

    //close sockfds
    close(listenerfd);
    close(clientfd);
    return 0;
}

