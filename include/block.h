#ifndef _BLOCK_H
#define _BLOCK_H
#include <stdint.h>

#define BLOCK_TRANSACTIONS 10U /*Number of transactions each block stores*/
#define MAX_PAYLOAD 20U /*Max length of transaction*/

struct Block {
    uint32_t prev_hash; /*hash of last block. 0 for gen*/
    uint32_t hash; /*hash of current block. Computed on transactoin data*/
    char* payload; /*Block payload */
};

/*single link in blockchain*/
struct Link {
    struct Block block; /*The chain 'links' block*/
    struct Link* next; /*Next block in the chain*/
};

/*Operations on chain*/
void print_chain(const struct Link* head);
struct Link* append_link(struct Link* tail);
void delete_chain(struct Link** head_p);

void add_payload(struct Block* block_p, const char* payload);
#endif //_BLOCK_H
