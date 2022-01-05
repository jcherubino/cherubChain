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
        print_block(link->block);
    }
}

//Initialise new linked list
//Returns NULL ptr on failure
struct Link* initialise_chain(void) {
    struct Link * plink = malloc(sizeof(struct Link));
    if (plink == NULL) return NULL;

    plink->next = NULL;
    return plink;
}

//Add a new link to the end of the chain and return a pointer to it
//Returns NULL ptr if fail
struct Link* append_link(struct Link* head) {
    if (head == NULL) return NULL;

    struct Link* plink = malloc(sizeof(struct Link));
    //Leave error handling to caller
    if (plink == NULL) return NULL;

    plink->next = NULL;

    //walk chain
    struct Link* tail;
    for (tail = head; tail->next != NULL; tail = tail->next);

    //connect in chain
    tail->next = plink;
    //link hash
    plink->block.prev_hash = tail->block.hash;

    return plink;
}

//Free memory for chain
void free_chain(struct Link** phead) {
    struct Link* current = *phead;
    struct Link* next;
    while (current != NULL) {
        next = current->next;
        free(current->block.payload); //free payload memory
        free(current); //free remainder of block
        current = next;
    }
    //Set head to NULL
    *phead = NULL;
}

//print single block to stdout
void print_block(const struct Block block) {
    printf("Previous block hash: %u\nHash: %u\nPayload: %s\n", block.prev_hash,
            block.hash, block.payload);
}

//set blocks payload to null terminated payload string
void add_payload(struct Block* pblock, const char* payload) {
    //+1 for null char
    size_t payload_sz = strlen(payload) + 1;
    payload_sz = (payload_sz > (MAX_PAYLOAD + 1)) ? MAX_PAYLOAD + 1: payload_sz;

    pblock->payload = malloc(payload_sz);
    memcpy(pblock->payload, payload, payload_sz);
    
    //ensure null termination
    *(pblock->payload + payload_sz - 1) = '\0';
}

/**
 * Populate block buffer. Fills block buffer with appropriate bitstream of data to transmit.
 * Manually pack to avoid platform dependant alignment/packing bytes in Block struct.
 * @param block to pack into buffer
 * @param pblock_buf block buffer to fill. Must have buf attribute set to NULL for initial 
 * allocation or a previously allocated set of memory (such as from previous call to pack_block)
 * @return void (instead populates the given pblock_buf)
 */
void pack_block(const struct Block block, struct BlockBuf *pblock_buf) {
    size_t block_sz;
    uint16_t payload_sz = strlen(block.payload);
    //must send length length of payload which will vary between blocks.
    //payload length, Prev hash, hash, payload
    block_sz = sizeof(uint16_t) + 2*sizeof(uint32_t) + payload_sz;

    //realloc so successive calls to pack_block can re-use same memory for efficiency
    pblock_buf->buf = realloc(pblock_buf->buf, block_sz);

    if (pblock_buf->buf == NULL) {
        pblock_buf->len = 0;
    }

    pblock_buf->len = block_sz;

    //get network ordered data
    uint32_t net_prev_hash = htonl(block.prev_hash);
    uint32_t net_hash = htonl(block.hash);
    uint16_t net_payload_sz = htons(payload_sz);

    //pack data into buf
    uint8_t *cur = pblock_buf->buf;
    memcpy(cur, &net_payload_sz, sizeof(uint16_t)); cur+= sizeof(uint16_t); 
    memcpy(cur, &net_prev_hash, sizeof(uint32_t)); cur+= sizeof(uint32_t); 
    memcpy(cur, &net_hash, sizeof(uint32_t)); cur+= sizeof(uint32_t); 
    memcpy(cur, block.payload, payload_sz);
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

