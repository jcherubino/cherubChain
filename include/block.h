#ifndef _BLOCK_H
#define _BLOCK_H
#include <stdint.h>

#define MAX_PAYLOAD UINT16_MAX /*Max length of transaction*/

struct Block {
    uint32_t prev_hash; /*hash of last block. 0 for gen*/
    uint32_t hash; /*hash of current block. Computed on transactoin data*/
    char* payload; /*Block payload */
};

/*Struct to store a packed block buffer*/
struct BlockBuf {
    uint8_t* buf; /*pointer to allocated buffer*/
    size_t len; /*Length of buf*/
};

/*single link in blockchain*/
struct Link {
    struct Block block; /*The chain 'links' block*/
    struct Link* next; /*Next block in the chain*/
};

/*Operations on chain*/
void print_chain(const struct Link* head);
struct Link* initialise_chain(void);
struct Link* append_link(struct Link* tail);
void free_chain(struct Link** phead);

/*Operations on block*/
void print_block(const struct Block block);
void add_payload(struct Block* pblock, const char* payload);
void pack_block(const struct Block block, struct BlockBuf *pblock_buf);
void hash_block(struct Block* pblock);
#endif //_BLOCK_H
