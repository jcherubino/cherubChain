CC = gcc
CFLAGS = -Wall

BUILD_DIR = ./build
INC_DIR = ./include
SRC_DIR = ./src

PROG = cherub_chain

.PHONY: clean default

default: $(PROG)

prog.o: src/cherub_chain.c include/cherub_chain.h
	mkdir -p build
	$(CC) $(CFLAGS) -c src/cherub_chain.c -o build/prog.o

cherub_chain: prog.o
	$(CC) $(CFLAGS) build/prog.o -o cherub_chain 

clean:
	rm -rf build
	rm ./cherub_chain
