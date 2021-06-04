#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/select.h>
#include <limits.h>
#include <signal.h>
#include <libgen.h>

#include <myqueue.h>
#include <utils.h>
#include <request.h>
#include <response.h>
#include <myhashstoragefile.h>

typedef struct {
    int n_openfile;
    int n_closefile;
    int n_readfile;
    int n_readNfile;
    int n_writefile;
    int n_appendfile;
    int n_removefile;
    int n_lockfile;
    int n_unlockfile;

}stats;

#define BUFSIZE 1024

#define FILELOG "./filelog.txt"
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_file = PTHREAD_MUTEX_INITIALIZER;

config configurazione;
hashtable storage;
list files_rejected;
stats stats_op;

void cleanup();
void handle_task(int *pclient_socket);
void setUpServer(config* Server);

void* worker_thread_function(void* args);
void* main_thread_function(void* args);
int do_task(request* req_server,int* client_s);
int task_openFile(request* r, response* feedback, int* flag_open_lock,int* flag_lock);
int task_read_file(request* r, response* feedback);
int task_read_N_file(request* r, response* feedback);
int task_write_file(request* r, response* feedback,int* flag_open_lock);
int task_append_file(request* r, response* feedback);
int task_unlock_file(request* r, response* feedback);
int task_close_file(request* r, response* feedback);
int task_remove_file(request* r, response* feedback);
void create_FileLog();
void createFiles_inDir(char* dirname,list* l);
void createFiles_fromDupList_inDir(char* dirname,dupFile_list* l);
void init_Stats();

int main(){

    setUpServer(&configurazione);
    init_hash(&storage,configurazione);
    init_list(&files_rejected);
    init_Stats(&stats_op);

    pthread_t thread_main;
    pthread_t thread_workers[configurazione.n_thread_workers];
   
    pthread_create(&thread_main,NULL, main_thread_function,NULL);
    for(size_t i = 0; i < configurazione.n_thread_workers;i++){
        pthread_create(&thread_workers[i],NULL, worker_thread_function,NULL);
    }

    pthread_join(&thread_main,NULL);
    for(size_t i = 0; i < configurazione.n_thread_workers;i++){
        pthread_join(&thread_workers[i],NULL);
    }
    create_FileLog();

    return 0;
}
void cleanup() {
  unlink(configurazione.socket_name);
}


void* main_thread_function(void* args){
    cleanup();
    atexit(cleanup);
    int server_socket, client_socket;
    SA serv_addr;

    if(server_socket = socket(AF_UNIX, SOCK_STREAM, 0) == -1){
        perror("Errore Creazione socket_s");
        exit(EXIT_FAILURE);
    }
    memset(&serv_addr, 0, sizeof(SA));
    strncpy(serv_addr.sun_path, configurazione.socket_name,NAME_MAX);
    serv_addr.sun_family = AF_UNIX; 

    if(bind(server_socket,(struct sockaddr*) &serv_addr, 1) == -1){
        perror("Errore Bind socket_s");
        exit(EXIT_FAILURE);
    }

    if(listen(server_socket,50) == -1){
        perror("Errore Listen Socket_s");
        exit(EXIT_FAILURE);
    }
    while(1){
        
        if(client_socket = accept(server_socket,(struct sockaddr*)&client_socket, sizeof(client_socket)) == -1){
            perror("Errore Accept socket_s");
            exit(EXIT_FAILURE);
        }
        printf("\nConnesso al client!\n\n");
        pthread_mutex_lock(&mutex);
        push_q(&client_socket);
        pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&mutex);

    }
}

void* worker_thread_function(void* args){
    while (1){
        int *pclient;
        pthread_mutex_lock(&mutex);
        if((pclient = pop_q()) == NULL){
            pthread_cond_wait(&cond_var,&mutex);
        }    
        pclient = pop_q();
        pthread_mutex_unlock(&mutex);
        if(pclient != NULL){
            //connessi
            handle_task(pclient);    
        }
    }

}
void handle_task(int *pclient_socket){
    int client_socket = *(int*) pclient_socket;
    char actualpath[NAME_MAX+1];
    request richiesta_server;

    memset(&richiesta_server, 0, sizeof(request));
    readn(client_socket, &richiesta_server, sizeof(request)); 


    if(do_task(&richiesta_server,client_socket)){
        close(client_socket);
        return ;
    } 
    printf("Impossibile svolgere task!\n");
    close(client_socket);
   // DA RIGUARDARE
}

