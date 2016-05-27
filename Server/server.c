/********************************************************
 * Server-Side using TCP for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182 (dsv)
 * Date Modified: 25/5/2016
 *
 * The following helped immensely in my Server-(Multi)Client implementation:
 * https://github.com/uchicago-cs/cmsc23300/blob/master/samples/sockets/server-pthreads.c
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
#include <signal.h>

#include "server.h"


/* Globally accessible variables and a mutex.
   All global variables are changed only when it has obtained 
   a mutex lock. */
FILE *fp;

// Allows the structure to represent the current time for our log.
time_t rawtime;
struct tm *timeinfo;

// Mutex lock
pthread_mutex_t lock;

// Socket ID to be handled by sighandler
int socketfd;

// Server 'Performance' and usage statistics
int successful_connections = 0;
int successful_guesses = 0;


int
main(int argc, char* argv[]){
   
    // Representing our client
    struct sockaddr_in client;
    socklen_t clientaddrlen = sizeof(struct sockaddr_in);
    
    // Representing our server
    String secret_code;
    int threadsocketfd, server_port;
    struct sockaddr_in server;
    
    // Representing our thread
    pthread_t worker_thread;
    struct workerArgs *wa;
    
    // Initialize the mutex lock
    if(pthread_mutex_init(&lock, NULL) != 0){
        fprintf(stderr, "Error: Mutex lock initialization has failed!\n");
        exit(EXIT_FAILURE);
    }
    
    // Create (or overwrite) a new log file.
    if((fp = fopen(LOGFILE, "w")) == NULL)
    {
        perror("File Error");
        exit(EXIT_FAILURE);
    }        
    
    // Assign our sighandler function to handle signal Ctrl+C (SIGINT 2)
    signal(SIGINT, sighandler);
       
    // Handle SIGPIPE signal by ignoring it.
    signal(SIGPIPE, SIG_IGN);
       
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
            // Error messages have already been printed to stderr.
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
    }
    
    printf("\nServer is running. Press Ctrl+C to exit!\n");
    
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
        
        // Apparently, it's good practice to pass in a struct that encapsulates
        // the parameters to the thread. So that's what we shall do.
        // Memory Leak Info: This will be freed in the thread.
        wa = malloc(sizeof(struct workerArgs));
        
        // Assert that malloc worked
        assert(wa);
                
        // Get the Client's IP Address
        String client_ip = malloc((INET_ADDRSTRLEN + 1) * sizeof(char));
        inet_ntop(AF_INET, &(client.sin_addr), client_ip, INET_ADDRSTRLEN);
        
        // Assign the struct components
        wa->socket = threadsocketfd;
        wa->secret_code = secret_code;
        wa->client_ip = client_ip;
        
        if(pthread_create(&worker_thread, NULL, 
                        worker_function, (void*)wa) != 0)
        {
            perror("Error in creating a worker thread");
            free(wa);
            free(secret_code);
            
            close(threadsocketfd);
            close(socketfd);
            exit(EXIT_FAILURE);            
        }
        
        
        // Log the client connection.        
        // Message will be freed after being written to the log file.
        String log_client_connection = 
            malloc(((LOGMESSAGESIZE + 1) * sizeof(char)) 
                                    + sizeof(threadsocketfd));
          
        sprintf(log_client_connection, 
                    "(%d) client connected", threadsocketfd); 
        add_log_entry(client_ip, NULL, log_client_connection);
        
        free(log_client_connection);
        
        // Since only one client can connect to the server in one iteration,
        // mutex locks are not necessary. 
        successful_connections += 1;

    }
    
    // Should not reach here
    close(socketfd);
    
    return 0;    
}

