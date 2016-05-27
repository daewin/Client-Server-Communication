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
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#include "client.h"

int
main(int argc, char* argv[]){
        
    // Representing our client
    char client_message[BUFFERSIZE+1];
    int socketfd;
    
    // Representing our server
    String input_hostname;
    struct hostent *server_ent;
    struct sockaddr_in server;
    int server_port;
    
    // Handle SIGPIPE signal by ignoring it.
    signal(SIGPIPE, SIG_IGN);
    
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
    bcopy((char *)server_ent->h_addr_list[0], (char *)&server.sin_addr.s_addr, 
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
                                    sizeof(server)) < 0){
                socket_error_handler("Error in connecting to the server", socketfd);
            }
            
        
    // Read introduction message from server
    memset(client_message, '\0', BUFFERSIZE+1);
    
    if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
    {
        socket_error_handler("Error reading from the socket", socketfd);
    }
    printf("%s", client_message);
    
    // Acknowledge message has been received
    acknowledge_received(socketfd);
    
    
    
    // Read "make a guess" message from server
    memset(client_message, '\0', BUFFERSIZE+1);

    if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
    {
        socket_error_handler("Error reading from the socket", socketfd);
    }    
    printf("%s", client_message);
    
    // Acknowledge message has been received
    acknowledge_received(socketfd);
    
    
    
    while(1)
    {        
        ////////////////////////////////////// Block 1 //////////////////////////////////////
        // Read if there are too many attempts.
        memset(client_message, '\0', BUFFERSIZE+1);

        if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
        {
            socket_error_handler("Error reading from the socket", socketfd);
        }     
        
        // Acknowledge message has been received
        acknowledge_received(socketfd);            
        
        if(strstr(client_message, FAILURE) != NULL){
            // If "FAILURE" receipt found, print message, and end connection.
            printf("%s", client_message);            
            break;
            
        } else {
            // Print "FAILURE" receipt not found, print attempt message.
            printf("%s", client_message);
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
                socket_error_handler("Error writing to the socket", socketfd);
            }
            
            // Check if server has acknowledged message
            while(!acknowledge_sent(socketfd));
            
            ////////////////////////////////////// Block 2a //////////////////////////////////////
            // Read if code is valid
            memset(client_message, '\0', BUFFERSIZE+1);
        
            if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
            {
                socket_error_handler("Error reading from the socket", socketfd);
            }
            
            // Acknowledge message has been received
            acknowledge_received(socketfd);     
            
            if(strstr(client_message, INVALID) != NULL){
                // If "INVALID" receipt found, we also check the following
                // message for a "FAILURE" receipt indicating that the attempts
                // have exceeded MAXATTEMPTS. We break twice if so. Else, just
                // print.
                printf("%s", client_message);

                // Print new attempt/failure message
                memset(client_message, '\0', BUFFERSIZE+1);
            
                if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
                {
                    socket_error_handler("Error reading from the socket", socketfd);
                }
                
                // Acknowledge message has been received
                acknowledge_received(socketfd);   
                
                // Check if the "FAILURE" receipt is present
                if(strstr(client_message, FAILURE) != NULL){
                    // We have, so we print and break here once
                    printf("%s", client_message);
                    break;
                } 

                // Not a failure, so we just print the attempt message
                printf("%s", client_message);
                
                
            } else {
                // No "INVALID" receipt found, so code was valid. Break.
                break;
            }            
        }
        /////////////////////////////////////////////////////////////////////////////////////
        
        // Check if the "FAILURE" receipt is present
        if(strstr(client_message, FAILURE) != NULL){
            // We have, so we break here again to end client
            break;
        }       
        
        ////////////////////////////////////// Block 3 //////////////////////////////////////
        // Read result of our code.
        memset(client_message, '\0', BUFFERSIZE+1);
    
        if(recv(socketfd, client_message, BUFFERSIZE+1, 0) < 0)
        {
            socket_error_handler("Error reading from the socket", socketfd);
        }

        // Acknowledge message has been received
        acknowledge_received(socketfd);    
        
            
        if(strstr(client_message, SUCCESS) != NULL){
            // If "SUCCESS" receipt found, print message and break
            printf("%s", client_message);
            
            break;
            
        } else {
            // No "SUCCESS" receipt found, so we print feedback
            printf("%s", client_message);
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
        socket_error_handler("Error reading from the socket", socketfd);
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
        socket_error_handler("Error writing to the socket", socketfd);
    }
    
    return 1;      
}


void
socket_error_handler(String error_message, int socketfd){
    perror(error_message);
    fprintf(stderr, "Or, the server has been disconnected.\n");
    close(socketfd);
    exit(EXIT_FAILURE);
}