int do_task(request* req_server,int* client_s){
    //In caso di successo ritorna 1 else 0
    int flag_open_lock = 0;
    int flag_lock = 0;
    int b,res = 0;
    response feedback;
    memset(feedback.content,0,sizeof(feedback.content));
    switch (req_server->type){

    case open_file:
        if(task_openFile(req_server,&feedback,&flag_open_lock,&flag_lock)){
            res = 1;
            stats_op.n_openfile++;
        }    
        break;
    case read_file:
        if(task_readFile(req_server,&feedback)){
            res = 1;
            stats_op.n_readfile++;
        } 
        break;

    case read_N_file:
        if(task_read_N_File(req_server,&feedback)){
            res = 1;
            stats_op.n_readNfile++;
        }  
        break;
    case write_file:
        if(task_writeFile(req_server,&feedback)){
            res = 1;
            stats_op.n_writefile++;
        }  
        break;
    case append_file:
        if(task_appendFile(req_server,&feedback)){
            res = 1;
            stats_op.n_appendfile++;
        }  
        break;
    
    default:
        break;
    }
    while(flag_open_lock || flag_lock){

    }  
    
    if(b = writen(client_s,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
    }
    return res;
}



int task_openFile(request* r, response* feedback, int* flag_open_lock,int* flag_lock){
    //In caso di successo ritorna 1 else 0
    //DA RIGUARDARE O_LOCK
    file_t* f = research_file(storage,r->file_name);
    int res = 0, b;
    switch (r->flags){

    case O_CREATE :
        if(f == NULL){
            f = init_file(r->file_name);
            f->opened_flag = 1;
            if(!ins_file_server(&storage,f,&files_rejected)){
                feedback->type = NO_SPACE_IN_SERVER;
            }
            else{
                feedback->type =  O_CREATE_SUCCESS;
                storage.n_files_free++;
                res = 1;
            }
        }
        else{
            free(f);
            feedback->type = FILE_ALREADY_EXIST;
        }
        break;
    
    case O_LOCK :
        
    break;
    
    default: break;
    }
    if(r->flags != O_CREATE){
        if(f == NULL)  feedback->type = O_CREATE_NOT_SPECIFIED_AND_FILE_NEXIST;
        else{
            if(f->locked_flag == 1) feedback->type = CANNOT_ACCESS_FILE_LOCKED;
            else{
                if(f->opened_flag != 1){
                    f->opened_flag = 1;
                    storage.n_files_free++;
                    feedback->type = OPEN_FILE_SUCCESS;
                    res = 1;
                }
                else{
                    feedback->type = FILE_ALREADY_OPENED;
                    res = 1;
                }
                
            }
        }   
    }

    if(b = writen(r->socket_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
    }
    return  res;
}

int task_read_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
    int b,res = 0;
    file_t* f = research_file(storage,r->file_name);
    if(f == NULL) feedback->type = FILE_NOT_EXIST;
    else{
        if(f->locked_flag == 1){
            feedback->type = CANNOT_ACCESS_FILE_LOCKED;
            goto finetask;
        } 
        if(f->opened_flag != 0){
            feedback->type = FILE_NOT_OPENED;
            goto finetask;
        } 

        feedback->type = READ_FILE_SUCCESS;
        res = 1;
        FILE* to_read;
        char *s;
        if((to_read = fopen(r->file_name,"r")) == NULL){
            perror("Errore apertura file in task_read_file");
        }
        while(s = fgets(feedback->content,MAX_LENGHT_FILE,to_read) != NULL);
        fclose(to_read);
        free(to_read);
    }

    finetask:
    if(b = writen(r->socket_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
    }
    return res;
}

int task_read_N_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
    int b, res = 0,n_to_read;
    if(r->c <= 0 || r->c >= storage.n_files_free) n_to_read = storage.n_files_free;
    else n_to_read = r->c;

    if(b = writen(r->socket_fd,&n_to_read,sizeof(int)) == -1){
        errno = EAGAIN;
    }
    size_t h = 0;
    size_t contatore_file_letti = 0; 
    dupFile_list d_list;
    init_dupFile_list(&d_list);   
    while(n_to_read > 0 && h <= storage.len){
        if(storage.cell[h].head != NULL){
			list temp = storage.cell[h];
			while(temp.head != NULL && n_to_read > 0){
                if(temp.head->opened_flag == 1 && temp.head->locked_flag == 0){
                    char *buff = malloc(sizeof(char)*MAX_LENGHT_FILE);
                    dupFile_t* df = init_dupFile(temp.head);
                    ins_head_dupFilelist(&d_list,df);
                    FILE* f;
                    if(f = fopen(temp.head->abs_path,"r") == NULL){
                        perror("Errore apertura file in readNfile");
                        feedback->type = READ_N_FILE_FAILURE;
                        goto finetask;
                    }
                    char* s;
                    while(s = fgets(buff,MAX_LENGHT_FILE,f) != NULL);
                    if(b = writen(r->socket_fd,buff,sizeof(buff)) == -1){
                        errno = EAGAIN;
                    }
                    contatore_file_letti++;
                    fclose(f);
                    free(f);
                    free(s);
                    free(df);
                    free(buff);
                } 	
				temp.head = temp.head->next;
			}
		}
        h++;   
    }
    if(r->dirname != NULL){
        createFiles_fromDupList_inDir(r->dirname,&d_list);
    }

    if(contatore_file_letti != r->c || n_to_read != 0){
        feedback->type = READ_N_FILE_FAILURE;    
    }
    else{
        feedback->type = READ_FILE_SUCCESS;
        feedback->c = contatore_file_letti;
        res = 1;
    }

finetask:
    free_duplist(&d_list);
    if(b = writen(r->socket_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
    }
    return res;
}
int task_write_file(request* r, response* feedback,int* flag_open_lock){
    //In caso di successo ritorna 1 else 
    int b,res = 0;
    if(*flag_open_lock != 1){
        feedback->type = WRITE_FILE_FAILURE;
    }
    else{
        list removed_files;
        if(ins_file_server(&storage,r->file_name,&removed_files)){
            feedback->type = WRITE_FILE_SUCCESS;
            res = 1;
            if(r->dirname != NULL){
                createFiles_inDir(r->dirname,&removed_files);
            }
            else concatList(&files_rejected,&removed_files);    
        }
        else feedback->type = NO_SPACE_IN_SERVER;    
    }

    if(b = writen(r->socket_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
    }
    return res;
}
int task_append_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
    int b,res = 0;
    file_t* file = research_file(storage,r->file_name);
    if(file == NULL){
        feedback->type = FILE_NOT_EXIST;
    }
    else{
        if(file->locked_flag == 1){
            feedback->type = CANNOT_ACCESS_FILE_LOCKED;
            goto finetask;
        }

        if(file->opened_flag != 1){
            feedback->type = FILE_NOT_OPENED;
            goto finetask;
        }

        pthread_mutex_lock(&mutex_file);
        if(modifying_file(&storage,file,r->request_size,&files_rejected)){
            FILE* f;
            if(f = fopen(r->file_name,"a") == NULL){
                perror("Errore apertura appendfile");
            }
            fputs(r->buff,f);
            fclose(f);
            free(f);
            feedback->type = APPEND_FILE_SUCCESS;
            pthread_mutex_unlock(&mutex_file);
            res = 1;
        }
        else{
            pthread_mutex_unlock(&mutex_file);
            feedback->type = NO_SPACE_IN_SERVER;
        }    
    }

finetask:
    if(b = writen(r->socket_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
    }
    return res;
}

int task_unlock_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
}
int task_lock_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
}

int task_close_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
}

