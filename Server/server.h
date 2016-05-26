/********************************************************
 * Server-Side using TCP for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 22/5/2016
 ********************************************************/
 
#include "helper.h"

#define MAXCLIENTS 20
#define LOGMESSAGESIZE 64   // Very generous as we have short log messages

// Prototypes
void *worker_function(void* args);


struct workerArgs{
    int socket;
    String secret_code;
    String client_ip;
};


const String LOGFILE = "log.txt";
const String MOCKSERVERIP = "0.0.0.0";