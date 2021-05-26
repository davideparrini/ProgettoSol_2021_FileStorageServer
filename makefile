CC = gcc
C_FLAGS = -Wall -pedantic -std=c99 -g -lpthread -fPIC

INCLUDES = -I./headers

OBJS = ./objects








$(OBJS)/myqueue.o :
	$(CC) $(C_FLAGS) $(INCLUDES)  sources/myqueue.c -c -o $@

$(OBJS)/myhashstoragefile.o :
	$(CC) $(C_FLAGS) $(INCLUDES)  sources/myhashstoragefile.c -c -o $@

$(OBJS)/serverAPI.o :
	$(CC) $(C_FLAGS) $(INCLUDES)  sources/serverAPI.c -c -o $@
$(OBJS)/utils.o :
	$(CC) $(C_FLAGS) $(INCLUDES)  sources/utils.c -c -o $@
