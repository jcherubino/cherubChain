CC = gcc
CFLAGS = -Wall -Werror -Iinclude/

PROG = cherub_chain

.PHONY: clean default

default: all

all: cherub_chain chain_endpoint

cherub_chain: cherub_chain.o block.o server.o endpoints.o
	$(CC) $(CFLAGS) build/cherub_chain.o build/block.o build/server.o build/endpoints.o -o cherub_chain 

chain_endpoint: chain_endpoint.o block.o server.o
	$(CC) $(CFLAGS) build/chain_endpoint.o build/block.o build/server.o -o chain_endpoint

cherub_chain.o: src/cherub_chain.c 
	mkdir -p build
	$(CC) $(CFLAGS) -c src/cherub_chain.c -o build/cherub_chain.o

chain_endpoint.o: src/chain_endpoint.c
	mkdir -p build
	$(CC) $(CFLAGS) -c src/chain_endpoint.c -o build/chain_endpoint.o

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
	rm ./cherub_chain
	rm ./chain_endpoint
