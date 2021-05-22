#ifdef MYHASHSTORAGEFILE_H_
#define MYHASHSTORAGEFILE_H_


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#define NUMERO_PRIMO_ENORME 93199

typedef struct node{
	char* filename;
	struct node *next;
    struct node *prec;
}file_t;

typedef struct _list{
	int size;
	file_t *head;
	file_t *tail;
}list;

typedef struct _hash{
    int n_file;
	int len;
    long max_capacity;
    int max_size_last_cell;
	list *cell;

}hashtable;





void init_hash(hashtable *table,size_t a,size_t b, int n_file);
int hash(hashtable table,char *namefile);
void append_list(list *cell,char *file);
void insert_list(list *cell, char *file);
file_t* pop_list(list *cell);
void print_hash(hashtable table);
int isContains_hash(hashtable table, char* namefile);
	
void ins_hashtable(hashtable *table, char* file);
file_t* extract_file(list* cell,char* namefile);


void update_hash(hashtable *table,file_t* file);
file_t* extract_file_to_server(hashtable *table,file_t* file);









#endif