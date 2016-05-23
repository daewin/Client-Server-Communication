/********************************************************
 * Game for Project 2
 * Subject: COMP30023 Computer Systems
 * Author: Daewin SV Lingam
 * Student ID: 679182
 * Date Modified: 21/5/2016
 ********************************************************/
 
#include "linked_list.h"

#define MAXGUESSES 10
#define SECRETCODELENGTH 4
#define FEEDBACKLENGTH 10

static const String colours = "ABCDEF";

// Declarations
String codemaker_generate_code();
String codebreaker();
String char_to_string(char character);
String codemaker_provide_feedback(String secret_code, String input_code);
int is_code_invalid(String secret_code, int is_server);