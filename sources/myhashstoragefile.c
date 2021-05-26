#include "myhashstoragefile.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>

#include <utils.h>

file_t* init_file(char *namefile){
	file_t* new = malloc(sizeof(file_t));
	new->filename = malloc(sizeof(char)*strlen(namefile)+1);
	strncpy(new->filename,namefile,strlen(namefile));
	new->modified_flag = 0;
	new->next = NULL;
	new->prec = NULL;
	struct stat s;
	if (stat(namefile, &s) == -1) {
        perror("stat creazione file\n");
        exit(EXIT_FAILURE);
    }
	new->dim_bytes = s.st_size;
}

void init_hash(hashtable *table, config s){
	//inizializzazione hash
	table->len = s.max_n_file*2+1;
	table->memory_capacity = s.memory_capacity;
	table->memory_used = 0;
	table->n_file = 0;
	table->n_file_modified = 0;
	table->max_n_file = s.max_n_file;
	table->max_size_last_cell = s.max_n_file * 10 / 100;
	table->cell = malloc(table->len*sizeof(list));
	for (size_t i = 0;i < table->len; i++){
		table->cell->head = NULL;
		table->cell->tail = NULL;
		table->cell->size = 0;
	}
}

int hash(hashtable table,char *namefile){
	//funzione hash
	int len = strlen(namefile);
	int somma = 0;
	for (size_t i=0; i < len; i++){
		somma += namefile[i];
	}
	return somma * NUMERO_PRIMO_ENORME % (table.len-1);
}

void ins_tail_list(list *cell,file_t *file){
	//inserimento in coda della lista
	file->next = NULL; //per sicurezza annullo i puntatori next/prec del file
	file->prec = NULL;
	if (cell->size == 0){
		cell->head = file;
	}
	else{
        file->prec = cell->tail;
		cell->tail->next = file;
	}
	cell->tail  = file;
	cell->size++;
}

void ins_head_list(list *cell,file_t *file){
	//inserimento in testa della lista
	file->next = NULL; //per sicurezza annullo i puntatori next/prec del file
	file->prec = NULL;
    file->next = cell->head;
    cell->head->prec = file;
    cell->head = file;
    cell->size++;
}

file_t* pop_list(list *cell){
	//rimouve e ritorna l'ultimo elemento della lista
    if(cell->head == NULL){
            return NULL; 
    }
    else{
		file_t *res = cell->tail;
		cell->tail = cell->tail->prec;
		cell->tail->next = NULL;
		res->prec = NULL;
		cell->size--;
		return res;
	}
}


	
void ins_file_hashtable(hashtable *table, file_t* file){
	size_t h = hash(*table,file->filename);
    ins_tail_list(&table->cell[h],file);   
	table->memory_used += file->dim_bytes;
	table->n_file++;
}

void ins_file_cache(hashtable *table,file_t* file){
	ins_head_list(&table->cell[table->len],file);
	table->memory_used += file->dim_bytes;
	table->n_file++;
}

void extract_file(list* cell,file_t* file){
	//questa funzione va utilizzata assumendo di sapere per certo che il file è nella cella passata della hash
	if(file->prec != NULL){
		if(file->next != NULL) {
			file->prec->next = file->next;
			file->next->prec = file->prec;
		}
		else {
			file->prec->next = NULL;
			cell->tail = file->prec;
		}	
	}
	else{
		if(file->next != NULL){
			cell->head = file->next;
			file->next->prec = NULL;
		}
		else{
			cell->head = NULL;
			cell->tail = NULL;			
		}	
	}
	file->next = NULL;
	file->prec = NULL;		
	cell->size--;
	return file;
}


file_t* extract_file_list(list* cell,file_t* file){
	//cerca il file e lo estrare dalla cella della hash ridandolo come risultato
	list* temp = cell;
	file_t* res;
	while(temp->head != NULL){
		if (!strcmp(temp->head->filename,file->filename) ){
			res = temp->head;
			
			if(temp->head->next != NULL){
				if(temp->head->prec != NULL) {
					temp->head->prec->next = temp->head->next;
					temp->head->next->prec = temp->head->prec;
				}
				else {	
					cell->head = temp->head->next;
					cell->tail = cell->head;
					temp->head->prec = NULL;
				}	
				
			}
			else{
				cell->head = NULL;
				cell->tail = NULL;			
			}
			res->next = NULL;
			res->prec = NULL;		
			return res;
		}
		else{
			temp->head = temp->head->next;
		}
	}
	return NULL;
}

file_t* extract_file_to_server(hashtable *table,file_t* file){
	file_t* f;
	if( (f = extract_file_list(&table->cell[table->len],file)) != NULL){
		//se è nell'ultima lista della hash, mette il file in testa alla coda
		table->memory_used -= f->dim_bytes;
		table->n_file_modified--;
		table->n_file--;
		return f;
    }
	else{
		//altrimenti, estraggo il file dalla hash "generale", ed inserico il file in cima alla
		//l'ultima lista della hash ed estaggo il file meno recentemente modificato della lista
		//nella hash "generale"
		int h = hash(*table,file->filename);
		if( (f = extract_file_list(&table->cell[h],file->filename)) != NULL){
			table->memory_used -= f->dim_bytes;
			return f;
		}		
	}
	return NULL;
}

void update_hash(hashtable *table,file_t* file){
	/*	funzione per aggiornare la posizione dei file
	*/
    file_t* f;
	//se è nell'ultima lista della hash, mette il file in testa alla coda
	if((f = extract_file_list(&table->cell[table->len],file)) !=  NULL){
		ins_head_list(&table->cell[table->len],file);
		return;
	}
	else{
		//altrimenti, estraggo il file dalla hash "generale", ed inserico il file in cima alla
		//l'ultima lista della hash ed estaggo il file meno recentemente modificato della lista
		//nella hash "generale"
		int h = hash(*table,file->filename);
		if( (f = extract_file_list(&table->cell[h],file)) != NULL){
			ins_head_list(&table->cell[table->len],file);
			file_t* file_to_realloc = pop_list(&table->cell[table->len]);
			h  = hash(*table,file_to_realloc->filename);
			ins_tail_list(&table->cell[h],file_to_realloc);
		}		
	}
}

int isEmpty(list cella){
	if(cella.head != NULL) return 0;
	else return 1; 
}
int isCacheFull(hashtable table){
	if(table.cell->size == table.max_size_last_cell) return 1;
	else return 0;
}
int isContains_list(list cell, file_t* file){
	list temp = cell;
	while (temp.head != NULL){
		if (!strcmp(file->filename,temp.head->filename)) {
			 return 1;
		}
		else temp.head = temp.head->next;
	}
	return 0;
}
int isContains_hash(hashtable table, file_t* file){
	// la hash contiene il file? Si, return 1. No, return 0;
	if(isContains_list(table.cell[table.len],file)) return 1;
	size_t h = hash(table,file->filename);
	return isContains_list(table.cell[h],file);
}
void free_file(file_t* file){
	free(file->filename);
	free(file);
}
void print_storageServer(hashtable table){
	//stampa hash
	for (int i=0; i <= table.len; i++){
		if(i != table.len) printf("%d -> ",i);
		else printf("Cache -> ");
		if(table.cell[i].head != NULL){
			size_t n = table.cell[i].size;
			list temp = table.cell[i];
			while(n > 0){
				printf("%s -> ",temp.head->filename);	
				temp.head = temp.head->next;
				n--;
			}
		}
		printf("NULL\n");
	}
}

