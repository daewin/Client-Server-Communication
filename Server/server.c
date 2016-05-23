/********************************************************
 * Server-Side using TCP for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 22/5/2016
 ********************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <assert.h>

#include "server.h"

int
main(int argc, char* argv[]){
    
    // Representing our client
    struct sockaddr_in client;
    socklen_t clientaddrlen;
    int attempt = 0;
    String client_secret_code;
    
    // Representing our server
    char server_message[BUFFERSIZE+1];
    String secret_code;
    int socketfd, newsocketfd, server_port;
    int fixed_secret_code = 0;
    struct sockaddr_in server;
        
    
    /* Parse arguments */     
    
    // Required argument: Port Number.
    if(argc >= 2){
       
        server_port = atoi(argv[1]);
        
    } else {
        fprintf(stderr, "Usage: %s portno [secret_code]\n", 
                    argv[0]);
        exit(EXIT_FAILURE);
    }
    
    //// Diagnostic
    printf("Server Port Number: %d\n", server_port);
    
    
    // Optional argument: Secret Code.
    // Memory Leak Info: secret_code will have a malloc-ed String either 
    // randomly (later) or as an argument, so we will free it when finished.
        
    if(argc == 3){
        // Malloc necessary for error checking function later and also
        // because argv[2] is not a String.
        secret_code = malloc((SECRETCODELENGTH+1) * sizeof(char));
        
        // Assert that malloc succeeded
        assert(secret_code);
        
        // Check that the code is valid.
        if(is_code_invalid(argv[2], 1)){
            // Error messages have already been printed to stderr
            exit(EXIT_FAILURE);
        }      
        
        // Assign to argument input
        strcpy(secret_code, argv[2]);    
        
        // Flag to ensure we free only when closing server
        ///////// Make sure to free later //////////////
        fixed_secret_code = 1;
        printf("Final Secret Message: %s\n", secret_code);
    }  
    
    
    /* Now we build the data structure for our sockets. */
    memset((char *)&client, '\0', sizeof(client));
    memset((char *)&server, '\0', sizeof(server));
    
    // Server socket initialization
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(server_port);
    
    // Create a passive open socket for clients to connect
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error in creating a socket");
        exit(EXIT_FAILURE);
    }
        
    
    // Bind the socket to servers local address
    if(bind(socketfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        
        perror("Error in binding socket to the local address");
        exit(EXIT_FAILURE);        
    }
    
        
    // Now we start listening for clients
    if(listen(socketfd, MAXCLIENTS) < 0){
        perror("listen() failed with error");
        exit(EXIT_FAILURE);
    } else {
        printf("Listening on port %d...\n", ntohs(server.sin_port));
    }
    
    
    // Accept connection to the client
    
    /*
        Pthreads here possibly
     */
     
    clientaddrlen = sizeof(client);
    
    if((newsocketfd = accept(socketfd, (struct sockaddr *) &client, &clientaddrlen)) < 0)
    {
        perror("Accepting connection failed");
        exit(EXIT_FAILURE);
    } else {
        char ip4[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client.sin_addr), ip4, INET_ADDRSTRLEN);
        printf("connection accepted from client %s\n", ip4);
    }
    
    // Send introduction message
    memset(server_message, '\0', BUFFERSIZE+1);    
    strcpy(server_message, "Introduction...\n");
    
    if(write(newsocketfd, server_message, strlen(server_message)) < 0)
    {
      perror("Error writing to the socket");
      exit(EXIT_FAILURE);
    }
    
    
    // If the secret code was not provided as an argument, randomly generate it
    if(!fixed_secret_code){
        secret_code = codemaker_generate_code();
        printf("Final Secret Message: %s\n", secret_code);    
    }
    
    // Send request to client to submit a guess of the secret code.
    memset(server_message, '\0', BUFFERSIZE+1);
    strcpy(server_message, "Now, make a guess of the secret code. You have ");
    
    String temp = itoa(MAXGUESSES);
    strcat(server_message, temp);
    free(temp);
    
    strcat(server_message, " attempts!\n");
    
    
    if(write(newsocketfd, server_message, strlen(server_message)) < 0)
    {
      perror("Error writing to the socket");
      exit(EXIT_FAILURE);
    }
    
    
    // Read clients guess
    while(attempt < MAXGUESSES)
    {    
        memset(server_message, '\0', BUFFERSIZE+1);
        if(read(newsocketfd, server_message, BUFFERSIZE+1) < 0)
        {
            perror("Error in reading from the socket");
            exit(EXIT_FAILURE);
        }
        
        // Store clients code
        client_secret_code = malloc((SECRETCODELENGTH+1) * sizeof(char));
        
        // Assert that malloc succeeded
        assert(client_secret_code);
        
        strcpy(client_secret_code, "");
        
        // Add only the code to client_secret_code
        int i; 
        for(i = 0; i < SECRETCODELENGTH; i++){
            // Memory Leak Info: char_to_string has been concatenated and freed
            // along with client_secret_code.
            strcat(client_secret_code, char_to_string(server_message[i]));
        }
        
        printf("Clients strlen: %d\n", (int)strlen(client_secret_code));
        printf("Clients guess: %s\n", client_secret_code);
        printf("Size: %d\n", (int)sizeof(client_secret_code));
        
        // Check that the code is valid.
        if(is_code_invalid(client_secret_code, 1)){
            // Error messages have already been printed to stderr
            exit(EXIT_FAILURE);
        }   
        
        if(strcmp(client_secret_code, secret_code) == 0){
            printf("Correct!");
            break;
        } else {
            /* We write to client */
            //codemaker_provide_feedback(secret_code, combination);
            
            memset(server_message, '\0', BUFFERSIZE+1);    
            strcpy(server_message, "Wrong! Try again...\n");
            
            if(write(newsocketfd, server_message, strlen(server_message)) < 0)
            {
            perror("Error writing to the socket");
            exit(EXIT_FAILURE);
            }
            
            
        }        
        
        attempt++;
    }
    
    
    
    
    close(newsocketfd);
    
    
    
    

    
    close(socketfd);
    
    return 0;    
}


// Helper Functions

/* Integer to ASCII converter. Obtained and modified from: 
 * http://www.cprogramming.com/tips/tip/converting-numbers-to-strings 
 * to account for different numbers.
 */
String itoa(int num)
{ 
    int count = 0;
    int n = num;
    
    
    while(n != 0){
        n /= 10;
        count++;
    }
    
    // Add the null byte
    count += 1;

    String output = malloc(count * sizeof(char));
    
    // Assert that malloc succeeded
    assert(output);
    
    snprintf(output, count, "%d", num);
    
    return output;
}