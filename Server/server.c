/********************************************************
 * Server-Side using TCP for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 22/5/2016
 
 *https://github.com/uchicago-cs/cmsc23300/blob/master/samples/sockets/server-pthreads.c
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
#include <pthread.h>

#include "server.h"

int
main(int argc, char* argv[]){
    
    // Representing our client
    struct sockaddr_in client;
    socklen_t clientaddrlen = sizeof(struct sockaddr_in);
    
    // Representing our server
    String secret_code;
    int socketfd, threadsocketfd, server_port;
    struct sockaddr_in server;
    
    // Representing our thread
    pthread_t worker_thread;
    struct workerArgs *wa;
        
       
    /* Parse arguments */
             
    // Required argument: Port Number.
    if(argc >= 2){
       
        server_port = atoi(argv[1]);
        
    } else {
        fprintf(stderr, "Usage: %s portno [secret_code]\n", 
                    argv[0]);
        exit(EXIT_FAILURE);
    }    
         
    // Optional argument: Secret Code. 
    // Memory Leak Info: secret_code will have a malloc-ed string either 
    // randomly (later) or as an argument (now), so we will free it when 
    // we are done with it, in the thread.
    if(argc == 3)
    {
        // Malloc necessary for error checking function later and also
        // for storing in the thread argument struct.
        secret_code = malloc((SECRETCODELENGTH+1) * sizeof(char));
        
        // Assert that malloc succeeded
        assert(secret_code);
        
        // Check that the code is valid.
        if(is_code_invalid(argv[2], 1)){
            // Error messages have already been printed to stderr
            free(secret_code);
            exit(EXIT_FAILURE);
        }      
        
        // Assign to argument input
        strcpy(secret_code, argv[2]);
    } else {
        // Set it to NULL to be generated randomly by the worker thread.
        secret_code = NULL;
    }
    
    
    /* Now we build the data structure for our sockets. */    
    memset((char *)&server, '\0', sizeof(server));
    memset((char *)&client, '\0', sizeof(client));
    
    // Server socket initialization
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(server_port);
    
    // Create a passive open socket for clients to connect
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error in creating a socket");
        free(secret_code);
        exit(EXIT_FAILURE);
    }        
    
    // Bind the socket to servers local address
    if(bind(socketfd, (struct sockaddr *)&server, sizeof(server)) < 0){        
        perror("Error in binding socket to the local address");
        free(secret_code);
        exit(EXIT_FAILURE);        
    }    
        
    // Now we start listening for clients
    if(listen(socketfd, MAXCLIENTS) < 0){
        perror("Error in listening for clients");
        free(secret_code);
        exit(EXIT_FAILURE);
    } else {
        printf("Listening on port %d...\n", ntohs(server.sin_port));
    }
    
    
    // The server continually accepts new connections and passes the work
    // off to the worker thread.
    while(1)
    {   
        if ((threadsocketfd = accept(socketfd, (struct sockaddr *) &client,
                                                        &clientaddrlen)) < 0){
            // If this particular connection fails, no need to kill the server.
            perror("Accepting connection failed");
            continue;
        }
        
        
        String client_ip = inet_ntoa(client.sin_addr);        
        printf("Connection accepted from client %s\n", client_ip);        
        
        // Apparently, it's good practice to pass in a struct that encapsulates
        // the parameters to the thread. So that's what we shall do.
        // Memory Leak Info: This will be freed in the thread.
        wa = malloc(sizeof(struct workerArgs));
        wa->socket = threadsocketfd;
        wa->secret_code = secret_code;
        
        
        if(pthread_create(&worker_thread, NULL, worker_function, (void*)wa) != 0)
        {
            perror("Error in creating a worker thread");
            free(wa);
            free(secret_code);
            
            close(threadsocketfd);
            close(socketfd);
            exit(EXIT_FAILURE);            
        }           
    }
    
    
    // Use the Ctrl+C to end thing here...
    close(socketfd);
    
    return 0;    
}



