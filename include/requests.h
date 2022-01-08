#ifndef _REQUESTS_H
#define _REQUESTS_H

#include "block.h"
#include "server.h"

//Define command enum
enum endpoint_id {
    ENDPOINT_CHAIN = 0,
    ENDPOINT_ADD_BLOCK = 1
};

int request_chain_endpoint(int sockfd, struct BlockChain * pblock_chain);
int request_add_block_endpoint(int sockfd, const char * payload);

#endif //_REQUESTS_H
