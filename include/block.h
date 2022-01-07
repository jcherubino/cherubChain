#ifndef _BLOCK_H
#define _BLOCK_H
#include <stdint.h>

#define MAX_PAYLOAD 1024 /*Max length of transaction exc null char*/
#define TOTAL_PAYLOAD_LEN MAX_PAYLOAD + 1 /*Actual size of buffer to alloc considering null char*/

struct Block {
    uint32_t prev_hash; /*hash of last block. 0 for gen*/
    uint32_t hash; /*hash of current block. Computed on transactoin data*/
    char* payload; /*Block payload */
};

/*single link in blockchain*/
struct Link {
    struct Block block; /*The chain 'links' block*/
    struct Link* next; /*Next block in the chain*/
    struct Link* prev; /*The previous block in the chain*/
};

struct BlockChain {
    uint32_t len; /*Length of block chain*/
    struct Link * head; /*Head of chain*/
    struct Link * tail; /*Tail of chain to speed up additions to chain*/
};

/*Struct to store a packed block buffer*/
struct BlockBuf {
    uint8_t* buf; /*pointer to allocated buffer*/
    size_t len; /*Length of buf*/
};

/*Operations on chain*/
//TODO: Make unneeded external functions internal only to allow creation of block
//only via add_block interface
struct BlockChain initialise_chain(void);
void deinitialise_chain(struct BlockChain * pblock_chain);
void print_chain(const struct BlockChain * pblock_chain);
struct Link* append_link(struct BlockChain* pblock_chain);
int add_block(struct BlockChain * pblock_chain, const char * payload);

/*Operations on block*/
void print_block(const struct Block block);
int add_payload(struct Block* pblock, const char* payload);
void pack_block(const struct Block block, struct BlockBuf *pblock_buf);
void hash_block(struct Block* pblock);
#endif //_BLOCK_H
