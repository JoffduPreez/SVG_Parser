UNAME := $(shell uname)
ENVIRONMENT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
CC = gcc
CFLAGS = -Wall -std=c11 -g
LDFLAGS= -L.

INC = include/
SRC = src/
BIN = bin/
PARSER_SRC_FILES = $(wildcard src/SVG*.c)
PARSER_OBJ_FILES = $(patsubst src/SVG%.c,bin/SVG%.o,$(PARSER_SRC_FILES))
SHARED_LIB_FILES = $(patsubst bin/SVG%.o,parser/bin/SVG%.o,$(PARSER_OBJ_FILES))

ifeq ($(UNAME), Linux)
	XML_PATH = /usr/include/libxml2
endif
ifeq ($(UNAME), Darwin)
	XML_PATH = /System/Volumes/Data/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/libxml2
endif

parser: libsvgparser.so main 

libsvgparser.so: $(PARSER_OBJ_FILES) $(BIN)LinkedListAPI.o
	cd .. && gcc -shared -o libsvgparser.so $(SHARED_LIB_FILES) parser/$(BIN)LinkedListAPI.o -lxml2 -lm && cd parser

#Compiles all files named SVG*.c in src/ into object files, places all corresponding SVG*.o files in bin/
$(BIN)SVG%.o: $(SRC)SVG%.c $(INC)LinkedListAPI.h $(INC)SVG*.h
	$(CC) $(CFLAGS) -I$(XML_PATH) -I$(INC) -c -fpic $< -o $@

$(BIN)liblist.so: $(BIN)LinkedListAPI.o
	$(CC) -shared -o $(BIN)liblist.so $(BIN)LinkedListAPI.o

$(BIN)LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic -I$(INC) $(SRC)LinkedListAPI.c -o $(BIN)LinkedListAPI.o

clean:
	rm -rf $(BIN)main $(BIN)*.so $(BIN)*.o  && cd .. && rm $(ENVIRONMENT)/*.so

#These are sample targets for the list demo code included in the class examples.  They will not be used
#for A1, but they can help you figure out who to set up a target for your own test main

main: $(BIN)main.o $(BIN)liblist.so
	$(CC) $(CFLAGS) -L$(ENVIRONMENT) $(BIN)main.o -lxml2  -lsvgparser -o main
	
$(BIN)main.o: $(SRC)main.c
	$(CC) $(CFLAGS) -I$(XML_PATH) -I$(INC) -c $(SRC)main.c -o $(BIN)main.o

###################################################################################################