/* This will handle the game for each client as a thread. */
void *worker_function(void* args)
{
    /* This tells the pthreads library that no other thread is going to
    join() this thread. This means that, once this thread terminates,
    its resources can be safely freed (instead of keeping them around
    so they can be collected by another thread join()-ing this thread) 
    The above is written by Borja Sotomayor (link in header comment) */
	pthread_detach(pthread_self());
    
    // Worker Function variables
    int threadsocketfd, attempt = 0;
    struct workerArgs *wa;
    char server_message[BUFFERSIZE+1];
    String client_secret_code, secret_code, client_ip;
        
    
    // Unpack the Worker Arguments
    wa = (struct workerArgs*)args;
    
    threadsocketfd = wa->socket;
    secret_code = wa->secret_code;
    client_ip = wa->client_ip;
    
    if(secret_code == NULL){
        // If the secret code was not provided as an argument, randomly
        // generate it. Now it is malloc-ed for both cases.
        secret_code = codemaker_generate_code();        
    }
    
    // We ensure that secret code should never be NULL from here on
    if(secret_code == NULL){
        fprintf(stderr, "Error: Secret Code is NULL\n");
        close(threadsocketfd);
        pthread_exit(NULL);      
    }
    
    // Log the secret code generation.        
    // Message will be freed after being written to the log file.
    String log_secret_code = 
        malloc((LOGMESSAGESIZE + 1 + strlen(secret_code) + 1) * sizeof(char));
    sprintf(log_secret_code, " server secret = %s", secret_code);
    add_log_entry(MOCKSERVERIP, NULL, log_secret_code);
    
    free(log_secret_code);
   
       
    // Send introduction message
    memset(server_message, '\0', BUFFERSIZE+1);    
    strcpy(server_message, INTRODUCTION);
    
    if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0){
        socket_error_handler("Error writing to the socket", threadsocketfd);
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
    
    if(send(threadsocketfd, server_message, strlen(server_message), 0) < 0){
        socket_error_handler("Error writing to the socket", threadsocketfd);
    }
    
    // Check if client has acknowledged message
    while(!acknowledge_sent(threadsocketfd));
    
    
    // Malloc to store clients code and pass it to the code checker
    // Memory Leak Info: client_secret_code has been freed after use
    client_secret_code = malloc((SECRETCODELENGTH+1) * sizeof(char));
    
    // Assert that malloc succeeded
    assert(client_secret_code);
    
    
    // Interacts with client. My communication flow between the client and
    // server is structured such that the messages are synchronized.    
    while(1)
    {
        /////////////////////////////// Block 1 ///////////////////////////////
        // Check and send message if too many attempts have been made.
        if(attempt >= MAXGUESSES){
            // Yes, both break out of loop and client closes connection.
            memset(server_message, '\0', BUFFERSIZE+1);  
              
            strcpy(server_message, 
              "FAILURE: There are no more attempts left. The secret code is ");
            strcat(server_message, secret_code);
            strcat(server_message, "\n");
            

            if(send(threadsocketfd, server_message, 
                            strlen(server_message), 0) < 0){
                socket_error_handler("Error writing to the socket", threadsocketfd);
            }
            
            // Check if client has acknowledged message
            while(!acknowledge_sent(threadsocketfd));     
            
            // Log the servers response.        
            // Message will be freed after being written to the log file.
            String log_response_failure = 
                malloc((LOGMESSAGESIZE + 1) * sizeof(char));
            
            // Just log our response to the client
            sprintf(log_response_failure, " FAILURE game over");
            
            add_log_entry(client_ip, &threadsocketfd, log_response_failure);
                    
            free(log_response_failure);
                        
            break;
            
        } else {
            // No, send the number of attempts so far.
            memset(server_message, '\0', BUFFERSIZE+1);    
            sprintf(server_message, "Attempt %d/%d: ", attempt+1, MAXGUESSES);
            

            if(send(threadsocketfd, server_message, 
                            strlen(server_message), 0) < 0){
                socket_error_handler("Error writing to the socket", 
                                                        threadsocketfd);
            }
            
            // Check if client has acknowledged message
            while(!acknowledge_sent(threadsocketfd));       
        }
        ///////////////////////////////////////////////////////////////////////        
           
        /////////////////////////////// Block 2 ///////////////////////////////   
        // Client sends code 
        // We keep looping through until the code client entered is valid.
        while(1)
        {            
            // Read guess from client
            memset(server_message, '\0', BUFFERSIZE+1);        
            if(recv(threadsocketfd, server_message, BUFFERSIZE+1, 0) < 0){
                socket_error_handler("Error reading from the socket", threadsocketfd);
            }
            
            // Acknowledge message has been received
            acknowledge_received(threadsocketfd);  
            
            //////////////////////////// Block 2a /////////////////////////////            
            // Check if the code is valid.
            int validity = is_code_invalid(server_message, 0);
            if(validity == 1){       
                         
                // Log the client's invalid guess        
                // Message will be freed after being written to the log file.
                String log_guess_invalid = 
                    malloc((LOGMESSAGESIZE + 1 + strlen(server_message) + 1) 
                                                               * sizeof(char));
                    
                sprintf(log_guess_invalid, 
                        " client's guess = %s", server_message);
                
                add_log_entry(client_ip, &threadsocketfd, log_guess_invalid);
                free(log_guess_invalid);
                
                
                // Code is invalid: Not four colours
                memset(server_message, '\0', BUFFERSIZE+1);    
                strcpy(server_message, 
                        "INVALID: Four colours are required!\n");
                
                if(send(threadsocketfd, server_message, 
                                strlen(server_message), 0) < 0){
                    socket_error_handler("Error writing to the socket", 
                                                            threadsocketfd);      
                }
                
                // Check if client has acknowledged message 
                while(!acknowledge_sent(threadsocketfd));
        
                
            } else if (validity == 2){
                                
                // Log the client's invalid guess        
                // Message will be freed after being written to the log file.
                String log_guess_invalid = 
                    malloc((LOGMESSAGESIZE + 1 + strlen(server_message) + 1) 
                                                               * sizeof(char));
                    
                sprintf(log_guess_invalid, 
                                " client's guess = %s", server_message);
                
                add_log_entry(client_ip, &threadsocketfd, log_guess_invalid);
                free(log_guess_invalid);
                
                
                // Code is invalid: Not allowed colours
                memset(server_message, '\0', BUFFERSIZE+1);    
                strcpy(server_message, "INVALID: Only colours from \"");
                strcat(server_message, colours);
                strcat(server_message, "\" are allowed!\n");
                
                if(send(threadsocketfd, server_message, 
                                strlen(server_message), 0) < 0){
                    socket_error_handler("Error writing to the socket", 
                                                               threadsocketfd);
                }
                
                // Check if client has acknowledged message 
                while(!acknowledge_sent(threadsocketfd));
                
            } else {
                // Code is valid
                // Assign it to the malloc-ed client_secret_code.
                strcpy(client_secret_code, server_message);
                                
                memset(server_message, '\0', BUFFERSIZE+1);    
                
                // We just use a dummy empty string. I'm sure there is a more
                // elegant approach to this.
                strcpy(server_message, "DUMMY");
                
                if(send(threadsocketfd, server_message, 
                                strlen(server_message), 0) < 0){
                    socket_error_handler("Error writing to the socket", 
                                                               threadsocketfd);
                }
                        
                // Check if client has acknowledged message
                while(!acknowledge_sent(threadsocketfd));
                
                // Code is valid so we break out of loop.
                break;   
            }
            
            /////////////// INVALID HERE ////////////////
            // Log the servers response. Only "INVALID" responses reaches here.     
            // Message will be freed after being written to the log file.
            String log_response_invalid = 
                malloc((LOGMESSAGESIZE + 1) * sizeof(char));
            
            // Log server's response to the client
            sprintf(log_response_invalid, 
                            " server's response = INVALID input is invalid");
            
            add_log_entry(MOCKSERVERIP, NULL, log_response_invalid);
            free(log_response_invalid);
            
            // As each INVALID input is counted as a "guess", 
            // we increment attempt.
            attempt++;
            
            // Check that attempt is not >= MAXGUESSES
            if(attempt >= MAXGUESSES){
                break;
            }
            
            // We send the new attempt value to the client
            memset(server_message, '\0', BUFFERSIZE+1);    
            sprintf(server_message, "Attempt %d/%d: ", attempt+1, MAXGUESSES);
            
            if(send(threadsocketfd, server_message, 
                            strlen(server_message), 0) < 0){
                socket_error_handler("Error writing to the socket", 
                                                            threadsocketfd);
            }
            
            // Check if client has acknowledged attempt message
            while(!acknowledge_sent(threadsocketfd));
            
        }
    
        // Check that attempt is not >= MAXGUESSES
        // If it is, we continue to next iteration and handle appropriately
        if(attempt >= MAXGUESSES){
            continue;
        }
    
        ///////////////////////////////////////////////////////////////////////
        
        // Log the client's valid guess        
        // Message will be freed after being written to the log file.
        String log_response_valid = 
            malloc((LOGMESSAGESIZE + 1 + strlen(client_secret_code) + 1) * sizeof(char));
          
        sprintf(log_response_valid, " client's guess = %s", client_secret_code);
        
        add_log_entry(client_ip, &threadsocketfd, log_response_valid);
        free(log_response_valid);
        
        /////////////////////////////// Block 3 ///////////////////////////////
        // Send result of the clients code.
        String feedback = 
            codemaker_provide_feedback(secret_code, client_secret_code);
            
        if(strcmp(client_secret_code, secret_code) == 0)
        {
            // Increase successful guess count
            client_guess_successful();
            
            memset(server_message, '\0', BUFFERSIZE+1);    
            strcpy(server_message, 
                        "SUCCESS: You guessed it right! Game Over\n");
            
            if(send(threadsocketfd, server_message, 
                                strlen(server_message), 0) < 0)
            {
                socket_error_handler("Error writing to the socket", 
                                                            threadsocketfd);
            }

            // Check if client has acknowledged message
            while(!acknowledge_sent(threadsocketfd));         

            // Log the servers response.        
            // Message will be freed after being written to the log file.
            String log_response_feedback = 
                malloc((LOGMESSAGESIZE + 1 + strlen(feedback) + 1) 
                                                            * sizeof(char));
            
            // Just log our response to the client
            sprintf(log_response_feedback, " server's hint = %s", feedback);
            
            add_log_entry(MOCKSERVERIP, NULL, log_response_feedback);
            free(log_response_feedback);      
            
            // Log the successful guess.        
            // Message will be freed after being written to the log file.
            String log_response_success = 
                malloc((LOGMESSAGESIZE + 1) * sizeof(char));
            
            // Just log our response to the client
            sprintf(log_response_success, " SUCCESS game over");
            
            add_log_entry(client_ip, &threadsocketfd, log_response_success);
            free(log_response_success);
            
            break;
            
        } else {            
            memset(server_message, '\0', BUFFERSIZE+1);                
            
            strcpy(server_message, "Result: ");
            strcat(server_message, feedback);
            strcat(server_message, "\n");
            
            if(send(threadsocketfd, server_message, 
                                         strlen(server_message), 0) < 0){
                socket_error_handler("Error writing to the socket", 
                                                        threadsocketfd);
            }
            
            // Check if client has acknowledged message
            while(!acknowledge_sent(threadsocketfd));  
            
            // Log the servers response.        
            // Message will be freed after being written to the log file.
            String log_response_feedback = 
                malloc((LOGMESSAGESIZE + 1 + strlen(feedback) + 1) 
                                                          * sizeof(char));
            
            // Just log our response to the client
            sprintf(log_response_feedback, " server's hint = %s", feedback);
            
            add_log_entry(MOCKSERVERIP, NULL, log_response_feedback);
            free(log_response_feedback);   
        }    
        // Free feedback
        free(feedback);
        ///////////////////////////////////////////////////////////////////////      
        
        attempt++;
    }
    
    close(threadsocketfd);
    
    // Free secret and input codes, client ip string and the structure
    secret_code = NULL;
    client_secret_code = NULL;
    client_ip = NULL;
    wa = NULL;
    
    free(secret_code);
    free(client_secret_code);
    free(client_ip);    
    free(wa);
    
    
    pthread_exit(NULL);
    
}


// Interrupt Signal Handler
// All server threads and clients have a socket error handler which 
// exits gracefully (sortof) with a message.  
void
sighandler(int signum){
    
    // First we flush our output buffer (if there's anything in there),
    // then write the performance and resource usage statistics.
    fflush(fp);
    
    printf("\nPrinting log...");
    
    // Lock the mutex (or block until flush is complete)
    pthread_mutex_lock(&lock);  
       
    print_usage();

    // Unlock the mutex
    pthread_mutex_unlock(&lock);
    
    printf("Done. Exiting!\n");
    
    // Destroy the mutex
    pthread_mutex_destroy(&lock);
    
    // Close the socket
    close(socketfd);
    
    // Close the file
    fclose(fp);
    
    exit(EXIT_SUCCESS);
}