int task_remove_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
    int b,res = 0;
    file_t* file = research_file(storage,r->file_name);
    if(file == NULL){
        feedback->type = FILE_NOT_EXIST;
    }
    else{
        if(file->locked_flag == 1){
            feedback->type = CANNOT_ACCESS_FILE_LOCKED;
            goto finetask;
        }
        pthread_mutex_lock(&mutex_file);
        file->locked_flag = 1;
        remove_file_server(&storage,file);
        pthread_mutex_unlock(&mutex_file);
        feedback->type = REMOVE_FILE_SUCCESS;
        res = 1;
    }

    finetask:
    if(b = writen(r->socket_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
    }
    return res;
}



void create_FileLog(){
    
    FILE *f;
    if(f= fopen(FILELOG,"a") == NULL){
        perror("Errore creazione filelog");
    }
    fprintf(f,"Numero di file attualmente nel server : %d\n",storage.n_file);
    fprintf(f,"Numero di file massimo memorizzato nel server : %d\n",storage.stat_max_n_file);
    fprintf(f,"Memoria attualmente utilizzata in Mbytes nel file storage : %ld\n",bytesToMb(storage.memory_used));
    fprintf(f,"Dimensione massima in Mbytes raggiunta dal file storage : %ld\n",storage.stat_dim_file);
    fprintf(f,"Numero di volte in cui l’algoritmo di rimpiazzamento della cache è stato eseguito per selezionare uno o più \
    file “vittima” : %d\n",storage.stat_n_replacing_algoritm); 

    fprintf(f,"Lista dei file contenuti nello storage al momento della chiusura del server:\n\n");
	for (int i=0; i <= storage.len; i++){
		if(i != storage.len) fprintf(f,"%d -> ",i);
		else fprintf(f,"Cache -> ");
		if(storage.cell[i].head != NULL){
			size_t n = storage.cell[i].size;
			list temp = storage.cell[i];
			while(n > 0){
				fprintf(f,"%s -> ",temp.head->filename);
                file_t* temp_file = temp.head;	
				temp.head = temp.head->next;
                free_file(temp_file);
				n--;
			}
		}
		fprintf(f,"NULL\n");
	}
    fprintf(f,"\n");
    fprintf(f,"Lista file vittima dell'algoritmo di rimpiazzamento:\n");
    while(!isEmpty(files_rejected)){
        fprintf(f,"*rejected* %s\n",files_rejected.head->filename);
        file_t* temp = files_rejected.head;
        files_rejected.head = files_rejected.head->next;
        free_file(temp);
    }

    fprintf(f,"Numero di operazioni di openFile : %d\n",stats_op.n_openfile);
    fprintf(f,"Numero di operazioni di closeFile : %d\n",stats_op.n_closefile);
    fprintf(f,"Numero di operazioni di readFile : %d\n",stats_op.n_readfile);
    fprintf(f,"Numero di operazioni di readNFile : %d\n",stats_op.n_readNfile);
    fprintf(f,"Numero di operazioni di writeFile : %d\n",stats_op.n_writefile);
    fprintf(f,"Numero di operazioni di appendFile : %d\n",stats_op.n_appendfile);
    fprintf(f,"Numero di operazioni di removeFile : %d\n",stats_op.n_removefile);
    fprintf(f,"Numero di operazioni di lockFile : %d\n",stats_op.n_lockfile);
    fprintf(f,"Numero di operazioni di unlockFile : %d\n",stats_op.n_unlockfile);

    fclose(f);
}


