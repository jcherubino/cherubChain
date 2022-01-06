#ifndef _ENDPOINTS_H
#define _ENDPOINTS_H

#include "block.h"

enum endpoint_dispatch_retval {
    DISPATCH_OK = 0,
    DISPATCH_UNKNOWN_ERR = -1,
    DISPATCH_INVALID_ENDPOINT = -2,
    DISPATCH_SEND_FAIL = -3,
    DISPATCH_RECV_FAIL = -4
};

enum endpoint_dispatch_retval endpoint_dispatch(unsigned int endpoint_id,
        int sockfd, struct BlockChain * pblock_chain);

#endif /*_ENDPOINTS_H*/
