CC = gcc
CFLAGS = -Wall -Werror -Iinclude/

PROG = cherub_chain

.PHONY: clean default

default: cherub_chain

cherub_chain: cherub_chain.o block.o server.o
	$(CC) $(CFLAGS) build/cherub_chain.o build/block.o build/server.o -o cherub_chain 

client: client.o block.o server.o
	$(CC) $(CFLAGS) build/client.o build/block.o build/server.o -o client

client.o: src/client.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/client.c -o build/client.o

cherub_chain.o: src/cherub_chain.c 
	mkdir -p build
	$(CC) $(CFLAGS) -c src/cherub_chain.c -o build/cherub_chain.o

block.o: src/block.c include/block.h
	mkdir -p build
	$(CC) $(CFLAGS) -c src/block.c -o build/block.o

server.o: src/server.c include/server.h
	mkdir -p build
	$(CC) $(CFLAGS) -c src/server.c -o build/server.o

clean:
	rm -rf build
	rm ./cherub_chain
	rm ./client
