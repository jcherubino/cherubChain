#ifndef _BLOCK_H
#define _BLOCK_H
#include <stdint.h>
#include <stdlib.h>

#define MAX_PAYLOAD 1024 /*Max length of transaction exc null char*/
#define TOTAL_PAYLOAD_LEN MAX_PAYLOAD + 1 /*Actual size of buffer to alloc considering null char*/

struct Block {
    uint32_t prev_hash; /*hash of last block. 0 for gen*/
    uint32_t hash; /*hash of current block. Computed on transactoin data*/
    uint16_t payload_len; /*length of payload excluding null char*/
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

/*Operations on chain*/
//TODO: Make unneeded external functions internal only to allow creation of block
//only via add_block interface
struct BlockChain initialise_chain(void);
void deinitialise_chain(struct BlockChain * pblock_chain);
void print_chain(const struct BlockChain * pblock_chain);
struct Link* append_link(struct BlockChain* pblock_chain);
int add_block(struct BlockChain * pblock_chain, const char * payload);
int unpack_block(int sockfd, struct BlockChain * pblock_chain);

/*Operations on block*/
void print_block(const struct Block block);
int add_payload(struct Block* pblock, const char* payload, uint8_t length_known);
void pack_block(const struct Block block, uint8_t** pbuf, size_t* len);
void hash_block(struct Block* pblock);
#endif //_BLOCK_H
