#include <stdio.h>
#include "block.h"

int main() {
    
    struct Link* head = append_link(NULL);
    head->block.prev_hash = 0;
    head->block.hash = 12345;
    add_payload(&head->block, "Genesis block"); 
    append_link(head);
    add_payload(&head->next->block, "Other block"); 
    head->next->block.prev_hash = head->block.hash;
    head->next->block.hash = 666;
    print_chain(head);
    
    delete_chain(&head);
    return 0;
}


