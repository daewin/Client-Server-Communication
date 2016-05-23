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

#define SECRETCODELENGTH 4
#define BUFFERSIZE 1024

int
main(int argc, char* argv[]){
        
    // Representing our client
    char client_message[BUFFERSIZE+1];
    int socketfd;
     
    
    // Representing our server
    char *input_hostname;
    struct hostent *server;
    struct sockaddr_in server_addr;
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
    server = gethostbyname(input_hostname);
    
    if(server == NULL){
        fprintf(stderr, "Error: Unknown Server Hostname/IP\n");
        exit(EXIT_FAILURE);
    }
    
    
    /* Now we build the data structure for our socket. */
    memset((char *)&server_addr, '\0', sizeof(server_addr));
    
    // Using IPv4
    server_addr.sin_family = AF_INET;
    
    // Copy the IP address from the hostname entry to the socket data structure
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, 
                server->h_length);
    
    // Ensure it is stored in network byte order
    server_addr.sin_port = htons(server_port);
    
    
    /* We can now create the socket */
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error in creating a socket");
        exit(EXIT_FAILURE);
    }
    
    /* If there are no problems, connect to the server */
    if (connect(socketfd, (struct sockaddr *)&server_addr, 
                                    sizeof(server_addr)) < 0)
            {
                perror("Error in connecting to the server");
                close(socketfd);
                exit(EXIT_FAILURE);
            }

    // Read introduction message from server
    memset(client_message, '\0', BUFFERSIZE+1);
    
    if(read(socketfd, client_message, BUFFERSIZE+1) < 0)
    {
      perror("Error in reading from the socket");
      exit(EXIT_FAILURE);
    }
    printf("%s", client_message);
    
    
    
    // Read "make a guess" message from server
    memset(client_message, '\0', BUFFERSIZE+1);
    if(read(socketfd, client_message, BUFFERSIZE+1) < 0)
    {
      perror("Error reading from the socket");
      exit(EXIT_FAILURE);
    }    
    printf("%s", client_message);
    
    
    
    // Write your guess to the server
    bzero(client_message, BUFFERSIZE+1);
    fgets(client_message, BUFFERSIZE, stdin);
    
    if(write(socketfd, client_message, strlen(client_message)) < 0)
    {
      perror("Error writing to the socket");
      exit(EXIT_FAILURE);
    }
    
    
    
    // Close the socket after we're done
    close(socketfd);
    
    return 0;
}