void setUpServer(config *Server){
    char buff[NAME_MAX];
    FILE* conf;
    memset(buff,0,MAX_LENGHT_FILE);
    char* s;
    if((conf = fopen("config.txt", "r"))  == NULL){
        perror("Errore apertura file conf");
        exit(EXIT_FAILURE);
    }
    
    if((s = fgets(buff,NAME_MAX,conf)) != NULL ){
        Server->n_thread_workers = atoi(s);
        exit(EXIT_FAILURE);
    }
    else{
        perror("Errore lettura N thread workers");
        exit(EXIT_FAILURE);
    }
    if((s = fgets(buff,NAME_MAX,conf) ) != NULL ){
        Server->max_n_file = atoi(s);
    }
    else{
        perror("Errore lettura max n file");
        exit(EXIT_FAILURE);
    }
    if((s = fgets(buff,NAME_MAX,conf) ) != NULL ){
        Server->memory_capacity = atoi(s);
    }
    else{
        perror("Errore lettura memory capacity");
        exit(EXIT_FAILURE);
    }
    if((s = fgets(buff,NAME_MAX,conf)) != NULL ){
        strncpy(Server->socket_name, s, strlen(s));
    }
    else{
        perror("Errore lettura socket path");
        exit(EXIT_FAILURE);
    }
    if((s = fgets(buff,NAME_MAX,conf)) != NULL ){
        strncpy(Server->log_file_path, s, strlen(s));
    }
    else{
        perror("Errore lettura log file path");
        exit(EXIT_FAILURE);
    }

    fclose(conf);
    free(conf);
}

