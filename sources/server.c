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
#include <myhashstoragefile.h>





#define BUFSIZE 1024


pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *log_file;
config configurazione;

void cleanup();
void handle_task(int *pclient_socket);
void setUpServer(config* Server);
void* worker_thread_function(void* args);
void* main_thread_function(void* args);
void do_task(request* req_server);

int main(){

    setUpServer(&configurazione);
    hashtable storage;
    

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

void handle_task(int *pclient_socket){
    int client_socket = *(int*) pclient_socket;
    char actualpath[PATH_MAX+1];
    request richiesta_server;

    memset(&richiesta_server, 0, sizeof(request));
    readn(client_socket, &richiesta_server, sizeof(request)); 

    do_work(&richiesta_server);
   /* if(realpath(richiesta_server,actualpath) == NULL){
        printf("Errore (bad path) : %s\n",buffer);
        close(client_socket);
        return;
    }
    */


   // DA RIGUARDARE
}


void do_task(request* req_server){
    switch (req_server->type){

    case open_file:
        if(task_openFile())
        break;
    case read_file:
        if(task_readFile())
        break;

    case write_file:
        if(task_writeFile())
        break;
    case append_file:
        if(task_appendFile())
        break;
    
    default:
        break;
    }

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
    strncpy(serv_addr.sun_path, configurazione.socket_name,PATH_MAX);
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
        printf("Connesso al client!\n");
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

void setUpServer(config *Server){
    char buff[PATH_MAX];
    FILE* conf;
    if((conf = fopen("config.txt", "r"))  == NULL){
        perror("Errore apertura file conf");
        exit(EXIT_FAILURE);
    }
    char* s;
    if((s = fgets(buff,PATH_MAX,conf)) != NULL ){
        Server->n_thread_workers = atoi(s);
    }
    else{
        perror("Errore lettura N thread workers");
    }
    if((s = fgets(buff,PATH_MAX,conf) ) != NULL ){
        Server->max_n_file = atoi(s);
    }
    else{
        perror("Errore lettura max n file");
    }
    if((s = fgets(buff,PATH_MAX,conf) ) != NULL ){
        Server->memory_capacity = atoi(s);
    }
    else{
        perror("Errore lettura memory capacity");
    }
    if((s = fgets(buff,PATH_MAX,conf)) != NULL ){
        strncpy(Server->socket_name, s, strlen(s));
    }
    else{
        perror("Errore lettura socket path");
    }
    if((s = fgets(buff,PATH_MAX,conf)) != NULL ){
        strncpy(Server->log_file_path, s, strlen(s));
    }
    else{
        perror("Errore lettura log file path");
    }

    fclose(conf);
}





