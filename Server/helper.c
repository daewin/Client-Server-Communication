/********************************************************
 * Server-Side Helper Functions for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 26/5/2016
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
 
#include "helper.h"

// Extern values from server for our helper to use
extern FILE *fp;
extern time_t rawtime;
extern struct tm *timeinfo;
extern pthread_mutex_t lock;
extern int successful_guesses;

 
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


// Helper function to acknowledge that the receipient has received our message.
int
acknowledge_sent(int threadsocketfd){
    char message[BUFFERSIZE+1];
    memset(message, '\0', BUFFERSIZE+1);
    
    if(recv(threadsocketfd, message, BUFFERSIZE+1, 0) < 0){
        socket_error("Error reading from the socket", threadsocketfd);
    }
    
    if(strstr(message, RECEIVED) != NULL){
        // Client received message
        return 1;
    }
    
    return 0; 
}


// Helper function to acknowledge that we have received a message.
int
acknowledge_received(int threadsocketfd){
    if(send(threadsocketfd, RECEIVED, strlen(RECEIVED), 0) < 0){
        socket_error("Error writing to the socket", threadsocketfd);        
    }
    
    return 1;      
}


// Helper function to print error (with errno), close socket and exit thread.
void
socket_error(String error_message, int threadsocketfd){
    perror(error_message);
    close(threadsocketfd);
    pthread_exit(NULL);    
}



// Helper function to increment the number of sucessful client connections.
void
client_guess_successful(){
    // Lock the mutex
    pthread_mutex_lock(&lock);  
    
    // Increase the count by 1
    successful_guesses += 1;
    
    // Unlock the mutex
    pthread_mutex_unlock(&lock);
}


// To avoid repetitive log entry writes per entry attempt, we use one 
// function which locks the logfile, writes all the information necessary,
// and unlocks. If there is a field that does not need to be written 
// (i.e. socket for server), the respective argument is NULL;
void
add_log_entry(String ip_address, int* socket_number, String message){
    // Lock the mutex
    pthread_mutex_lock(&lock);    
    
    // Get the current date and time and write to the log file
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    // Since asctime includes a newline, we remove it by filling up a
    // new string with NULL bytes and subsequently copying only the 
    // "time bits" over without the newline.
    int time_len = strlen(asctime(timeinfo)) - 1;
    char current_time[time_len+1];
    
    memset(current_time, '\0', time_len+1);
    memcpy(current_time, asctime(timeinfo), time_len);
    
    fprintf(fp, "[%s]", current_time);
        
    // Write the IP Address to the log file
    fprintf(fp, "(%s)", ip_address);
    
    // Write the socket number to the log file
    if(socket_number != NULL){
        fprintf(fp, "(socket id: %d)", *socket_number);
    }
    
    // Write the message to the log file
    fprintf(fp, "%s\n", message);   
    
    // Flush this log entry from the buffer to the file
    fflush(fp); 
    
    // Unlock the mutex
    pthread_mutex_unlock(&lock);
}

