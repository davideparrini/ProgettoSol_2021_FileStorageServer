#include <myhashstoragefile.h>


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
	l->dim_bytes = 0;
}
void init_hash(hashtable *table, config s){
	//inizializzazione hash
	table->len = s.max_n_file*2+1;
	table->memory_capacity = MbToBytes(s.memory_capacity);
	table->memory_used = 0;
	table->memory_used_from_modified_files = 0;
	table->n_file = 0;
	table->n_files_free = 0;
	table->n_file_modified = 0;
	table->max_n_file = s.max_n_file;
	table->max_size_cache = s.max_n_file * 10 / 100;

	table->stat_dim_file = 0;
	table->stat_max_n_file = 0;
	table->stat_n_replacing_algoritm = 0;

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
	cell->dim_bytes += file->dim_bytes;
}

void ins_head_list(list *cell,file_t *file){
	//inserimento in testa della lista
	file->next = NULL; //per sicurezza annullo i puntatori next/prec del file
	file->prec = NULL;
    file->next = cell->head;
    cell->head->prec = file;
    cell->head = file;
    cell->size++;
	cell->dim_bytes += file->dim_bytes;
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
		cell->dim_bytes -= res->dim_bytes;
		return res;
	}
}


	
void ins_file_hashtable(hashtable *table, file_t* file){
	size_t h = hash(*table,file->abs_path);
    ins_tail_list(&table->cell[h],file);   
	table->memory_used += file->dim_bytes;
	table->n_file++;
	if(table->stat_max_n_file < table->n_file) table->stat_max_n_file = table->n_file;
}

void ins_file_cache(hashtable *table,file_t* file){
	ins_head_list(&table->cell[table->len],file);
	table->memory_used += file->dim_bytes;
	table->n_file++;
	if(table->stat_max_n_file < table->n_file) table->stat_max_n_file = table->n_file;
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
	cell->dim_bytes -= file->dim_bytes;
}


void extract_file_to_server(hashtable *table,file_t* file){
	
	if(file->inCache_flag){
		extract_file(table->cache,file);
    }
	else{	
		int h = hash(*table,file->abs_path);
		extract_file(&table->cell[h],file);
	}
	table->memory_used -= file->dim_bytes;
	table->memory_used_from_modified_files -= file->dim_bytes;
	if(file->modified_flag){
		table->n_file_modified--;
		file->modified_flag = 0;
	}
	if(file->opened_flag == 1 && file->locked_flag == 0) table->n_files_free--;
	table->n_file--;
	file->inCache_flag = 0;
	

}

void update_file(hashtable *table,file_t* file){
	/*	funzione per aggiornare la posizione dei file
	*/
	if(file->modified_flag == 0){
		table->n_file_modified++;
		file->modified_flag = 1;
	}
    if(file->inCache_flag){
		extract_file(table->cache,file);
		ins_head_list(
			table->cache,file);
	}
	else{
		int h;
		if(isCacheFull){
			file_t *f = pop_list(table->cache);
			f->inCache_flag = 0;
			table->cache->dim_bytes -= f->dim_bytes;
			h = hash(*table,f->abs_path);
			ins_tail_list(&table->cell[h],file);
		}
		h = hash(*table,file->abs_path);
		extract_file(&table->cell[h],file);
		ins_head_list(&table->cell[table->len],file);
		table->cache->dim_bytes += file->dim_bytes;
		file->inCache_flag = 1;		
	}
}

file_t* research_file(hashtable table,char *namefile){
	char abs_path[NAME_MAX];
	MY_REALPATH(research_file,namefile,abs_path);
	int h = hash(table,abs_path);
	file_t* f;
	if((f = research_file_list(table.cell[h],abs_path)) != NULL){
		return f;
	}
	else{
		if((f = research_file_list(*table.cache,abs_path)) != NULL){
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
			f = temp.head;
			return f;
		}
		else temp.head = temp.head->next;
	}
	return NULL;
}



