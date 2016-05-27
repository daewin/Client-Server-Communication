#
# Makefile for Project 2
# Subject: COMP30023 Computer Systems
# Author: Daewin SV Lingam
# Student ID: 679182 (dsv)
# Date Modified: 26/5/2016
#


## CC  = Compiler.
## CFLAGS = Compiler flags.
CC	= gcc
CFLAGS 	= -Wall -std=c99 -pedantic

## Server Make
## OBJ = Object files.
## SRC = Source files.
## EXE = Executable name.

SRCS =		linked_list.c game.c helper.c server.c
OBJS =		linked_list.o game.o helper.o server.o
EXES = 		server

## Top level target is executable.
$(EXES):	$(OBJS)
		$(CC) $(CFLAGS) -o $(EXES) $(OBJS) -lm -pthread

## Client Make
## OBJ = Object files.
## SRC = Source files.
## EXE = Executable name.

SRCC =		client.c
OBJC =		client.o
EXEC = 		client

## Top level target is executable.
$(EXEC):	$(OBJC)
		$(CC) $(CFLAGS) -o $(EXEC) $(OBJC) -lm -pthread



## Clean: Remove object files and core dump files.
clean:
		/bin/rm $(OBJS)
		/bin/rm $(OBJC)

## Clobber: Performs Clean and removes executable file.
clobber: clean
		/bin/rm $(EXES)
		/bin/rm $(EXEC) 




## Dependencies
linked_list.o: linked_list.h
game.o: game.h linked_list.h
helper.o: helper.h game.h
server.o: server.h helper.h
client.o: client.h