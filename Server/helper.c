/********************************************************
 * Server-Side Helper Functions for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182 (dsv)
 * Date Modified: 26/5/2016
 ********************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <pthread.h>
#include <sys/resource.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
 
#include "helper.h"

// Extern values from Server for our helper to use
extern FILE *fp;
extern time_t rawtime;
extern struct tm *timeinfo;
extern pthread_mutex_t lock;
extern int successful_guesses;
extern const String RECEIVED;
extern int successful_connections;
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
        socket_error_handler("Error reading from the socket", threadsocketfd);
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
        socket_error_handler("Error writing to the socket", threadsocketfd);        
    }
    
    return 1;      
}


// Helper function to print error (with errno), close socket and exit thread.
void
socket_error_handler(String error_message, int threadsocketfd){
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


// Helper function to print various resource usage statistics 
void
print_usage(){
    
    struct rusage ru;
    struct timeval utime;
    struct timeval stime;
    
    getrusage(RUSAGE_SELF, &ru);
    
    utime = ru.ru_utime;
    stime = ru.ru_stime;
    
    fprintf(fp, 
        "\n============= Performance and Usage Statistics =============\n\n");
    
    fprintf(fp, 
        "Number of successful connections: %d\n", successful_connections);
    fprintf(fp, "Number of successful guesses: %d\n", successful_guesses);
    
    fprintf(fp, "User CPU time used: %" PRId64 ".%" PRId64 " seconds\n",  
        (int64_t)utime.tv_sec, (int64_t)utime.tv_usec);
        
    fprintf(fp, "System CPU time used: %" PRId64 ".%" PRId64 " seconds\n", 
        (int64_t)stime.tv_sec, (int64_t)stime.tv_usec);
    
    
    fprintf(fp, "Maximum resident set size used: %ld kB\n", ru.ru_maxrss);
    fprintf(fp, "Page reclaims (soft page faults): %ld\n", ru.ru_minflt);
    fprintf(fp, "Page faults (hard page faults): %ld\n", ru.ru_majflt);
    fprintf(fp, 
        "Number of block output operations to log: %ld\n", ru.ru_oublock);
    fprintf(fp, 
        "Voluntary context switches between threads: %ld\n", ru.ru_nvcsw);
    fprintf(fp, 
        "Involuntary context switches between threads: %ld\n", ru.ru_nivcsw);
    
    fprintf(fp, 
        "\n============================================================\n");
    
    // Flush the statistics from the buffer to the file
    fflush(fp); 
    
}

