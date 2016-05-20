/********************************************************
 * Linked List Header
 * Adapted for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 19/5/2016
 ********************************************************/

typedef char* String; 
 
struct node
{
    String data;
    int guessed;        // Has this colour been guessed
    int index;          // "Index" in the list
    struct node* next;
};

// List of Determining Nodes and Type Identifier
struct list
{
    struct node* head;
    struct node* tail;
};

struct list* create_list();
void list_insert(struct list* this_list, String data);
void list_remove(struct list* this_list);
int is_empty(struct list* this_list);
void empty_list(struct list* this_list);
void check_list(struct list* this_list);
int get_list_size(struct list* this_list);
void print_list(struct list* this_list);