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

#include <myqueue.h>
#include <utils.h>
#include <request.h>
#include <response.h>
#include <myhashstoragefile.h>





#define BUFSIZE 1024


pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *log_file;
config configurazione;
hashtable storage;
list file_rejected;

void cleanup();
void handle_task(int *pclient_socket);
void setUpServer(config* Server);
void* worker_thread_function(void* args);
void* main_thread_function(void* args);
int do_task(request* req_server,int* client_s);
int task_openFile(request* r, response* feedback);
int task_read_file(request* r, response* feedback);
int task_read_N_file(request* r, response* feedback);
int task_write_file(request* r, response* feedback);
int task_append_file(request* r, response* feedback);
int task_unlock_file(request* r, response* feedback);
int task_close_file(request* r, response* feedback);
int task_remove_file(request* r, response* feedback);
void append_FileLog(char* buff);

int main(){

    setUpServer(&configurazione);
    init_hash(&storage,configurazione);
    init_list(&file_rejected);

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

    //check validity
    if(realpath(richiesta_server.file_name,actualpath) == NULL){
        printf("Errore (bad path) : %s\n",richiesta_server.file_name);
        close(client_socket);
        return;
    }
    
    if(!do_task(&richiesta_server,client_socket)){
        close(client_socket);
        return ;
    } 
    printf("Impossibile svolgere task!\n");
    close(client_socket);
   // DA RIGUARDARE
}

int do_task(request* req_server,int* client_s){
    response feedback;
    memset(feedback.content,0,sizeof(feedback.content));
    switch (req_server->type){

    case open_file:
        if(task_openFile(req_server,&feedback))
        break;
    case read_file:
        if(task_readFile(req_server,&feedback))
        break;

    case read_N_file:
        if(task_readFile(req_server,&feedback))
        break;
    case write_file:
        if(task_writeFile(req_server,&feedback))
        break;
    case append_file:
        if(task_appendFile(req_server,&feedback))
        break;
    
    default:
        break;
    }

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

int task_openFile(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0

    file_t* f = research_file(storage,r->file_name);
    int res = 0, b;
    switch (r->flags){

    case O_CREATE :
        if(f == NULL){
            f = init_file(r->file_name);
            f->opened_flag = 1;
            if(!ins_file_server(&storage,f,&file_rejected)){
                feedback->type = CANNOT_CREATE_FILE;
            }
            else{
                feedback->type =  O_CREATE_SUCCESS;
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
    PRINT_ERRNO(task_openFile,errno);
    return  res;
}

int task_read_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
    int res = 0;
    file_t* f = research_file(storage,r->file_name);
    if(f == NULL) feedback->type = FILE_NOT_EXIST;
    else{
        if(f->locked_flag == 1) feedback->type = CANNOT_ACCESS_FILE_LOCKED;
        else{
            if(f->opened_flag != 0) feedback->type = FILE_NOT_OPENED;
            else{
                feedback->type = READ_FILE_SUCCESS;
                res = 1;
                FILE* to_read;
                char *s;
                if((to_read = fopen(r->file_name,"r")) == NULL){
                    perror("Errore apertura file in task_read_file");
                }
                while(!feof(to_read)){
                    if((s = fgets(feedback->content,NAME_MAX,to_read)) != NULL){
                        perror("Errore lettura file in task_read_file");
                    }
                    printf("%s",feedback->content);
                }
                fclose(to_read);
                free(to_read);
            }
        }
    }
    PRINT_ERRNO(task_read_file,errno);
    return res;
}

int task_read_N_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
}
int task_write_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
}
int task_append_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
}
int task_unlock_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
}
int task_close_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
}

int task_remove_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
}

void append_FileLog(char* buff){
    //In caso di successo ritorna 1 else 0
}

