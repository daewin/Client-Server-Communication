/********************************************************
 * Linked List Data Structure for Project 1
 * Adapted for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 19/5/2016
 ********************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "linked_list.h"


// Creates a new Linked List and returns it.
struct list* create_list(){
    struct list *new_list = malloc(sizeof(*new_list));
    
    if(new_list == NULL)
    {
        fprintf(stderr, "Error: Malloc Failed\n");
        exit(EXIT_FAILURE);
    }
    
    new_list->head = new_list->tail = NULL;
    
    return new_list;
}



// Inserts link to list to become the new tail
void list_insert(struct list* this_list, String data)
{
    check_list(this_list);
    
    struct node *n = malloc(sizeof(*n));
    
    if(n == NULL)
    {
        fprintf(stderr, "Error: Malloc Failed\n");
        exit(EXIT_FAILURE);
    }
    
    // For safety, we assert that data is not NULL
    assert(data != NULL);
    
    // Create a link with default values
    n->data = data;
    n->guessed = 0;
    n->index = get_list_size(this_list);
    n->next = NULL;
    
    if(is_empty(this_list))
    {
        // First entry
        this_list->head = this_list->tail = n;
        
    } else 
    {
        // Not the first, so we add to the end
        this_list->tail->next = n;
        this_list->tail = n;
    }     
}

// Removes node from the head of the list and sets the new head accordingly.
// Clears that particular node when done.
void list_remove(struct list* this_list)
{
    check_list(this_list);
    
    if(is_empty(this_list))
    {
        printf("List is already empty!\n");
        return;
    }
    
    struct node* old_head = NULL;
    struct node* new_head = NULL;
    
    // Sets old head
    old_head = this_list->head;
    
    // Set new_head as the next element
    new_head = this_list->head->next;
    
    // Replace current head with new_head
    this_list->head = new_head;
    
    // If this is the last element to be removed, set head and tail to NULL
    if(this_list->head == NULL)
    {
        this_list->tail = this_list->head;
    }
    
    // Free data in old head node
    old_head->data = NULL;
    free(old_head->data);
    
    // Free old head node
    old_head = NULL;
    free(old_head); 
}


// Checks if list is empty
int 
is_empty(struct list* this_list){
    
    check_list(this_list);
    
    if((this_list->head == NULL) && (this_list->tail == NULL)){
        return 1;
    }    
    return 0;
}


// Empties the list
void
empty_list(struct list* this_list)
{
    // We do this by calling list_remove until head is NULL
    check_list(this_list);
    
    while(!is_empty(this_list)){
        list_remove(this_list);
    }
    
}

// Checks if list is void
void
check_list(struct list* this_list){
    if(this_list == NULL)
    {
        // List has not been initialized
        fprintf(stderr, "Error: List has not been initialized\n");
        exit(EXIT_FAILURE);
        
    }
}

// Returns the number of lists in the linked list
int 
get_list_size(struct list* this_list)
{   
    check_list(this_list);
    
    struct node *temp = this_list->head;
    int count = 0;
    
    while(temp != NULL)
    {
        count++;    
        temp = temp->next;
    }
    
    return count;
}

void
print_list(struct list* this_list){
    
    struct node* temp = this_list->head;
    
    printf("[");
    while(temp){
        printf("(%s, %d)", temp->data, temp->guessed);
                
        temp = temp->next;
    }
    
    printf("]\n");
    
}