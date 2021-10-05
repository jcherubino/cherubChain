#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "block.h"

#define BLOCK_DIV "------\n"

//Internal functions
static uint32_t hash_djb2(unsigned char *payload);

//Print entire chain from head until the end of the chain
void print_chain(const struct Link* head) {
    //walk chain
    for (const struct Link* link = head; link != NULL; link = link->next) {
        printf(BLOCK_DIV);
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
        //In this case we have genesis block
        plink->block.prev_hash = 0;
        return plink;
    }
    
    //walk chain
    struct Link* tail;
    for (tail = head; tail->next != NULL; tail = tail->next);

    //connect in chain
    tail->next = plink;
    //link hash
    plink->block.prev_hash = tail->block.hash;

    return plink;
}

//Free memory for chain, starting at end 
void delete_chain(struct Link** phead) {
    struct Link* current = *phead;
    struct Link* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    //Set head to NULL
    *phead = NULL;
}

//set blocks payload to null terminated payload string
void add_payload(struct Block* pblock, const char* payload) {
    //+1 for null char
    size_t payload_sz = strlen(payload) + 1;
    payload_sz = (payload_sz > (MAX_PAYLOAD + 1)) ? MAX_PAYLOAD + 1: payload_sz;

    pblock->payload = (char *) malloc(payload_sz);
    memcpy(pblock->payload, payload, payload_sz);
    
    //ensure null termination
    *(pblock->payload + payload_sz - 1) = '\0';
}


//Interface to hash block. 
//Abstracts underlying hash function from user code
void hash_block(struct Block* pblock) {
    pblock->hash = hash_djb2((unsigned char *)pblock->payload);
}

//djb2 hash function - http://www.cse.yorku.ca/~oz/hash.html
//TODO: Replace with cryptographic hash function
static uint32_t hash_djb2(unsigned char *payload) {
    uint32_t hash = 5381;
    int32_t c;

    while ((c = *payload++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

