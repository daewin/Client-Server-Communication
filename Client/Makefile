#
# Client Makefile for Project 2
# Subject: COMP30023 Computer Systems
# Author: Daewin SV Lingam
# Student ID: 679182 (dsv)
# Date Modified: 26/5/2016
#



## CC  = Compiler.
## CFLAGS = Compiler flags.
CC	= gcc
CFLAGS 	= -Wall -std=c99 -pedantic


## OBJ = Object files.
## SRC = Source files.
## EXE = Executable name.

SRC =		client.c
OBJ =		client.o
EXE = 		client

## Top level target is executable.
$(EXE):	$(OBJ)
		$(CC) $(CFLAGS) -o $(EXE) $(OBJ) -lm


## Clean: Remove object files and core dump files.
clean:
		/bin/rm $(OBJ) 

## Clobber: Performs Clean and removes executable file.

clobber: clean
		/bin/rm $(EXE) 

## Dependencies

##driver.o:	prime.h
##prime.o:	prime.h

client.o: client.h