int modifying_file(hashtable* table,file_t* f,size_t size_inplus,list* list_reject){
	
	if(table->memory_capacity < table->memory_used + size_inplus){
		if(table->n_file_modified == 0){
            printf("Server pieno e non ci sono file modificati nel server\nImpossibile modificare file\n");
			free(f);  
			return 0;
        }
		else{
			if(table->n_file_modified > table->max_size_cache){
				//il numero dei file modificati è maggiore della capacità della cache
				//quindi inizio a cercare nel server
				int stop = 0, max_files_toVisite = table->n_file - table->cache->size;
				int h = 0;
				dupFile_t* to_reject;
				dupFile_list* temp_reject;
				init_dupFile_list(temp_reject);
				unsigned long somma_bytes; //somma bytes della list_reject

				while(!stop && max_files_toVisite > 0 && h < table->len){
					list temp = table->cell[h];
					while(temp.head != NULL){
						max_files_toVisite--;
						if(temp.head->modified_flag == 1){
							to_reject = init_dupFile(temp.head);
							somma_bytes += to_reject->riferimento_file->dim_bytes;
							ins_head_dupFilelist(temp_reject,to_reject);
							if((table->memory_capacity >= table->memory_used - somma_bytes + size_inplus) ){
								stop=1;
								break;
							}  
						}
						temp.head = temp.head->next;
					}
					h++;
				}
				if(!stop && max_files_toVisite == 0){
					if(table->memory_capacity > table->cache->dim_bytes + somma_bytes){
						free(to_reject);
						free_duplist(temp_reject);
						return 0;
					}
					else{ 
						while(table->memory_capacity > table->memory_used + size_inplus){
							file_t* to_reject = pop_list(table->cache);
							if(to_reject->opened_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
							table->n_file--;
							table->n_file_modified--;
							table->memory_used -= to_reject->dim_bytes;
							table->memory_used_from_modified_files -= to_reject->dim_bytes;
							ins_tail_list(list_reject,to_reject);
							free(to_reject);
						}
					}	 
				}
				ins_dupList_to_list(table,list_reject,temp_reject);
				free(to_reject);
				free_duplist(temp_reject);
				table->stat_n_replacing_algoritm++;
			}
			else{
				if(table->memory_capacity > table->cache->dim_bytes + size_inplus){
					free(f);
					return 0; 
				}
				while(table->memory_capacity > table->memory_used + f->dim_bytes){
					file_t* to_reject = pop_list(table->cache);
					table->n_file--;
					table->n_file_modified--;
					ins_tail_list(list_reject,to_reject);
					free(to_reject);
				}
				table->stat_n_replacing_algoritm++; 
			}
		}
	}
	table->memory_used += size_inplus;
	table->memory_used_from_modified_files += size_inplus;
	f->dim_bytes += size_inplus;
	update_file(table,f);
	if(table->stat_dim_file < f->dim_bytes + size_inplus) table->stat_dim_file = f->dim_bytes + size_inplus;
	return 1;
}


int ins_file_server(hashtable* table, file_t *f,list* list_reject){

    if(table->n_file < table->max_n_file && (table->memory_used + f->dim_bytes) <= table->memory_capacity){
        ins_file_hashtable(table,f);
		return 1;
    }
    else{
        if(table->n_file_modified == 0){
            printf("Server pieno e non ci sono file modificati nel server\nImpossibile inserire file\n");
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
                    int stop = 0,h = 0, max_files_toVisite = table->n_file - table->cache->size;
					dupFile_t* to_reject;
					dupFile_list* temp_reject;
					init_dupFile_list(temp_reject);
                    unsigned long somma_bytes; //somma bytes della list_reject
                    while(!stop && max_files_toVisite > 0 && h < table->len){
                        list temp = table->cell[h];
                        while(temp.head != NULL){
							max_files_toVisite--;
                            if(temp.head->modified_flag == 1){
                                to_reject = init_dupFile(temp.head);
                                somma_bytes += to_reject->riferimento_file->dim_bytes;
                                ins_head_dupFilelist(temp_reject,to_reject);
                                if(table->memory_capacity >= (table->memory_used - somma_bytes + f->dim_bytes) ){
                                    stop=1;
                                    break;
                                }  
                            }
                            temp.head = temp.head->next;
                        }
                        h++;
                    }
					if(!stop && max_files_toVisite  == 0 ){
						if(f->dim_bytes > table->cache->dim_bytes + somma_bytes){
							free_file(f);
							free(to_reject);
							free_duplist(temp_reject);
							return 0;
						}
						while(table->memory_capacity > table->memory_used + f->dim_bytes){
							file_t* to_reject = pop_list(table->cache);
							table->memory_used -= to_reject->dim_bytes;
							table->memory_used_from_modified_files -= to_reject->dim_bytes;
							table->n_file--;
							table->n_file_modified--;
							if(to_reject->opened_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
							ins_tail_list(list_reject,to_reject);
							free(to_reject);
						} 
					}
                    table->n_file_modified -= temp_reject->size;
                    table->n_file -= temp_reject->size;
					ins_dupList_to_list(table,list_reject,temp_reject);        
					free(to_reject);
					free_duplist(temp_reject);
                }
                else{
					if(table->cache->dim_bytes >= f->dim_bytes){
						while(table->memory_capacity > table->memory_used + f->dim_bytes){
							file_t* to_reject = pop_list(table->cache);
							if(to_reject->opened_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
							table->memory_used -= to_reject->dim_bytes;
							table->memory_used_from_modified_files -= to_reject->dim_bytes;
							table->n_file--;
							table->n_file_modified--;
							ins_tail_list(list_reject,to_reject);
						}
					}
					else{
						free_file(f);
					}	
                }
            }
            else{
                //quindi non ha abbastanza posti 
                if((table->memory_used + f->dim_bytes) <= table->memory_capacity){
                    //lavoro solo sui posti
					//ricerca file da rimpiazzare
					int find = 0,h = 0;
					file_t* to_reject;
					while(!find && h < table->len){
						list temp = table->cell[h];
						while(temp.head != NULL){
							if(temp.head->modified_flag == 1){
								to_reject = temp.head;
								find=1;
								break;
							}
							temp.head = temp.head->next;
						}
						h++;
					}
					if(!find){
						to_reject = pop_list(table->cache);
						if(to_reject->opened_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
					}
					else extract_file(&table->cell[h], to_reject);
					table->memory_used -= to_reject->dim_bytes;
					table->memory_used_from_modified_files -= to_reject->dim_bytes;
					table->n_file_modified--;
					table->n_file--;
					ins_tail_list(list_reject,to_reject);
                    
                }
            }
        }
    }
	table->stat_n_replacing_algoritm++;
	ins_file_hashtable(table,f);
	if(f->dim_bytes > table->stat_dim_file) table->stat_dim_file = f->dim_bytes;
	return 1;
}

file_t* remove_file_server(hashtable* table, file_t* f){
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

list* concatList(list *l,list *l2){
	if(l->head != NULL && l2->head != NULL){
		l->tail->next = l2->head;
		l2->head->prec = l->tail;
		l->size += l2->size;
		l2->head = NULL;
		l2->tail = NULL;
		return l;
	}
	else return NULL;
}
void free_list(list* l){
	while(l->head != NULL){
		file_t *reject = l->head;
		l->head = l->head->next;
		free_file(reject);
	}
	free(l);
}
void cleanList(list *l){
	while(l->head != NULL){
		file_t *reject = l->head;
		l->head = l->head->next;
		free_file(reject);
	}
	l->head = NULL;
	l->tail = NULL;
	l->size = 0;
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


dupFile_t* init_dupFile(file_t* f){
	dupFile_t* new = malloc(sizeof(dupFile_t));
	new->riferimento_file = f;
	new->next = NULL;
	return new;
}

void init_dupFile_list(dupFile_list* l){
	l->head = NULL;
	l->size = 0;
}

void ins_head_dupFilelist(dupFile_list *l,dupFile_t *file){
	//inserimento in testa della lista
	file->next = NULL; //per sicurezza annullo i puntatori next/prec del file
    file->next = l->head;
    l->head = file;
    l->size++;
}

dupFile_t* pop_dupFilelist(dupFile_list *cell){
	//rimouve e ritorna l'ultimo elemento della lista
    if(cell->head == NULL){
            return NULL; 
    }
    else{
		dupFile_t *res = cell->head;
		cell->head = cell->head->next;
		res->next = NULL;
		cell->size--;
		return res;
	}
}

void ins_dupList_to_list(hashtable* table, list* l, dupFile_list* dl){
	while(dl->head != NULL){
		dupFile_t* temp = dl->head;
		extract_file_to_server(table,dl->head->riferimento_file);
		ins_tail_list(l,dl->head->riferimento_file);
		dl->head = dl->head->next;
		dl->size--;
		free(temp);
	}
}

void free_duplist(dupFile_list* dl){
	while(dl->head != NULL){
		dupFile_t* temp = dl->head;
		dl->head = dl->head->next;
		dl->size--;
		free(temp);
	}
	free(dl);
}