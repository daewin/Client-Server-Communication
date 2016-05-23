/********************************************************
 * Client-Side using TCP for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 22/5/2016
 ********************************************************/

#define SECRETCODELENGTH 4
#define BUFFERSIZE 1024

typedef char* String;

static const String SUCCESS = "SUCCESS";
static const String FAILURE = "FAILURE";
static const String INVALID = "INVALID";
static const String RECEIVED = "RECEIVED";


int acknowledge_sent(int socketfd);
int acknowledge_received(int socketfd);
