#ifndef MYHASHSTORAGEFILE_H_
#define MYHASHSTORAGEFILE_H_


#include "utils.h"

#define NUMERO_PRIMO_ENORME 93199

typedef struct node{
	int fd;
	char* abs_path;
	void* content;
	size_t dim_bytes;
	int modified_flag;
	int open_flag;
	int o_create_flag;
	int locked_flag;
	int inCache_flag;
	struct node *next;
    struct node *prec;
}file_t;
//struttura per mantenere il riferimento al file vero e proprio, 
//avendo comunque la possibilità di metterlo in una struttura dati secondaria
typedef struct dupf{
	file_t* riferimento_file;
	struct dupf *next;
}dupFile_t;

typedef struct temp_list{
	dupFile_t *head;
	int size;
}dupFile_list;

typedef struct _list{
	int size;
	size_t dim_bytes;
	file_t *head;
	file_t *tail;
}list;

typedef struct _hash{
	int len;
	size_t memory_used;
	size_t memory_used_from_modified_files;
    size_t memory_capacity;
	int n_file;
	int n_files_free;
	int n_file_modified;
	int max_n_file;
    int max_size_cache;

	int stat_max_n_file;
	size_t stat_dim_file;
	int stat_n_replacing_algoritm;
	
	list *cell;
	list *cache;
}hashtable;



//inizializzazioni server
file_t* init_file(char *namefile);
int writeContentFile(file_t* f);
void appendContent(file_t * f,void *buff,size_t size);
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
int init_file_inServer(hashtable* table,file_t *f,list* list_reject);
int ins_file_server(hashtable* storage, file_t *f,list* list_reject);
file_t* remove_file_server(hashtable* table, file_t* f);
//estrazione di file
void extract_file_to_server(hashtable *table,file_t* file);
void extract_file(list* cell,file_t* file);
//estrae il file dalla sua posizione attuale e lo mette in cima alla cache
void update_file(hashtable *table,file_t* file);
//modifica il file,mettendolo nella cache
int modifying_file(hashtable* table,file_t* f,size_t size_inplus,list* list_reject);
//ricerca file
file_t* research_file_list(list cell,char* namefile);
file_t* research_file(hashtable table,char* namefile);
//utilità
int isContains_hash(hashtable table, file_t* file);
int isEmpty(list cella);
int isCacheFull(hashtable table);
void free_file(file_t* file);
void free_list(list* l);
void free_hash(hashtable* table);

void print_storageServer(hashtable table);
list* concatList(list *l,list *l2);


dupFile_t* init_dupFile(file_t* f);


void init_dupFile_list(dupFile_list* l);

void ins_head_dupFilelist(dupFile_list *l,dupFile_t *file);
dupFile_t* pop_dupFilelist(dupFile_list *cell);
void ins_dupList_to_list(hashtable* table, list* l, dupFile_list* dl);
void free_duplist(dupFile_list* dl);
int isEmpty_duplist(dupFile_list dl);


#endif