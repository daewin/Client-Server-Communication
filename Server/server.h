/********************************************************
 * Server-Side using TCP for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182 (dsv)
 * Date Modified: 22/5/2016
 ********************************************************/
 
#include "helper.h"

#define MAXCLIENTS 20
#define LOGMESSAGESIZE 64   // Very generous as we have short log messages

// Prototypes
void *worker_function(void* args);
void sighandler(int signum);


struct workerArgs{
    int socket;
    String secret_code;
    String client_ip;
};

const String RECEIVED = "RECEIVED";
const String LOGFILE = "log.txt";
const String MOCKSERVERIP = "0.0.0.0";
const String INTRODUCTION = "\nWelcome! Mastermind is a code-breaking game for two players consisting of the\n"
"'codemaker' and the 'codebreaker'. You are the codebreaker. There are six possible\n"
"colours that you can choose from to form a sequence of four colours. In this case,\n"
"the colours are A, B, C, D, E and F. Your guess would result in four outcomes: namely\n"
"a success if you have guessed it right; a failure if you have exceeded the number of\n"
"attempts available; a feedback, in the form of [b:m] where 'b' and 'm' are described\n"
"below; and an invalid for invalid inputs.\n\n    b: the number of correct colours in the "
"correct position.\n    m: the number of colours that are part of the code but not in the "
"correct position.\n\n";