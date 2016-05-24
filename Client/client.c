/********************************************************
 * Client-Side using TCP for Project 2
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

#include "client.h"

int
main(int argc, char* argv[]){
        
    // Representing our client
    char client_message[BUFFERSIZE+1];
    int socketfd;     
    char attempt_message[ATTEMPTSIZE+1];
    
    // Representing our server
    String input_hostname;
    struct hostent *server_ent;
    struct sockaddr_in server;
    int server_port;
    
    
    if(argc == 3){
        input_hostname = argv[1];
        server_port = atoi(argv[2]);    
    } else {
        fprintf(stderr, "Usage: %s hostname portno\n", 
                    argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // Retrieve host information corresponding to a Hostname/IP from
    // a host database. In this case, obtain the server entry.
    server_ent = gethostbyname(input_hostname);
    
    if(server_ent == NULL){
        fprintf(stderr, "Error: Unknown Server Hostname/IP\n");
        exit(EXIT_FAILURE);
    }
    
    
    /* Now we build the data structure for our socket. */
    memset((char *)&server, '\0', sizeof(server));
    
    // Using IPv4
    server.sin_family = AF_INET;
    
    // Copy the IP address from the hostname entry to the socket data structure
    bcopy((char *)server_ent->h_addr, (char *)&server.sin_addr.s_addr, 
                server_ent->h_length);
    
    // Ensure it is stored in network byte order
    server.sin_port = htons(server_port);
    
    
    // We can now create the socket
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error in creating a socket");
        exit(EXIT_FAILURE);
    }
    
    // If there are no problems, connect to the server 
    if (connect(socketfd, (struct sockaddr *)&server, 
                                    sizeof(server)) < 0)
            {
                perror("Error in connecting to the server");
                close(socketfd);
                exit(EXIT_FAILURE);
            }
            
        
    // Read introduction message from server
    memset(client_message, '\0', BUFFERSIZE+1);
    
    if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
    {
        perror("Error reading from the socket");
        close(socketfd);
        exit(EXIT_FAILURE);
    }
    printf("%s", client_message);
    
    // Acknowledge message has been received
    acknowledge_received(socketfd);
    
    
    
    // Read "make a guess" message from server
    memset(client_message, '\0', BUFFERSIZE+1);

    if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
    {
        perror("Error reading from the socket");
        close(socketfd);
        exit(EXIT_FAILURE);
    }    
    printf("%s", client_message);
    
    // Acknowledge message has been received
    acknowledge_received(socketfd);
    


    // Interacts with server. My communication flow between the client and
    // server is structured such that the messages are synchronized as such:
    // [Loop]
    //   1.)  Server -> Client : Too many attempts? (yes/no)
    //   1a.)    == END ==     : Yes, both break out of loop and 
    //                           client closes connection. [FAILURE]
    //   1b.) == DO NOTHING == : No [GO]
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
        // Read if there are too many attempts.
        memset(client_message, '\0', BUFFERSIZE+1);

        if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
        {
            perror("Error reading from the socket");
            close(socketfd);
            exit(EXIT_FAILURE);
        }     
        
        // Acknowledge message has been received
        acknowledge_received(socketfd);            
        
        if(strstr(client_message, FAILURE) != NULL){
            // If "FAILURE" receipt found, print message, and end connection.
            printf(client_message);            
            break;
            
        } else {
            // Print "FAILURE" receipt not found, print attempt message.
            printf(client_message);
            
            // Store attempt
            memset(attempt_message, '\0', ATTEMPTSIZE+1);
            strcpy(attempt_message, client_message);
        }
        /////////////////////////////////////////////////////////////////////////////////////
        
        ////////////////////////////////////// Block 2 //////////////////////////////////////
        // Client sends code 
        // We keep looping through until the code entered is valid.
        while(1)
        {
            // Write code to the server
            memset(client_message, '\0', BUFFERSIZE+1);
            scanf("%s", client_message);            
            
            if(send(socketfd, client_message, strlen(client_message), 0) < 0)
            {
                perror("Error writing to the socket");
                close(socketfd);
                exit(EXIT_FAILURE);
            }
            
            // Check if server has acknowledged message
            while(!acknowledge_sent(socketfd));
            
            ////////////////////////////////////// Block 2a //////////////////////////////////////
            // Read if code is valid
            memset(client_message, '\0', BUFFERSIZE+1);
        
            if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
            {
                perror("Error reading from the socket");
                close(socketfd);
                exit(EXIT_FAILURE);
            }
            
            // Acknowledge message has been received
            acknowledge_received(socketfd);     
            
            if(strstr(client_message, INVALID) != NULL){
                // If "INVALID" receipt found, print message and retry
                printf(client_message);
                printf(attempt_message);
            } else {
                // No "INVALID" receipt found, so code was valid. Break.
                break;
            }            
        }
        /////////////////////////////////////////////////////////////////////////////////////
        
        ////////////////////////////////////// Block 3 //////////////////////////////////////
        // Read result of our code.
        memset(client_message, '\0', BUFFERSIZE+1);
    
        if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
        {
            perror("Error reading from the socket");
            close(socketfd);
            exit(EXIT_FAILURE);
        }

        // Acknowledge message has been received
        acknowledge_received(socketfd);    
        
            
        if(strstr(client_message, SUCCESS) != NULL){
            // If "SUCCESS" receipt found, print message and break
            printf(client_message);
            
            break;
            
        } else {
            // No "SUCCESS" receipt found, so we print feedback
            printf(client_message);
        }  
    }
    // Close the socket after we're done
    close(socketfd);
    
    return 0;
}



int
acknowledge_sent(int socketfd){
    char message[BUFFERSIZE+1];
    memset(message, '\0', BUFFERSIZE+1);
    
    if(recv(socketfd, message, BUFFERSIZE+1, 0) < 0)
    {
        perror("Error reading from the socket");
        close(socketfd);
        exit(EXIT_FAILURE);
    }
    
    if(strstr(message, RECEIVED) != NULL){
        // Server received message
        return 1;
    }
    
    return 0; 
}



int
acknowledge_received(int socketfd){
    if(send(socketfd, RECEIVED, strlen(RECEIVED), 0) < 0)
    {
        perror("Error writing to the socket");
        close(socketfd);
        exit(EXIT_FAILURE);
    }
    
    return 1;      
}