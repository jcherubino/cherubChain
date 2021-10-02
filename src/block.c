#include <string.h>
#include <stdio.h>
#include "block.h"
#include "stdlib.h"

//Print entire chain from head until the end of the chain
void print_chain(const struct Link* head) {
    //walk chain
    for (const struct Link* link = head; link != NULL; link = link->next) {
        printf("Previous block hash: %u\nHash: %u\nPayload: %s\n", link->block.prev_hash, link->block.hash,
                link->block.payload);
    }
}

//Add a new link to the end of the chain and return a pointer to it
//If head is NULL will start new chain
//Returns NULL ptr if fail
struct Link* append_link(struct Link* head) {
    struct Link* plink = (struct Link*) malloc(sizeof(struct Link));
    //Fail - leave error handling to caller
    if (plink == NULL) return NULL;

    plink->next = NULL;

    //connect to end of chain
    if (head == NULL) {
        //no chain exists - plink is head
        return plink;
    }
    
    //walk chain
    struct Link* tail;
    for (tail = head; tail->next != NULL; tail = tail->next);

    //connect in chain
    tail->next = plink;

    return plink;
}

//Free memory for chain, starting at end 
void delete_chain(struct Link** head_p) {
    struct Link* current = *head_p;
    struct Link* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    //Set head to NULL
    *head_p = NULL;
}

//set blocks payload to null terminated payload string
void add_payload(struct Block* block_p, const char* payload) {
    //+1 for null char
    size_t payload_sz = strlen(payload) + 1;
    payload_sz = (payload_sz > (MAX_PAYLOAD + 1)) ? MAX_PAYLOAD + 1: payload_sz;

    block_p->payload = (char *) malloc(payload_sz);
    memcpy(block_p->payload, payload, payload_sz);
    
    //ensure null termination
    *(block_p->payload + payload_sz - 1) = '\0';
}

