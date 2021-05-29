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
	new->abs_path= malloc(sizeof(char)*NAME_MAX);
	memset(new->abs_path,0,sizeof(new->abs_path));
	MY_REALPATH(init_file,namefile,new->abs_path);
	new->modified_flag = 0;
	new->opened_flag = 0;
	new->locked_flag = 0;
	new->inCache_flag = 0;
	new->next = NULL;
	new->prec = NULL;
	struct stat s;
	if (stat(new->abs_path, &s) == -1) {
        perror("stat creazione file\n");
        exit(EXIT_FAILURE);
    }
	new->dim_bytes = s.st_size;
	new->creation_time = s.st_atim;
	return new;	
}
void init_list(list* l){
	l->head = NULL;
	l->tail = NULL;
	l->size = 0;
}
void init_hash(hashtable *table, config s){
	//inizializzazione hash
	table->len = s.max_n_file*2+1;
	table->memory_capacity = s.memory_capacity;
	table->memory_used = 0;
	table->n_file = 0;
	table->n_file_modified = 0;
	table->max_n_file = s.max_n_file;
	table->max_size_cache = s.max_n_file * 10 / 100;
	table->cell = malloc(table->len*sizeof(list));
	table->cache = malloc(sizeof(file_t)*table->max_size_cache);
	for (size_t i = 0;i < table->len; i++){
		init_list(&table->cell[i]);
	}
	table->cache = &table->cell[table->len];
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
	size_t h = hash(*table,file->abs_path);
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
}


void extract_file_to_server(hashtable *table,file_t* file){
	
	if(file->inCache_flag){
		extract_file(&table->cache,file);
    }
	else{	
		int h = hash(*table,file->abs_path);
		extract_file(&table->cell[h],file);
	}
	table->memory_used -= file->dim_bytes;
	if(file->modified_flag){
		table->n_file_modified--;
		file->modified_flag = 0;
	}
	table->n_file--;
	file->inCache_flag = 0;
	

}

void update_file(hashtable *table,file_t* file){
	/*	funzione per aggiornare la posizione dei file
	*/
    if(file->inCache_flag){
		extract_file(&table->cache,file);
		ins_head_list(&table->cache,file);
	}
	else{
		int h;
		if(isCacheFull){
			file_t *f = pop_list(&table->cache);
			f->inCache_flag = 0;
			h = hash(*table,f->abs_path);
			ins_tail_list(&table->cell[h],file);
		}
		h = hash(*table,file->abs_path);
		extract_file(&table->cell[h],file);
		ins_head_list(&table->cell[table->len],file);
		file->modified_flag = 1;
		file->inCache_flag = 1;		
	}
}

file_t* research_file(hashtable table,char *namefile){
	char abs_path[NAME_MAX];
	MY_REALPATH(research_file,namefile,abs_path);
	int h = hash(table,abs_path);
	file_t* f;
	if(f = research_file_list(table.cell[h],abs_path) != NULL){
		return f;
	}
	else{
		if(f = research_file_list(*table.cache,abs_path) != NULL){
			return f;
		}
	}
	return NULL;
}

file_t* research_file_list(list cell,char* namefile){
	file_t* f;
	list temp = cell;
	while(temp.head != NULL){
		if(!strncmp(temp.head->abs_path,namefile,strlen(namefile))){
			f = &temp.head;
			return f;
		}
		else temp.head = temp.head->next;
	}
	return NULL;
}






