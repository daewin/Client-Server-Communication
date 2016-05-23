/********************************************************
 * Game for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 21/5/2016
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "game.h"

String
codemaker_generate_code(){
    int i;
    
    // Malloc and intialize the secret code to "empty". This allows us
    // to return and use the secret code.    
    String secret_code = malloc((SECRETCODELENGTH+1) * sizeof(char));
    
    // Assert that malloc succeeded
    assert(secret_code);
    
    strcpy(secret_code, "");
    
    // Seed the randomizer with the current time
    srand(time(NULL));
    
    // Randomly generate and concatenate the code chars to secret_code
    for(i = 0; i < SECRETCODELENGTH; i++){
        // Memory Leak Info: char_to_string has been concatenated and freed
        // along with secret_code.
        strcat(secret_code, char_to_string(colours[rand() % strlen(colours)]));
    }
       
    return secret_code;
}


void
codemaker_provide_feedback(String secret_code, String input_code){
    // We use linked lists to keep track of the specific colour and locations
    // ("indexes") that have been guessed right. This is so we don't have 
    // overlapping and/or double checks.
    
    int b = 0, m = 0;
    
    struct list *secret_code_list = create_list();
    struct list *input_code_list = create_list();
    
    // Insert individual colours into list
    int i, j;
    
    for(i = 0; i < strlen(secret_code); i++){
        // Memory Leak Info: char_to_string's return value is freed after use
        // by the empty_list function.
        list_insert(secret_code_list, char_to_string(secret_code[i]));
    }
    
    for(j = 0; j < strlen(input_code); j++){
        // Memory Leak Info: Same as above.
        list_insert(input_code_list, char_to_string(input_code[j]));
    }
    
    
    struct node *secret_node = secret_code_list->head;
    struct node *input_node = input_code_list->head;    
    
    /* First we check for b (correct colours at correct positions). */
    // Just check at that index.
    while(secret_node && input_node){        
        // First, we check if these values have been guessed before.
        // This check is not really necessary now because this is the 
        // first pass, but for consistency sake, and because why not.
        if(!input_node->guessed && !secret_node->guessed){
            
            // Check if the indexes are the same.
            if(secret_node->index == input_node->index){       
                
                // If it is, we check if the colours are the same.
                if(strcmp(secret_node->data, input_node->data) == 0){
                    secret_node->guessed = 1;
                    input_node->guessed = 1;
                    b++;                
                }
            } 
        }
        input_node = input_node->next;
        secret_node = secret_node->next;        
    }
    
    
    // Reuse node from above.
    secret_node = secret_code_list->head;
    
    // Use two while loops to traverse through the lists
    // Just like a double for loop!    
    while(secret_node){
        
        // Make sure we don't check the ones that have already been guessed
        if(!secret_node->guessed)
        {            
            // Reuse node from above.
            input_node = input_code_list->head;
            
            while(input_node)
            {
                // Same as above.
                if(!input_node->guessed)
                {                  
                    // Now we check if there's a matching colour
                    if(strcmp(secret_node->data, input_node->data) == 0){
                        
                        // Just set the inputs' "guessed" to 1, since we'll be
                        // only doing one pass over secret_node_list.
                        input_node->guessed = 1;
                        m++;
                        
                        // We break to ensure this match is unique.
                        break;
                    }
                }
            
                input_node = input_node->next;
            }
        }
                      
        secret_node = secret_node->next;
    }

    printf("[%d:%d]\n", b, m);
    
    
    
    
    // Empty and free secret_code_list
    empty_list(secret_code_list);
    secret_code_list = NULL;
    free(secret_code_list);
    
    // Empty and free input_code_list
    empty_list(input_code_list);
    input_code_list = NULL;
    free(input_code_list);
    
    // Free secret and input codes
    secret_code = NULL;
    input_code = NULL;
    
    free(secret_code);
    free(input_code);
}




String
char_to_string(char character){
    // To simplify and avoid pesky EOL flag manipulations, and also to use
    // our String typedef, we use an "empty" string array.
    char secret_code_character[] = "0";
    
    // Replace the dummy value with our character
    secret_code_character[0] = character;


    int secret_code_length = strlen(secret_code_character) + 1;
    
    // Memory Leak Info: This malloc has been freed after being used.
    String secret_code_string = malloc(secret_code_length * sizeof(char));
    
    // Assert that malloc succeeded
    assert(secret_code_string);
    
    strcpy(secret_code_string, secret_code_character);

    return secret_code_string;
}


/* Check for any errors in the secret code.
 * Returns the error flag as described below.
 *      0 : No Error
 *      1 : Number of code invalid
 *      2 : Colour of code invalid
 */
int
is_code_invalid(String secret_code, int is_server){
    int i;
    
    if(strlen(secret_code) != SECRETCODELENGTH){
        if(is_server){
            fprintf(stderr, "Error: Four colours are required!\n");
        }
        
        return 1;
    } else {            
        // Check that the "codes" are allowed colours
        
        for(i = 0; i < SECRETCODELENGTH; i++)
        {
            String valid_colour = char_to_string(secret_code[i]);
            if(strstr(colours, valid_colour) == NULL){
                if(is_server){
                    fprintf(stderr, "Error: Only colours from \"");
                    fprintf(stderr, colours);
                    fprintf(stderr,"\" are allowed!\n");  
                }
                
                // Free char_to_string's return value if non-valid colour.
                free(valid_colour);
                return 2;
            }
            // Free char_to_string's return value if valid colour.
            free(valid_colour); 
        } 
    }
    
    // Code is valid
    return 0;
}