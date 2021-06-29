#include <myhashstoragefile.h>
#include <utils.h>



file_t* init_file(char *namefile){
	if(namefile == NULL){
		fprintf(stderr,"ERRORE initFile, namefile == NULL\n");
		return NULL;
	}
	file_t* new = malloc(sizeof(file_t));
	memset(new,0,sizeof(file_t));

	new->fd = -2;

	new->abs_path= malloc(sizeof(char)*NAME_MAX);
	memset(new->abs_path,0,sizeof(char)*NAME_MAX);
	strncpy(new->abs_path,namefile,NAME_MAX);

	new->content = NULL;
	new->dim_bytes = 0;
	new->modified_flag = 0;
	new->open_flag = 0;
	new->o_create_flag = 0;
	new->locked_flag = 0;
	new->inCache_flag = 0;
	new->next = NULL;
	new->prec = NULL;
	return new;	
}

int writeContentFile(file_t* f){
	int l;
	struct stat s;
	if(f->fd == -2){
		perror("File non aperto, writeContent");
		return 0;
	}
	if (stat(f->abs_path, &s) == -1) {
        perror("stat creazione file\n");
		return 0;
    }
	f->content = malloc(s.st_size + 1);
	memset(f->content,0,s.st_size + 1);
	while((l = read(f->fd, f->content, s.st_size +1 )) > 0);
	if(l == -1){
		perror("lettura file in writeContentFile");
		return 0;
	}
	
	f->o_create_flag = 0;
	f->locked_flag = 0;
	f->dim_bytes = s.st_size;
	return 1;
}

void appendContent(file_t * f,void *buff,size_t size){
	if(f->content == NULL){
		f->content = malloc(size);
		memset(f->content,0,size);
		memcpy(f->content, buff, size);
		f->dim_bytes = size;
	}
	else{
		char content[f->dim_bytes + size + 1];
		memset(content, 0,f->dim_bytes + size +1);
		strncat(content, f->content, f->dim_bytes);
		strncat(content, (char*)buff, size);
		free(f->content);
		f->content = malloc(f->dim_bytes + size + 1);
		memset(f->content, 0, f->dim_bytes + size + 1);
		memcpy(f->content, content, f->dim_bytes + size +1);
		
		f->dim_bytes += size;
	}

}

void init_list(list* l){
	l->head = NULL;
	l->tail = NULL;
	l->size = 0;
	l->dim_bytes = 0;
}
void init_hash(hashtable *table, config s){
	//inizializzazione hash
	table->len = s.max_n_file*2;
	table->memory_capacity = MbToBytes(s.memory_capacity);
	table->memory_used = 0;
	table->memory_used_from_modified_files = 0;
	table->n_file = 0;
	table->n_files_free = 0;
	table->n_file_modified = 0;
	table->max_n_file = s.max_n_file;
	table->max_size_cache = ((s.max_n_file * 10 / 100) > 1 ? (s.max_n_file * 10 / 100) : 1);

	table->stat_dim_file = 0;
	table->stat_max_n_file = 0;
	table->stat_n_replacing_algoritm = 0;

	table->cell = malloc(table->len*sizeof(list));
	memset(table->cell,0,table->len*sizeof(list));
	for (size_t i = 0;i < table->len; i++){
		init_list(&table->cell[i]);
	}
	init_list(&table->cache);
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



void ins_head_list(list *cell,file_t *file){
	//inserimento in testa della lista
	file->next = NULL; //per sicurezza annullo i puntatori next/prec del file
	file->prec = NULL;
	if (cell->size == 0){
		cell->tail = file;
	}
	else cell->head->prec = file;
    file->next = cell->head;
    cell->head = file;
    cell->size++;
	cell->dim_bytes += file->dim_bytes;
}
void ins_tail_list(list *cell,file_t *file){
	//inserimento in coda della lista
	file->next = NULL; //per sicurezza annullo i puntatori next/prec del file
	file->prec = NULL;
	if (cell->size == 0){
		cell->head = file;
        cell->head->prec = NULL;
	}
	else{
        file->prec = cell->tail;
		cell->tail->next = file;
	}
	cell->tail = file;
    cell->tail->next = NULL;
	cell->size++;
	cell->dim_bytes += file->dim_bytes;
}
file_t* pop_head_list(list *cell){
	if(cell->size == 0){
        return NULL; 
    }
	file_t* res = cell->head;
	if(cell->size == 1){
		cell->head = NULL;
		cell->tail = NULL;
	}
	else{
		cell->head = cell->head->next;
		cell->head->prec = NULL;
	}
	res->next = NULL;
	res->prec = NULL;
	cell->size--;
	cell->dim_bytes -= res->dim_bytes;
	return res;

}
file_t* pop_tail_list(list *cell){
	//rimouve e ritorna l'ultimo elemento della lista
    if(cell->size == 0){
        return NULL; 
    }
    else{
		file_t *res = cell->tail;
		if(cell->size == 1){
			cell->head = NULL;
			cell->tail = NULL;
		}
		else{
			cell->tail->prec->next	= NULL;
			cell->tail = cell->tail->prec;
		}
		res->next = NULL;
		res->prec = NULL;
		cell->size--;
		cell->dim_bytes -= res->dim_bytes;
		return res;
	}
}


	
void ins_file_hashtable(hashtable *table, file_t* file){
	size_t h = hash(*table,file->abs_path);
    ins_tail_list(&table->cell[h],file);   
	table->n_file++;
	if(!file->locked_flag && file->open_flag) table->n_files_free++;
	if(table->stat_max_n_file < table->n_file) table->stat_max_n_file = table->n_file;
}

void ins_file_cache(hashtable *table,file_t* file){
	ins_head_list(&table->cache ,file);
	table->memory_used += file->dim_bytes;
	table->n_file++;
	if(table->stat_max_n_file < table->n_file) table->stat_max_n_file = table->n_file;
}

void extract_file(list* cell,file_t* file){
	//questa funzione va utilizzata assumendo di sapere per certo che il file è nella cella passata della hash
	if(cell->size == 0) return;
	if(cell->size == 1){
		cell->head = NULL;
		cell->tail = NULL;
	}
	else{
		if(file->prec == NULL){ // è il primo elemento della lista
			file->next->prec = NULL;
			cell->head = cell->head->next;
		}
		else{
			file->prec->next = file->next;
			if(file->next == NULL){ //è l'ultimo elemento della lista
				cell->tail = file->prec;
				
			}
			else{
				file->next->prec = file->prec;
			}
		}

	}
	
	
	
	
	/*
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
	}*/

	file->next = NULL;
	file->prec = NULL;		
	cell->size--;
	cell->dim_bytes -= file->dim_bytes;
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
		table->memory_used_from_modified_files -= file->dim_bytes;
		table->n_file_modified--;
		file->modified_flag = 0;
	}
	if(file->open_flag == 1 && file->locked_flag == 0) table->n_files_free--;
	table->n_file--;
	file->inCache_flag = 0;
	

}

