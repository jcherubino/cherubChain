/**
 * Interface to handle calling an appropriate endpoint function
 * when requested by a connected socket.
 * Written by Josh Cherubino (josh.cherubino@gmail.com)
 */
#include <stdio.h>
#include <stdlib.h>
#include "endpoints.h"
#include "block.h"
#include "server.h"

//Endpoint function typedef
typedef enum endpoint_dispatch_retval (*endpoint_f)(int sockfd, struct BlockChain * pblock_chain);

//Function prototypes
static enum endpoint_dispatch_retval chain_endpoint(int sockfd, struct BlockChain * pblock_chain);
static enum endpoint_dispatch_retval add_block_endpoint(int sockfd, struct BlockChain * pblock_chain);

//Define dispatch table of endpoints
static endpoint_f ENDPOINT_DISPATCH_TABLE[] = {
    chain_endpoint, //Endpoint 0
    add_block_endpoint //Endpoint 1
};

//Store compile time number of endpoints for iteration
static size_t N_ENDPOINTS = sizeof(ENDPOINT_DISPATCH_TABLE)/sizeof(*ENDPOINT_DISPATCH_TABLE);

/**
 * Function to handle dispatching to endpoint. Provides interface and error handling before 
 * executing pre-defined endpoints 
 * @param endpoint_id Unique id specifying which endpoint should be called
 * @param sockfd Open socket that requested the endpoint
 * @param head pointer to the head of the nodes chain
 * @return Result status of endpoint call
 */
enum endpoint_dispatch_retval endpoint_dispatch(unsigned int endpoint_id,
        int sockfd, struct BlockChain * pblock_chain) {

    if (endpoint_id >= N_ENDPOINTS) {
        fprintf(stderr, "Endpoints: Invalid endpoint: %d\n", endpoint_id);
        return DISPATCH_INVALID_ENDPOINT;
    }
    
    //Run desired endpoint
    return ENDPOINT_DISPATCH_TABLE[endpoint_id](sockfd, pblock_chain);
}

/**
 * Internal chain endpoint. Transmits entire block chain with pre-defined bit stream structure
 * @param sockfd Open socket that requested the endpoint
 * @param pblock_chain block chain to transmit
 * @return execution result of endpoint
 */
static enum endpoint_dispatch_retval chain_endpoint(int sockfd, struct BlockChain * pblock_chain) {
    uint8_t *buf = NULL; //set to NULL to allocate initial memory
    size_t len;
    int32_t nbytes;
    
    //Send chain length
    uint32_t network_chain_length = htonl(pblock_chain->len);
    if ((nbytes = send_buf(sockfd, (uint8_t*)&network_chain_length, sizeof(uint32_t))) == -1) {
        return DISPATCH_SEND_FAIL;
    }

    for (const struct Link* link = pblock_chain->head; link != NULL; link = link->next) {
        pack_block(link->block, &buf, &len);
        if (buf == NULL) {
            return DISPATCH_UNKNOWN_ERR;
        }
        if ((nbytes = send_buf(sockfd, buf, len)) == -1) {
                return DISPATCH_SEND_FAIL;
        }
    }
    
    printf("Endpoints: chain transmitted successfully\n");
    //once done transmitting, free buffer
    free(buf);
    return DISPATCH_OK;
}

/**
 * Internal add_block endpoint. Reads in transmitted payload and appends link to the chain
 * @param sockfd Open socket that requested the endpoint
 @ @param pblock_chain block chain to append to
 * @return execution result of adding block
 */
static enum endpoint_dispatch_retval add_block_endpoint(int sockfd, struct BlockChain * pblock_chain) {
    char payload_buf[TOTAL_PAYLOAD_LEN];
    uint16_t network_payload_sz, payload_sz;

    //Read length of payload from sock
    int ret = receive_buf(sockfd, &network_payload_sz, sizeof(payload_sz));
    //failed
    if (ret <= 0) {
        return DISPATCH_RECV_FAIL;
    }
    //convert to host byte-order
    payload_sz = ntohs(network_payload_sz); 
    
    if (payload_sz > MAX_PAYLOAD) {
        fprintf(stderr, "Endpoints: Specified payload size %d larger than max allowed payload %d\n",
                payload_sz, MAX_PAYLOAD);
        return DISPATCH_INVALID_ARGS;
    }

    //attempt to read that many bytes into a static buffer (make it the size of the payload)
    ret = receive_buf(sockfd, payload_buf, payload_sz);
    if (ret <= 0) {
        return DISPATCH_RECV_FAIL;
    }
    //Add null termination
    payload_buf[payload_sz] = '\0';

    if (add_block(pblock_chain, payload_buf) != 0) {
        return DISPATCH_UNKNOWN_ERR;
    }
    printf("Endpoints: block added successfully\n");
    return DISPATCH_OK;
}

