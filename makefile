CC = gcc
CFLAGS = -Wall -pedantic  -g 
THREAD_FLAGS = -lpthread 
INCLUDES = -I ./headers
OBJS = ./objects
LIBS = ./libs
SRC = ./sources

CLIENT_DEPS = $(SRC)/client.c $(LIBS)/libapi.so $(LIBS)/libclient.so
SERVER_DEPS =  $(SRC)/server.c $(LIBS)/libserver.so

TARGETS =  client server  

.PHONY : all, cleanall, test1, test2
.SUFFIXES: .c .h

all: $(TARGETS)

server: $(SERVER_DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC)/server.c -o server -Wl,-rpath,$(LIBS) -L $(LIBS) -lserver $(THREAD_FLAGS) 

client: $(CLIENT_DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRC)/client.c -o client -Wl,-rpath,$(LIBS) -L $(LIBS) -lclient -lapi $(THREAD_FLAGS)


$(LIBS)/libapi.so: $(OBJS)/utils.o
	$(CC) $(CFLAGS) -shared -o $@ $^

$(LIBS)/libclient.so: $(OBJS)/utils.o $(OBJS)/serverAPI.o $(OBJS)/myqueueopt.o
	$(CC) $(CFLAGS) -shared -o $@ $^

$(LIBS)/libserver.so: $(OBJS)/utils.o $(OBJS)/myhashstoragefile.o $(OBJS)/myqueueconnections.o $(OBJS)/request.o
	$(CC) $(CFLAGS) -shared -o $@ $^

$(OBJS)/myqueueconnections.o : $(SRC)/myqueueconnections.c 
	$(CC) $(C_FLAGS) $(INCLUDES)  $^ -c -fPIC -o $@

$(OBJS)/myqueueopt.o : $(SRC)/myqueueopt.c 
	$(CC) $(C_FLAGS) $(INCLUDES)  $^ -c -fPIC -o $@

$(OBJS)/request.o : $(SRC)/request.c 
	$(CC) $(C_FLAGS) $(INCLUDES)  $^ -c -fPIC -o $@

$(OBJS)/myhashstoragefile.o : $(SRC)/myhashstoragefile.c 
	$(CC) $(C_FLAGS) $(INCLUDES)  $^ -c -fPIC -o $@

$(OBJS)/serverAPI.o : $(SRC)/serverAPI.c 
	$(CC) $(C_FLAGS) $(INCLUDES)  $^ -c -fPIC -o $@

$(OBJS)/utils.o : $(SRC)/utils.c 
	$(CC) $(C_FLAGS) $(INCLUDES)  $^ -c -fPIC -o $@

cleanall: 	
	@echo "Removing garbage"  
	-rm -f read_files/*
	-rm -f rejected_files/*
	-rm -f $(OBJS)/*.o
	-rm -f $(LIBS)/*.so
	-rm -f ./logs/*
	-rm -f ./test_fileToSave/test_D/*
	-rm -f ./test_fileToSave/test-d/*
	-rm -f ./test/*.txt
	-rm -f ./test/test1/*.txt
	-rm -f ./test/test2/*.txt
	-rm -f /tmp/server_sock
	-rm -f $(TARGETS)

test1: client server
	./scripts/test1.sh

test2: client server
	./scripts/test2.sh

	