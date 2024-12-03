CC=gcc
CFLAGS=-Iinclude -Wall -Werror -g -Wno-unused

SSRC=$(shell find src -name '*.c')
DEPS=$(shell find include -name '*.h')

LIBS=-lpthread

all: setup server 

setup:
	mkdir -p bin 
	cp lib/zotPoll_client bin/zotPoll_client

server: setup
	$(CC) $(CFLAGS) $(SSRC) lib/protocol.o -o bin/zotPoll_server $(LIBS)
	
.PHONY: clean

clean:
	rm -rf bin 
