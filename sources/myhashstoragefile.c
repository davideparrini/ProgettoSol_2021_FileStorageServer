#include "myhashstoragefile.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>



void init_hash(hashtable *table,size_t a,size_t b, int n_file){
	//inizializzazione hash
	table->len = n_file*2+1;
    table->max_size_last_cell = 10;
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

void append_list(list *cell,char *file){
	//inserimento in coda della lista
	file_t* new = malloc(sizeof(file_t));
	new->filename = malloc(sizeof(char)*strlen(file)+1);
	new->next = NULL;
	if (cell->size == 0){
		cell->head = new;
	}
	else{
        new->prec = cell->tail;
		cell->tail->next = new;
	}
	cell->tail  = new;
	cell->size++;
}

void insert_list(list *cell, char *file){
	//inserimento in testa della lista
    file_t* new = malloc(sizeof(file_t));
	new->filename = malloc(sizeof(char)*strlen(file)+1);
	strcpy(new->filename,file);
    new->next = cell->head;
    cell->head->prec = new;
    cell->head = new;
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
void print_hash(hashtable table){
	//stampa hash
	for (int i=0; i< table.len; i++){
		printf("%d -> ",i);
		if(table.cell[i].head != NULL){
			size_t n = table.cell[i].size;
			list temp = table.cell[i];
			while(n > 0){
				printf("PATH FILE: %s -> ",temp.head->filename);	
				temp.head = temp.head->next;
				n--;
			}
		}
		printf("NULL\n");
	}
}
int isContains_hash(hashtable table, char* namefile){
	// la hash contiene il file? Si, return 1. No, return 0;
	size_t h,trovato = 0;
	h = hash(table,namefile);
	list temp = table.cell[h];
	while (temp.head != NULL && !trovato){
		if (!strcmp(namefile,temp.head->filename)) {
			 trovato = 1;
		}
		else temp.head = temp.head->next;
	}
	return trovato;
}
	
void ins_hashtable(hashtable *table, char* file){
	if( !isContains(*table,file)){
        insert_list(&table->cell[table->len],file);
        if(table->cell[table->len].size > table->max_size_last_cell){
            file_t* file = pop_list(&table->cell[table->len]);
            size_t h = hash(*table,file->filename);
            append_list(&table->cell[h],file);	
        }
    }     
}

file_t* extract_file(list* cell,char* namefile){
	//cerca il file e lo estrare dalla cella della hash ridandolo come risultato
	list* temp = cell;
	file_t* res;
	while(temp->head != NULL){
		if (!strcmp(temp->head->filename,namefile) ){
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


void update_hash(hashtable *table,file_t* file){
	/*	funzione per aggiornare la posizione dei file

	*/
    file_t* f;
	if( (f = extract_file(&table->cell[table->len],file->filename)) != NULL){
		//se è nell'ultima lista della hash, mette il file in testa alla coda

        insert_list(&table->cell[table->len],file);
		return;
    }
	else{
		//altrimenti, estraggo il file dalla hash "generale", ed inserico il file in cima alla
		//l'ultima lista della hash ed estaggo il file meno recentemente modificato della lista
		//nella hash "generale"
		int h = hash(*table,file->filename);
		if( (f = extract_file(&table->cell[h],file->filename)) != NULL){
			insert_list(&table->cell[table->len],file->filename);
			file_t* file_to_realloc = pop_list(&table->cell[table->len]);
			h  = hash(*table,file_to_realloc->filename);
			append_list(&table->cell[h],file_to_realloc->filename);
		}		
	}
}

file_t* extract_file_to_server(hashtable *table,file_t* file){
	file_t* f;
	if( (f = extract_file(&table->cell[table->len],file->filename)) != NULL){
		//se è nell'ultima lista della hash, mette il file in testa alla coda
		return f;
    }
	else{
		//altrimenti, estraggo il file dalla hash "generale", ed inserico il file in cima alla
		//l'ultima lista della hash ed estaggo il file meno recentemente modificato della lista
		//nella hash "generale"
		int h = hash(*table,file->filename);
		if( (f = extract_file(&table->cell[h],file->filename)) != NULL){
			return f;
		}		
	}
	return NULL;

}