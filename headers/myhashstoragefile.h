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
	char* abs_path;
	char* filename;
	long dim_bytes;
	int modified_flag;
	int opened_flag;
	int locked_flag;
	int inCache_flag;
	struct node *next;
    struct node *prec;
}file_t;
//struttura per mantenere il riferimento al file vero e proprio, 
//avendo comunque la possibilità di metterlo in una struttura dati secondaria
typedef struct f{
	file_t* riferimento_file;
	struct f *next;
}dupFile_t;

typedef struct temp_list{
	dupFile_t *head;
	int size;
}dupFile_list;

typedef struct _list{
	int size;
	long dim_bytes;
	file_t *head;
	file_t *tail;
}list;

typedef struct _hash{
	int len;
	long memory_used;
	long memory_used_from_modified_files;
    long memory_capacity;
	int n_file;
	int n_file_modified;
	int max_n_file;
    int max_size_cache;

	list *cell;
	list *cache;
}hashtable;



//inizializzazioni server
file_t* init_file(char *namefile);
void init_list(list* l);
void init_hash(hashtable *table, config s);

//funzione hash
int hash(hashtable table,char *namefile);

//operzioni su liste
void ins_tail_list(list *cell,file_t *file);
void ins_head_list(list *cell, file_t*file);
file_t* pop_list(list *cell);

//insermento file nel server
void ins_file_cache(hashtable *table,file_t* file); 
void ins_file_hashtable(hashtable *table, file_t* file);
//ins e rimozione file server seguendo politica lru
int ins_file_server(hashtable* storage, char* namefile,list* list_reject);
file_t* remove_file_server(hashtable* table, char* namefile);
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
void free_list(list* l);
void print_storageServer(hashtable table);
list* concatList(list *l,list *l2);
void cleanList(list *l);

dupFile_t* init_dupFile(file_t* f);


void init_dupFile_list(dupFile_list* l);

void ins_head_dupFilelist(dupFile_list *l,dupFile_t *file);
dupFile_t* pop_dupFilelist(dupFile_list *cell);
void ins_dupList_to_list(hashtable* table, list* l, dupFile_list* dl);

#endif