void createFiles_inDir(char* dirname,list* l){
  //crea file nella directory dirname, dirname deve essere il path assoluto della directory
  //i file sono passati dalla list l
  while(!isEmpty(*l)){
    file_t *temp = l->head;
    char* s_file = strndup(l->head->abs_path,strlen(l->head->abs_path));
    char* s_dir = strndup(dirname,strlen(dirname));
    char* pathfile = basename(s_file);
    free(s_file);
    free(s_dir);
    char* newfile_path = strncat(s_dir,s_file,strlen(s_file));
    free(pathfile);
    char* content = malloc(sizeof(char)*MAX_LENGHT_FILE);
    FILE *new, *to_read;

    if(new = fopen(newfile_path,"a") == NULL){
      perror("Errore apertura in createFiles_dir");
    }

    if(to_read = fopen(l->head->abs_path,"r") == NULL){
      perror("Errore apertura in createFiles_dir");
    }

    while(fgets(content,MAX_LENGHT_FILE,to_read) != NULL);
	
    fputs(content,new);

    fclose(to_read);
    fclose(new);

    free(newfile_path);
    free(content);
    free(new);
    free(to_read);
    l->head = l->head->next;
    free_file(temp);
  }

}

void createFiles_fromDupList_inDir(char* dirname,dupFile_list* l){
  //crea file nella directory dirname, dirname deve essere il path assoluto della directory
  //i file sono passati dalla dupFile_list l
  dupFile_t *temp = l->head;
  while(temp != NULL){
    char* s_file = strndup(l->head->riferimento_file->abs_path,strlen(l->head->riferimento_file->abs_path));
    char* s_dir = strndup(dirname,strlen(dirname));
    char* pathfile = basename(s_file);
    free(s_file);
    free(s_dir);
    char* newfile_path = strncat(s_dir,s_file,strlen(s_file));
    free(pathfile);
    char* content = malloc(sizeof(char)*MAX_LENGHT_FILE);
    FILE *new, *to_read;

    if(new = fopen(newfile_path,"a") == NULL){
      perror("Errore apertura in createFiles_fromDupList_inDir");
    }

    if(to_read = fopen(l->head->riferimento_file->abs_path,"r") == NULL){
      perror("Errore apertura in createFiles_fromDupList_inDir");
    }

    while(fgets(content,MAX_LENGHT_FILE,to_read) != NULL);
	
    fputs(content,new);

    fclose(to_read);
    fclose(new);

    free(newfile_path);
    free(content);
    free(new);
    free(to_read);
    temp = temp->next;

  }

}

void init_Stats(stats* statistiche_operazioni){

    statistiche_operazioni->n_openfile = 0;
    statistiche_operazioni->n_closefile = 0;
    statistiche_operazioni->n_readfile = 0;
    statistiche_operazioni->n_readNfile = 0;
    statistiche_operazioni->n_writefile = 0;
    statistiche_operazioni->n_appendfile = 0;
    statistiche_operazioni->n_removefile = 0;
    statistiche_operazioni->n_lockfile = 0;
    statistiche_operazioni->n_unlockfile = 0;

}