#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "endpoints.h"
#include "block.h"
#include "server.h"

//Setup signal handler to gracefully exit
static volatile sig_atomic_t prog_run_status = 1;

void sig_int_handler(int _) {
    (void)_;
    printf("\r"); //carriage return to remove ^C from output terminal
    prog_run_status = 0;    
}

int main() {
    struct Link* head = initialise_chain();
    char payload_buf[MAX_PAYLOAD];
    add_payload(&head->block, "Genesis block choo choo all aboard the cherub chrain");
    hash_block(&head->block);
        
    int i; struct Link* plink;
    for (i = 0, plink = head; i < 10; i++) {
        plink = append_link(plink);
        sprintf(payload_buf, "Block number: %d", i+1);
        add_payload(&plink->block, payload_buf); 
        hash_block(&plink->block);
    }

    plink = append_link(plink);
    add_payload(&plink->block, "End of the chain :)"); 
    hash_block(&plink->block);

    print_chain(head);

    struct ServerData* server_data = initialise_server();

    //Setup sigint handling
    struct sigaction act;
    act.sa_handler = sig_int_handler;
    sigaction(SIGINT, &act, NULL);

    printf("Initialisation complete, entering node loop\n"); 
    //main processing loop
    while(prog_run_status) {
        int poll_count = poll(server_data->pollfds, server_data->fd_count, 1);

        if (poll_count == -1) {
            perror("poll");
        }

        // Run through the existing connections looking for data to read
        for(int i = 0; i < server_data->fd_count; i++) {

            // Check if someone's ready to read
            if (server_data->pollfds[i].revents & POLLIN) { 

                if (server_data->pollfds[i].fd == server_data->listenerfd) {
                    // If listener is ready to read, handle new connection

                    int new_fd = get_client(server_data->listenerfd);

                    if (new_fd != -1) {
                        add_fd_to_server(server_data, new_fd);
                    }
                } else {
                    // If we have received POLLIN revent from socket that is not the listener,
                    //we must handle receiving the data
                    uint8_t endpoint_id;
                    //Read endpoint id to determine appropriate endpoint to run
                    int sender_fd = server_data->pollfds[i].fd;
                    size_t nbytes = recv(sender_fd, &endpoint_id, 1, 0);

                    if (nbytes <= 0) {
                        // Got error or connection closed by client
                        if (nbytes == 0) {
                            // Connection closed
                            printf("Server: socket %d hung up\n", sender_fd);
                        } else {
                            perror("recv");
                        }
                        
                        //close and remove closed fd from polling data 
                        delete_fd_from_server(server_data, i);

                    } else {
                        //Try to dispatch to the requested endpoint
                        if (endpoint_dispatch(endpoint_id, sender_fd, head) != DISPATCH_OK) {
                            //If we do not receive OK response then drop the connection
                            fprintf(stderr, "Dropping connection: %d\n", sender_fd);
                            delete_fd_from_server(server_data, i); 
                        }
                    }
                } // END handle data from client
            } // END got ready-to-read from poll()
        } // END looping through file descriptors
    } // END main node loop
    
    printf("Shutdown signal received -- stopping node\n"); 

    deinitialise_server(&server_data);
    free_chain(&head);

    return 0;
}

