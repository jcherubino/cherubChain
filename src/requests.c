/**
 * Interface to make a properly formed request to a particular server endpoint
 * and process the returned values where appropriate
 * Written by Josh Cherubino (josh.cherubino@gmail.com)
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "requests.h"

//Internal functions
static inline int send_endpoint_request(int sockfd, const enum endpoint_id);

/**
 * Request chain endpoint. Read the received data into a block chain struct
 * @param sockfd socket to request the endpoint on 
 * @param pblock_chain Initialised chain to append the received data into
 * @return 0 on success and -1 on failure
 */
int request_chain_endpoint(int sockfd, struct BlockChain * pblock_chain) {
    uint32_t network_len, host_len;

    if (send_endpoint_request(sockfd, ENDPOINT_CHAIN) == 1) {
        return -1;
    }

    //Read length of chain from sock
    int ret = receive_buf(sockfd, &network_len, sizeof(network_len));

    //failed
    if (ret <= 0) {
        return -1;
    }

    host_len = ntohl(network_len);

    //Receive all blocks
    for (int i = 0; i < host_len; i++) {
        //append each block to the chain
        if (unpack_block(sockfd, pblock_chain) == -1) {
            return -1;
        }
    }
    return 0;
}


/**
 * Request add block endpoint. Add block 
 * @param sockfd socket to transmit on
 * @param null terminated string containing payload to transmit
 * @return 0 on success and -1 on failure
 */
int request_add_block_endpoint(int sockfd, const char * payload) {
    //request endpoint
    if (send_endpoint_request(sockfd, ENDPOINT_ADD_BLOCK) == 1) {
        return -1;
    }
    
    size_t payload_len = strlen(payload);
    if (payload_len > MAX_PAYLOAD) {
        fprintf(stderr, "Requests: Specified payload size %lu larger than max allowed payload%d\n",
                payload_len, MAX_PAYLOAD);
        return -1;
    }
    
    uint16_t network_payload_len = htons((uint16_t)payload_len);

    if (send_buf(sockfd, &network_payload_len, sizeof(network_payload_len)) == -1) {
        return -1;
    }

    if (send_buf(sockfd, payload, payload_len) == -1) {
        return -1;
    }

    return 0;
}

/**
 * Send single byte request with specified endpoint id. 
 * @param sockfd Socket to send request on 
 * @return 0 on sucess and -1 on failure
 */
static inline int send_endpoint_request(int sockfd, const enum endpoint_id endpoint_id) {
    const uint8_t id = (uint8_t) endpoint_id; //Cast to uint8 explicitly so we can be sure of size for transmission
    ssize_t sent = send(sockfd, &id, 1, 0);

    if (sent == -1) {
        perror("Requests: send");
        return -1;
    }
    return 0;
}
