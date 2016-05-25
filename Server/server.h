/********************************************************
 * Server-Side using TCP for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 22/5/2016
 ********************************************************/
  
#include "game.h"

#define BUFFERSIZE 1024
#define MAXCLIENTS 20
#define LOGMESSAGESIZE 64   // Very generous as we have short log messages

// Prototypes
String itoa(int num);
void *worker_function(void* args);
int acknowledge_sent(int threadsocketfd);
int acknowledge_received(int threadsocketfd);
void socket_error(String error_message, int threadsocketfd);
void client_connection_successful();
void add_log_entry(String ip_address, void* socket_number, String message);


struct workerArgs{
    int socket;
    String secret_code;
    String client_ip;
};

const String RECEIVED = "RECEIVED";
const String LOGFILE = "log.txt";
const String MOCKSERVERIP = "0.0.0.0";

