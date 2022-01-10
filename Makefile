CC = gcc
CFLAGS = -Wall -Werror -Iinclude/

.PHONY: clean

all: node chain add_block

node: node.o block.o server.o endpoints.o requests.o
	mkdir -p bin
	$(CC) $(CFLAGS) build/node.o build/block.o build/server.o \
		build/endpoints.o build/requests.o -o bin/node 

chain: chain.o block.o server.o requests.o
	mkdir -p bin
	$(CC) $(CFLAGS) build/chain.o build/block.o build/server.o \
		build/requests.o -o bin/chain

add_block: add_block.o block.o server.o requests.o
	mkdir -p bin
	$(CC) $(CFLAGS) build/add_block.o build/block.o build/server.o\
		build/requests.o -o bin/add_block

node.o: src/node.c 
	mkdir -p build
	$(CC) $(CFLAGS) -c src/node.c -o build/node.o

chain.o: src/chain.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/chain.c -o build/chain.o

add_block.o: src/add_block.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/add_block.c -o build/add_block.o

requests.o: src/requests.c include/requests.h
	mkdir -p build
	$(CC) $(CFLAGS) -c src/requests.c -o build/requests.o

block.o: src/block.c include/block.h
	mkdir -p build
	$(CC) $(CFLAGS) -c src/block.c -o build/block.o

server.o: src/server.c include/server.h
	mkdir -p build
	$(CC) $(CFLAGS) -c src/server.c -o build/server.o

endpoints.o: src/endpoints.c include/endpoints.h
	mkdir -p build
	$(CC) $(CFLAGS) -c src/endpoints.c -o build/endpoints.o

clean:
	rm -rf build
	rm -rf bin
