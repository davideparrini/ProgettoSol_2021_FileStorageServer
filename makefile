

CC = gcc
C_FLAGS = -Wall -pedantic -std=c99 -g -fPIC
THREAD_FLAGS = -lpthread 
INCLUDES = -I./headers
OBJS = ./objects

CLIENT_DEPS = sources/client.c libs/libapi.so libs/libclient.so

SERVER_DEPS = sources/serves.c libs/libserver.so


.PHONY : all, cleanall, test1, test2
.SUFFIXES: .c .h

$(OBJS)/%.o: sources/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<


libs/libapi.so: $(OBJS)/utils.o 
	$(CC) -shared -o libs/libapi.so $^

libs/libclient.so: $(OBJS)/utils $(OBJS)/serverAPI.O
	$(CC) -shared -o libs/libclient.so $^


all: 
	make -B
