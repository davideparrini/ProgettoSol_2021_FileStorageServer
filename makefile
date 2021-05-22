CC = gcc
C_FLAGS = -Wall -pedantic -std=c99 -g -lpthread -fPIC

INCLUDES = -I./headers

OBJS = ./objects

LIBS = 




all: client server


server:


client:

clear:

$(OBJS)/myqueue.o :
	$(CC) $(C_FLAGS) $(INCLUDES)  sources/myqueue.c -c -o $@