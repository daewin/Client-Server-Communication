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

String itoa(int num);
void *worker_function(void* args);
int acknowledge_sent(int threadsocketfd);
int acknowledge_received(int threadsocketfd);

struct workerArgs{
    int socket;
    String secret_code;
};

static const String RECEIVED = "RECEIVED";