// This will handle the game for each client as a thread.
void *worker_function(void* args)
{
    int threadsocketfd, attempt = 0, i;
    struct workerArgs *wa;
    char server_message[BUFFERSIZE+1];
    String client_secret_code, secret_code;
    
    
    // Unpack the Worker Arguments
    wa = (struct workerArgs*)args;
    
    threadsocketfd = wa->socket;                // Free later
    secret_code = wa->secret_code;              // Free later
    
    if(secret_code == NULL){
        // If the secret code was not provided as an argument, randomly
        // generate it. Now it is malloc-ed for both cases.
        secret_code = codemaker_generate_code();        
    }
    printf("Socket number: %d\n", threadsocketfd);
    printf("Final Secret Message: %s\n", secret_code);  
    
    /* This tells the pthreads library that no other thread is going to
    join() this thread. This means that, once this thread terminates,
    its resources can be safely freed (instead of keeping them around
    so they can be collected by another thread join()-ing this thread) */
	//pthread_detach(pthread_self());
       
    // Send introduction message
    memset(server_message, '\0', BUFFERSIZE+1);    
    strcpy(server_message, "Introduction...\n");
    
    if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0)
    {
      perror("Error writing to the socket");
      exit(EXIT_FAILURE);
    }
    
    // Check if client has acknowledged message
    while(!acknowledge_sent(threadsocketfd));
    

    
    // Send request to client to submit a guess of the secret code.
    memset(server_message, '\0', BUFFERSIZE+1);
    strcpy(server_message, "Now, make a guess of the secret code. You have ");
    
    String guesses_string = itoa(MAXGUESSES);
    strcat(server_message, guesses_string);
    free(guesses_string);
    
    strcat(server_message, " attempts at getting it right!\n");
    
    if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0)
    {
      perror("Error writing to the socket");
      exit(EXIT_FAILURE);
    }
    
    // Check if client has acknowledged message
    while(!acknowledge_sent(threadsocketfd));
    
    
    // Malloc to store clients code and pass it to the code checker
    // Memory Leak Info: client_secret_code has been freed after use
    client_secret_code = malloc((SECRETCODELENGTH+1) * sizeof(char));
    
    // Assert that malloc succeeded
    assert(client_secret_code);
    
    
    // Interacts with client. My communication flow between the client and
    // server is structured such that the messages are synchronized as such:
    // [Loop]
    //   1.)  Server -> Client : Too many attempts? (yes/no)
    //   1a.)    == END ==     : Yes, both break out of loop and 
    //                           client closes connection. [FAILURE]
    //   1b.) == DO NOTHING == : No
    //   2.)  Client -> Server : Client sends code.
    // [Inner Loop]
    //   3.)  Server -> Client : Is code valid? (yes/no)
    //   3a.) Client -> Server : No, Client sends corrected code
    //   3b.)                  : Yes, both break out of loop.
    // [End Inner Loop]
    //   3.)  Server -> Client : Is the code right? (yes/no)
    //   3a.)                  : No, next iteration for both loops.
    //   3b.)                  : Yes, both break out of loop and 
    //                           client closes connection.
    // [End Loop]
    
    while(1)
    {
        ////////////////////////////////////// Block 1 //////////////////////////////////////
        // 1.)  Server -> Client : Too many attempts? (yes/no)
        if(attempt >= MAXGUESSES){
            // 1a.) Yes, both break out of loop and client closes connection.
            memset(server_message, '\0', BUFFERSIZE+1);  
              
            strcpy(server_message, 
              "FAILURE: There are no more attempts left. The secret code is ");
            strcat(server_message, secret_code);
            strcat(server_message, "\n");
            

            if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0)
            {
                perror("Error writing to the socket");
                exit(EXIT_FAILURE);
            }
            
            break;
            
        } else {
            // 1b.) No, send the number of attempts so far.
            memset(server_message, '\0', BUFFERSIZE+1);    
            sprintf(server_message, "Attempt %d/%d: ", attempt, MAXGUESSES);
            

            if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0)
            {
                perror("Error writing to the socket");
                exit(EXIT_FAILURE);
            }
            
        }
        
        // Check if client has acknowledged message
        while(!acknowledge_sent(threadsocketfd));
        
        
        ////////////////////////////////////////////////////////////////////////////////////   
           
        // 2.) Client -> Server : Client sends code.
        memset(server_message, '\0', BUFFERSIZE+1);        
        if(recv(threadsocketfd, server_message, BUFFERSIZE+1, 0) < 0){
            perror("Error reading from the socket");
            exit(EXIT_FAILURE);
        }
        
        // Make client_secret_code an empty string
        strcpy(client_secret_code, "");
        
        // Add only the code to client_secret_code
        for(i = 0; i < SECRETCODELENGTH; i++)
        {
            // Memory Leak Info: char_to_string has been concatenated and freed
            // along with client_secret_code.
            strcat(client_secret_code, char_to_string(server_message[i]));
        }                
        
        // 3.)  Server -> Client : Is code valid? (yes/no)
        // We keep looping through until the code entered is valid.
        while(1)
        {
            int validity = is_code_invalid(client_secret_code, 0);
            if(validity == 1){
                memset(server_message, '\0', BUFFERSIZE+1);    
                strcpy(server_message, "INVALID: Four colours are required!\n");
                
                if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0)
                {
                    perror("Error writing to the socket");
                    exit(EXIT_FAILURE);
                }
                
            } else if (validity == 2){
                memset(server_message, '\0', BUFFERSIZE+1);    
                strcpy(server_message, "INVALID: Only colours from \"");
                strcat(server_message, colours);
                strcat(server_message, "\" are allowed!\n");
                
                if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0)
                {
                    perror("Error writing to the socket");
                    exit(EXIT_FAILURE);
                }
                
            } else {
                memset(server_message, '\0', BUFFERSIZE+1);    
                strcpy(server_message, "");
                
                if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0)
                {
                    perror("Error writing to the socket");
                    exit(EXIT_FAILURE);
                }
                break;   
            }
        }

        
        if(strcmp(client_secret_code, secret_code) == 0)
        {
            memset(server_message, '\0', BUFFERSIZE+1);    
            strcpy(server_message, "SUCCESS: You guessed it right!\n");
            
            if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0)
            {
                perror("Error writing to the socket");
                exit(EXIT_FAILURE);
            }
            break;
            
        } else {            
            memset(server_message, '\0', BUFFERSIZE+1);    
            strcpy(server_message, 
                codemaker_provide_feedback(secret_code, client_secret_code));
            
            if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0)
            {
                perror("Error writing to the socket");
                exit(EXIT_FAILURE);
            }
            
            
        }        
        
        attempt++;
    }
    
    printf("Socket %d disconnected\n", threadsocketfd);
    close(threadsocketfd);
    
    // Free secret and input codes
    secret_code = NULL;
    client_secret_code = NULL;
    
    free(secret_code);
    free(wa);
    free(client_secret_code);
    
    pthread_exit(NULL);
    
}
    



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

    // Memory Leak Info: This will be freed after call.
    String output = malloc(count * sizeof(char));
    
    // Assert that malloc succeeded
    assert(output);
    
    snprintf(output, count, "%d", num);
    
    return output;
}


// We loop this until acknowledged
int
acknowledge_sent(int threadsocketfd){
    char message[BUFFERSIZE+1];
    memset(message, '\0', BUFFERSIZE+1);
    
    if(recv(threadsocketfd, message, BUFFERSIZE+1, 0) < 0)
    {
        perror("Error reading from the socket");
        close(threadsocketfd);
        exit(EXIT_FAILURE);
    }
    
    if(strstr(message, RECEIVED) != NULL){
        // Client received message
        return 1;
    }
    
    return 0; 
}



int
acknowledge_received(int threadsocketfd){
    if(send(threadsocketfd, RECEIVED, strlen(RECEIVED), 0) < 0)
    {
        perror("Error writing to the socket");
        close(threadsocketfd);
        exit(EXIT_FAILURE);
    }
    
    return 1;      
}