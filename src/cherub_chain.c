#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "block.h"
#include "server.h"
#include <signal.h>

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

    struct PollingData* polling_data = initialise_server();

    //Setup sigint handling
    struct sigaction act;
    act.sa_handler = sig_int_handler;
    sigaction(SIGINT, &act, NULL);

    printf("Initialisation complete, entering node loop\n"); 
    //main processing loop
    while(prog_run_status);
    
    printf("Shutdown signal received -- stopping node\n"); 
    deinitialise_server(&polling_data);

    free_chain(&head);

    return 0;
}

