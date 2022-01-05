#include <stdio.h>
#include <stdlib.h>
#include "endpoints.h"
#include "block.h"
#include "server.h"

//Endpoint function typedef
typedef enum endpoint_dispatch_retval (*endpoint_f)(int sockfd, struct Link * head);

//Function prototypes
static int chain(int sockfd, struct Link * head);

//Define dispatch table of endpoints
static endpoint_f ENDPOINT_DISPATCH_TABLE[] = {
    chain
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
        int sockfd, struct Link * head) {

    if (endpoint_id >= N_ENDPOINTS) {
    fprintf(stderr, "Server: Invalid endpoint: %d\n", endpoint_id);
    return DISPATCH_INVALID_ENDPOINT;
    }
    
    //Run desired endpoint
    return ENDPOINT_DISPATCH_TABLE[endpoint_id](sockfd, head);
}

/**
 * Internal chain endpoint. Transmits entire block chain with pre-defined bit stream structure
 * @param sockfd Open socket that requested the endpoint
 * @return
 */
static enum endpoint_dispatch_retval chain(int sockfd, struct Link * head) {
    //Temporary send back single block
    struct BlockBuf bbuf;
    bbuf.buf = NULL; //set to NULL to allocate initial memory
    pack_block(head->block, &bbuf);
    int32_t nbytes;
    
    if ((nbytes = send_buf(sockfd, bbuf.buf, bbuf.len)) == -1) {
            perror("send");
            return DISPATCH_SEND_FAIL;
    }
    //once done transmitting, free buffer
    free(bbuf.buf);
    return DISPATCH_OK;
}
