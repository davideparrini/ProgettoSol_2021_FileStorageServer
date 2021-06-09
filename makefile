CC = gcc
CFLAGS = -Wall -pedantic -std=c99 -g -fPIC
THREAD_FLAGS = -lpthread 
INCLUDES = -I./headers
OBJS = ./objects
LIBS = ./libs
SRC = ./sources

CLIENT_DEPS = $(SRC)/client.c $(LIBS)/libapi.so $(LIBS)/libclient.so
SERVER_DEPS =  $(SRC)/serves.c $(LIBS)/libserver.so


.PHONY : all, cleanall
.SUFFIXES: .c .h

$(OBJS)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<


$(LIBS)/libapi.so: $(OBJS)/utils.o 
	$(CC) -shared -o $(LIBS)/libapi.so $^

$(LIBS)/libclient.so: $(OBJS)/utils $(OBJS)/serverAPI.O
	$(CC) -shared -o $(LIBS)/libclient.so $^

server: $(SERVER_DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) server.c -o server -Wl,-rpath,$(LIBS) -L ./libs -lserv -lapi $(THREAD_FLAGS) 

client: $(SERVER_DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) client.c -o client -Wl,-rpath,$(LIBS) -L ./libs -lserv -lapi $(THREAD_FLAGS)

all: server client

cleanall: 	
	-rm -f read_files/*
	-rm -f rejected_files/*
	-rm -f $(OBJS)/*.o
	-rm -f $(LIBS)/*.so
	-rm -f ./logs/*
	-rm /tmp/server_sock