#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "block.h"
#include "server.h"

int main() {
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

    //print_chain(head);

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

    //Send block buffer
    struct BlockBuf bbuf = pack_block(head->next->next->next->block);

    size_t nbytes;
    if ((nbytes = send_buf(clientfd, bbuf.buf, bbuf.len)) == -1) {
        perror("send");
        return 1;
    }

    print_block(head->next->next->next->block);
    //free block buffer buffer 
    free(bbuf.buf);

    //close sockfds
    close(listenerfd);
    close(clientfd);

    //free entire chain
    free_chain(&head);

    return 0;
}

