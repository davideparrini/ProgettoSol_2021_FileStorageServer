#ifndef MYHASHSTORAGEFILE_H_
#define MYHASHSTORAGEFILE_H_


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#define NUMERO_PRIMO_ENORME 93199

typedef struct node{
	struct timespec creation_time;
	char* filename;
	long dim_bytes;
	int modified_flag;
	int inCache_flag;
	struct node *next;
    struct node *prec;
}file_t;

typedef struct _list{
	int size;
	file_t *head;
	file_t *tail;
}list;

typedef struct _hash{
	int len;
	long memory_used;
    long memory_capacity;
	int n_file;
	int n_file_modified;
	int max_n_file;
    int max_size_last_cell;
	list *cell;

}hashtable;



//inizializzazioni server
file_t* init_file(char *namefile);
void init_hash(hashtable *table, config s);

//funzione hash
int hash(hashtable table,char *namefile);

//operzioni su liste
void ins_tail_list(list *cell,file_t *file);
void ins_head_list(list *cell, file_t*file);
file_t* pop_list(list *cell);

//insermenti file nel server
void ins_file_cache(hashtable *table,file_t* file); 
void ins_file_hashtable(hashtable *table, file_t* file);
//ins e rimozione file server seguendo politica lru
void ins_file_server(hashtable* storage, char* namefile);
void remove_file_server(hashtable* table, char* namefile);
//estrazione di file
void extract_file_to_server(hashtable *table,file_t* file);
void extract_file(list* cell,file_t* file);
//estrae il file dalla sua posizione attuale e lo mette in cima alla cache
void update_file(hashtable *table,file_t* file);

//ricerca file
file_t* research_file_list(list cell,char* namefile);
file_t* research_file(hashtable table,char* namefile);
//utilità
int isContains_hash(hashtable table, file_t* file);
int isEmpty(list cella);
int isCacheFull(hashtable table);
void free_file(file_t* file);
void print_storageServer(hashtable table);





#endif