#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "block.h"

#define BLOCK_DIV "------\n"

//Internal functions
static uint32_t hash_djb2(unsigned char *payload);

/**
 * Print representation of chain to stdout.
 * @param pblock_chain block chain struct containing chain to print
 * @return void
 */
void print_chain(const struct BlockChain * pblock_chain) {
    //walk chain
    for (const struct Link* link = pblock_chain->head; link != NULL; link = link->next) {
        printf(BLOCK_DIV);
        print_block(link->block);
    }
}

/**
 * Initialise block chain struct. 
 * @param void
 * @return populated block chain instance
 */
struct BlockChain initialise_chain(void) {
    struct BlockChain block_chain;
    block_chain.head = block_chain.tail = NULL; //no elements in chain yet
    block_chain.len = 0;
    return block_chain;
}

/**
 * Add a new link to the end of the chain. 
 * @param pblock_chain pointer to the block chain to append a link to
 * @return pointer to newly added link in chain. NULL ptr on failure
 */
struct Link* append_link(struct BlockChain* pblock_chain) {
    //Our chain is really really big :O
    if (pblock_chain->len == UINT32_MAX) {
        fprintf(stderr, "Block: Max chain length reached");
        return NULL;
    }
    struct Link* plink = malloc(sizeof(struct Link));
    //Leave error handling to caller
    if (plink == NULL)  {
        fprintf(stderr, "Block: Failed to allocate memory for link");
        return NULL;
    }

    //empty block chain. Set up genesis block
    if (pblock_chain->tail == NULL) {
        pblock_chain->head = pblock_chain->tail = plink; //single elem so head and tail equal
        plink->prev = NULL; //no previous blocks to point to
        plink->block.prev_hash = 0; //no previous hash exists
    }
    else {
        plink->prev = pblock_chain->tail;
        //link hash
        plink->block.prev_hash = pblock_chain->tail->block.hash;
        pblock_chain->tail->next = plink;
        //advance tail
        pblock_chain->tail = plink; 
    }

    plink->next = NULL;
    pblock_chain->len++; //increase size

    return plink;
}


/**
 * Free allocated memory for given block chain.
 * @param pblock_chain Chain to free
 * @return void
 */
void deinitialise_chain(struct BlockChain * pblock_chain) {
    struct Link *current, *next;
    current = pblock_chain->head;
    while (current != NULL) {
        next = current->next;
        free(current->block.payload); //free payload memory
        free(current); //free remainder of block
        current = next;
    }

    //show chain as being empty
    pblock_chain->head = pblock_chain->tail = NULL;
    pblock_chain->len = 0;
}

/**
 * Add block to blockchain. Helper function to encapsulate required steps
 * @param pblock_chain block chain to append to
 * @param payload payload of new block
 * @return 0 on success, -1 otherwise
 */
int add_block(struct BlockChain * pblock_chain, const char * payload) {

    struct Link * plink = append_link(pblock_chain);
    if (plink == NULL) {
        //Failed to append link
        return -1;
    }
    
    if (add_payload(&plink->block, payload) != 0) {
        //failed to add payload
        return -1;
    }
    
    hash_block(&plink->block);
    return 0;
}

/**
 * Print single block to stdou
 * @param block block to print
 * @return void
 */
void print_block(const struct Block block) {
    printf("Previous block hash: %u\nHash: %u\nPayload: %s\n", block.prev_hash,
            block.hash, block.payload);
}

/**
 * Set block payload to null terminated string. Allocates and copies memory
 * @param pblock Block to add payload to
 * @param payload string to copy into payload
 * @return 0 on success, -1 otherwise
 */
int add_payload(struct Block* pblock, const char* payload) {
    //+1 for null char
    size_t payload_sz = strlen(payload) + 1;
    payload_sz = (payload_sz > TOTAL_PAYLOAD_LEN) ? TOTAL_PAYLOAD_LEN: payload_sz;

    pblock->payload = malloc(payload_sz);

    if (pblock->payload == NULL) {
        fprintf(stderr, "Block: failed to allocate memory for payload");
        return -1;
    }
    memcpy(pblock->payload, payload, payload_sz);
    
    //ensure null termination -1 because of 0 indexing
    *(pblock->payload + payload_sz - 1) = '\0';
    return 0;
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