int ins_file_server(hashtable* table, char* namefile,list* list_reject){
    file_t *f = init_file(namefile);
    if(table->n_file < table->max_n_file && (table->memory_used + f->dim_bytes) <= table->memory_capacity){
        ins_file_hashtable(table,f);
    }
    else{
        if(table->n_file_modified == 0){
            printf("Server pieno e non ci sono file \
                    modificati nel server\nImpossibile inserire file\n");  
            free_file(f);
			return 0;
        }
        else{
            if(table->n_file < table->max_n_file){
                //quindi non ha memoria
                //lavoro sulla memoria e di conseguenza anche sui posti
                if(table->n_file_modified > table->max_size_cache){
                    //se il numero dei file modificati è maggiore della capacità della cache,
                    // allora cerco il primo file modificato da rimuovere

                    //ricerca file da rimpiazzare
                    int stop = 0;
                    int h = hash(*table,f->abs_path);
                    file_t* to_reject;
                    
                    long somma_bytes; //somma bytes della list_reject
                    while(!stop){
                        list* temp = table->cell[h].head;
                        while(temp->head != NULL){
                            if(temp->head->modified_flag == 1){
                                to_reject = temp->head;
                                somma_bytes += to_reject->dim_bytes;
                                ins_tail_list(list_reject,to_reject);
                                table->n_file_modified--;
                                table->n_file--;
                                if((table->memory_used - somma_bytes + f->dim_bytes) <= table->memory_capacity){
                                    stop=1;
                                    break;
                                }  
                            }
                            temp->head = temp->head->next;
                        }
                        h++;
                    }
                    extract_file(table->cell[h].head, to_reject);
                    table->n_file_modified--;
                    table->n_file--;
                    ins_file_hashtable(table,f);
                }
                else{
                    file_t* to_reject = pop_list(&table->cell[table->len]);
                    int h = hash(*table,f->abs_path);
                    ins_file_hashtable(&table->cell[h],f);
                    table->n_file--;
                    table->n_file_modified--;
                    ins_tail_list(list_reject,to_reject);
                }
            }
            else{
                //quindi non ha abbastanza posti 
                if((table->memory_used + f->dim_bytes) <= table->memory_capacity){
                    //lavoro solo sui posti
                    if(table->n_file_modified > table->max_size_cache){
                        //se il numero dei file modificati è maggiore della capacità della cache,
                        // allora cerco il primo file modificato da rimuovere

                        //ricerca file da rimpiazzare
                        int find = 0;
                        int h = hash(*table,f->abs_path);
                        file_t* to_reject;
                        while(!find){
                            list* temp = table->cell[h].head;
                            while(temp->head != NULL){
                                if(temp->head->modified_flag == 1){
                                    to_reject = temp->head;
                                    find=1;
                                    break;
                                }
                                temp->head = temp->head->next;
                            }
                            h++;
                        }
                        extract_file(table->cell[h].head, to_reject);
                        table->n_file_modified--;
                        table->n_file--;
                        ins_tail_list(list_reject,to_reject);
                        ins_file_hashtable(table,f);
                    }
                    else{
                        file_t* to_reject = pop_list(&table->cache);
                        int h = hash(*table,f->abs_path);
                        ins_file_hashtable(&table->cell[h],f);
                        table->n_file--;
                        table->n_file_modified--;
                        ins_tail_list(list_reject,to_reject);                        
                    }
                }
            }
        }
    }
	return 1;
}

file_t* remove_file_server(hashtable* table, char* namefile){
	
    file_t* f = research_file(*table,namefile);
    if(f != NULL){
        extract_file_to_server(table,f);
		return f;
    }
	return NULL;
}

int isEmpty(list cella){
	if(cella.head != NULL) return 0;
	else return 1; 
}
int isCacheFull(hashtable table){
	if(table.cell->size == table.max_size_cache) return 1;
	else return 0;
}
int isContains_list(list cell, file_t* file){
	list temp = cell;
	while (temp.head != NULL){
		if (!strcmp(file->abs_path,temp.head->abs_path)) {
			return 1;
		}
		else temp.head = temp.head->next;
	}
	return 0;
}
int isContains_hash(hashtable table, file_t* file){
	// la hash contiene il file? Si, return 1. No, return 0;
	if(isContains_list(*table.cache,file)) return 1;
	size_t h = hash(table,file->abs_path);
	return isContains_list(table.cell[h],file);
}
void free_file(file_t* file){
	free(file->abs_path);
	free(file->filename);
	free(file);
}

void free_list(list* l){
	while(l->head != NULL){
		file_t *reject = l->head;
		l->head = l->head->next;
		free_file(reject);
	}
	free(l);
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

