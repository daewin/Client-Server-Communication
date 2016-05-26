/********************************************************
 * Server-Side Helper Functions for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 26/5/2016
 ********************************************************/
 
#include "game.h"

#define BUFFERSIZE 1024

// Prototypes
String itoa(int num);
int acknowledge_sent(int threadsocketfd);
int acknowledge_received(int threadsocketfd);
void socket_error_handler(String error_message, int threadsocketfd);
void client_guess_successful();
void add_log_entry(String ip_address, int* socket_number, String message);