file_t* research_file(hashtable table,char *namefile){
	if(namefile == NULL){
		printf("Non è stato passato bene namefile in researchFile\n\n");
		return NULL;
	}	
	int h = hash(table,namefile);    
	file_t* f = NULL;
	if((f = research_file_list(table.cell[h],namefile)) != NULL){
		return f;
	}
	else{
		if((f = research_file_list(table.cache,namefile)) != NULL){
			return f;
		}
	}
	return NULL;
}

file_t* research_file_list(list cell,char* namefile){
	file_t* f = NULL;
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



int init_file_inServer(hashtable* table,file_t *f,list* list_reject){
	if(table->n_file < table->max_n_file){
        ins_file_hashtable(table,f);
		return 1;
    }
	if(table->n_file_modified == 0){
        printf("Server pieno e non ci sono file modificati nel server\nImpossibile inizializzare file\n"); 
		return 0;
    }

	int find = 0,h = 0;
	file_t* to_reject;
	while(!find && h < table->len){
		list temp = table->cell[h];
		while(!find && temp.head != NULL){
			if(temp.head->modified_flag == 1){
				to_reject = temp.head;
				find=1;
				break;
			}
			temp.head = temp.head->next;
		}
		if(!find) h++;	
	}
	if(!find) to_reject = pop_tail_list(&table->cache);
	else extract_file(&table->cell[h], to_reject);
	if(to_reject->open_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
	table->memory_used -= to_reject->dim_bytes;
	table->memory_used_from_modified_files -= to_reject->dim_bytes;
	table->n_file_modified--;
	table->n_file--;
	ins_tail_list(list_reject,to_reject);       
	table->stat_n_replacing_algoritm++;
	ins_file_hashtable(table,f);
	return 1;
}



int ins_file_server(hashtable* table, file_t *f,list* list_reject){
	struct stat s;
	
	if (stat(f->abs_path, &s) == -1) {
        perror("stat file in ins_file_server\n");
		return 0;
    }
	
    if((table->memory_used + s.st_size) <= table->memory_capacity){
        if(!writeContentFile(f)) return 0;
		table->memory_used += f->dim_bytes;
		table->n_files_free++;
		if(f->dim_bytes > table->stat_dim_file) table->stat_dim_file = f->dim_bytes;
		return 1;
    }
	if(table->n_file_modified == 0){
		printf("Server pieno e non ci sono file modificati nel server\nImpossibile inserire file\n");
		return 0;
	}
	if(table->memory_capacity < table->memory_used - table->memory_used_from_modified_files + s.st_size){
		printf("Server pieno e non c è abbastanza spazio nel server\nImpossibile inserire file\n");
		return 0;
	}
	if(table->n_file_modified > table->max_size_cache){
		//ricerca file da rimpiazzare
		int stop = 0,h = 0, max_files_toVisite = table->n_file - table->cache.size;
		size_t somma_bytes; //somma bytes della list_reject
		while(!stop && max_files_toVisite > 0){
			list temp = table->cell[h];
			while(temp.head != NULL){
				max_files_toVisite--;
				if(temp.head->modified_flag == 1 && (strncmp(temp.head->abs_path,f->abs_path,strlen(f->abs_path)) != 0)){
					file_t *to_reject = temp.head;
					extract_file(&temp,to_reject);
					table->n_file--;
					table->n_file_modified--;
					somma_bytes += to_reject->dim_bytes;
					if(to_reject->open_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
					ins_head_list(list_reject,to_reject);
					if(table->memory_capacity >= (table->memory_used - somma_bytes + s.st_size) ){
						stop=1;
						break;
					}  
				}
				temp.head = temp.head->next;
			}
			h++;
		}
		if(!stop && max_files_toVisite  == 0 ){
			int flag_compare = 0;
			while(table->memory_capacity < table->memory_used + s.st_size){
				file_t* to_reject = pop_tail_list(&table->cache);
				if(!flag_compare && !strncmp(f->abs_path,to_reject->abs_path,strlen(f->abs_path))){
					to_reject = pop_tail_list(&table->cache);
					flag_compare = 1;
				}
				table->memory_used -= to_reject->dim_bytes;
				table->memory_used_from_modified_files -= to_reject->dim_bytes;
				table->n_file--;
				table->n_file_modified--;
				if(to_reject->open_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
				ins_tail_list(list_reject,to_reject);
			}
			if(flag_compare) ins_tail_list(&table->cache,f); 
		}
	}
	else{
		int flag_compare = 0;
		while(table->memory_capacity < table->memory_used + s.st_size){
			file_t* to_reject = pop_tail_list(&table->cache);
			if(!flag_compare && !strncmp(f->abs_path,to_reject->abs_path,strlen(f->abs_path))){
				to_reject = pop_tail_list(&table->cache);
				flag_compare = 1;
			}
			if(to_reject->open_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
			table->memory_used -= to_reject->dim_bytes;
			table->memory_used_from_modified_files -= to_reject->dim_bytes;
			table->n_file--;
			table->n_file_modified--;
			ins_tail_list(list_reject,to_reject);
		}
		if(flag_compare) ins_tail_list(&table->cache,f);	
	}
	if(!writeContentFile(f)) return 0;
	table->stat_n_replacing_algoritm++;
	table->memory_used += f->dim_bytes;
	table->n_files_free++;
	if(f->dim_bytes > table->stat_dim_file) table->stat_dim_file = f->dim_bytes;
	return 1;
}

int modifying_file(hashtable* table,file_t* f,size_t size_inplus,list* list_reject){
	
	if(table->memory_capacity < table->memory_used + size_inplus){
		if(table->n_file_modified == 0){
            printf("Server pieno e non ci sono file modificati nel server\nImpossibile modificare file\n");
			return 0;
        }
		else{
			if(table->memory_capacity < table->memory_used - table->memory_used_from_modified_files + size_inplus){
				printf("Server pieno e non c è abbastanza spazio nel server\nImpossibile modificare file\n");
				return 0;
			}
			if(table->n_file_modified > table->max_size_cache){
				//il numero dei file modificati è maggiore della capacità della cache
				//quindi inizio a cercare nel server
				int stop = 0, max_files_toVisite = table->n_file - table->cache.size;
				int h = 0;
				size_t somma_bytes; //somma bytes della list_reject
				while(!stop && max_files_toVisite > 0 && h < table->len){
					list temp = table->cell[h];
					while(temp.head != NULL){
						max_files_toVisite--;
						if(temp.head->modified_flag == 1 && (strncmp(temp.head->abs_path,f->abs_path,strlen(f->abs_path)) != 0)){
							file_t * to_reject = temp.head;
							extract_file(&temp,to_reject);
							table->n_file--;
							table->n_file_modified--;
							if(to_reject->open_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
							somma_bytes += to_reject->dim_bytes;
							ins_head_list(list_reject,to_reject);
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
					int flag_compare = 0;
					while(table->memory_capacity < table->memory_used + size_inplus){
						file_t* to_reject = pop_tail_list(&table->cache);
						if(!flag_compare && !strncmp(f->abs_path,to_reject->abs_path,strlen(f->abs_path))){
							to_reject = pop_tail_list(&table->cache);
							flag_compare = 1;
						}
						if(to_reject->open_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
						table->n_file--;
						table->n_file_modified--;
						table->memory_used -= to_reject->dim_bytes;
						table->memory_used_from_modified_files -= to_reject->dim_bytes;
						ins_tail_list(list_reject,to_reject);
					}
					if(flag_compare) ins_tail_list(&table->cache,f);
				}
			}
			else{
				int flag_compare = 0;
				while(table->memory_capacity < table->memory_used + f->dim_bytes){
					file_t* to_reject = pop_tail_list(&table->cache);
					if(!flag_compare && !strncmp(f->abs_path,to_reject->abs_path,strlen(f->abs_path))){
						to_reject = pop_tail_list(&table->cache);
						flag_compare = 1;
					}
					table->n_file--;
					table->n_file_modified--;
					if(to_reject->open_flag == 1 && to_reject->locked_flag == 0) table->n_files_free--;
					table->memory_used -= to_reject->dim_bytes;
					table->memory_used_from_modified_files -= to_reject->dim_bytes;
					ins_tail_list(list_reject,to_reject);
					free(to_reject);
				}
				if(flag_compare) ins_tail_list(&table->cache,f);
			}
			table->stat_n_replacing_algoritm++;
		}
	}
	table->memory_used += size_inplus;
	f->dim_bytes += size_inplus;
	table->memory_used_from_modified_files += f->dim_bytes;

	if(f->modified_flag == 0){
		table->n_file_modified++;
		f->modified_flag = 1;
	}
	update_file(table,f);
	if(table->stat_dim_file < f->dim_bytes + size_inplus) table->stat_dim_file = f->dim_bytes + size_inplus;
	return 1;
}

void update_file(hashtable *table,file_t* file){
	/*	funzione per aggiornare la posizione dei file
	*/
	if(file->modified_flag){

		if(file->inCache_flag){
			extract_file(&table->cache,file);
			ins_head_list(&table->cache,file);
		}
		else{
			int h;
			if(isCacheFull(*table)){
				file_t *f = pop_tail_list(&table->cache);
				f->inCache_flag = 0;
				table->cache.dim_bytes -= f->dim_bytes;
				h = hash(*table,f->abs_path);
				ins_tail_list(&table->cell[h],f);
			}
			h = hash(*table,file->abs_path);
			extract_file(&table->cell[h],file);
			ins_head_list(&table->cache,file);
			table->cache.dim_bytes += file->dim_bytes;
			file->inCache_flag = 1;		
		}
		
	}
}


file_t* remove_file_server(hashtable* table, file_t* f){
    if(f != NULL){
        extract_file_to_server(table,f);
		if(f->fd >= 0) close(f->fd);
		f->fd = -2;
		return f;
    }
	return NULL;
}





int isEmpty(list cella){
	if(cella.head != NULL) return 0;
	else return 1; 
}
int isCacheFull(hashtable table){
	if(table.cache.size == table.max_size_cache) return 1;
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
	if(isContains_list(table.cache,file)) return 1;
	size_t h = hash(table,file->abs_path);
	return isContains_list(table.cell[h],file);
}


void concatList(list *l,list *l2){
	while(l2->head != NULL){
		file_t* aux = pop_head_list(l2);
		ins_tail_list(l,aux);
	}

}

void print_list(file_t* head){
    if(head == NULL) printf("NULL\n");
    else{
        printf("%s -> ",head->abs_path);
        print_list(head->next);
    }
}

void print_storageServer(hashtable table){
	//stampa hash
	printf("\n\n");
	for (int i=0; i < table.len; i++){
		printf("%d -> ",i);
		if(table.cell[i].head != NULL){
			list temp = table.cell[i];
			
			while(temp.head != NULL){
				printf("%s -> ",temp.head->abs_path);
				temp.head = temp.head->next;
			}
		}
		printf("NULL\n");
	}
	printf("Cache -> ");
	list temp = table.cache;
	while(temp.head != NULL){
		printf("%s -> ",temp.head->abs_path);
		temp.head = temp.head->next;
	}
	printf("NULL\n\n");

	printf("N file in server %d\n\n\n",table.n_file);
}



void free_file(file_t* file){

	if(file->content != NULL) free(file->content);
	if(file->fd > 0) close(file->fd);
	free(file->abs_path);
	free(file);
}
void free_list(list* l){
	while(l->head != NULL){
		file_t *reject = l->head;
		l->head = l->head->next;
        if(l->head == NULL) l->tail = NULL;
		free_file(reject);
	}
}

void free_hash(hashtable *table){
	for (int i=0; i < table->len; i++){
		if(table->cell[i].head != NULL){
			free_list(&table->cell[i]);
		}
	}
	free_list(&table->cache);
	free(table->cell);
}