#include <myqueueconnections.h>
#include <utils.h>
#include <signal.h>
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

#define _POSIX_C_SOURCE 2001112L
#define PATHCONFIG 50


static pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
static pthread_cond_t cond_var_pipe_WM = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_file = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_pipe_signal = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_pipe_WM = PTHREAD_MUTEX_INITIALIZER;

static char configFile_path[NAME_MAX] = {"configs/"};
static char logFile_path[NAME_MAX] = {"logs/"};
config configurazione;
static hashtable storage;
static list files_rejected;
static stats stats_op;
static int signal_pipe[2];
static int pipeWorker_manager[2];
static int flag_semaforo = 0;

void showDirConfig();
void setConfigFile(char* s);
void setUpServer(config* Server);
void print_serverConfig();
void cleanup();
static void *sigHandler(void *arg);
void* worker_thread_function(void* args);
void* manager_thread_function(void* args);
int update_fdmax(fd_set set, int fd_num);
void handle_connection(int *pclient_socket);
void do_task(request* req_server,response *feedback);
int task_openFile(request* r, response* feedback);
int task_read_file(request* r, response* feedback);
int task_read_N_file(request* r, response* feedback);
int task_write_file(request* r, response* feedback);
int task_append_file(request* r, response* feedback);
int task_unlock_file(request* r, response* feedback);
int task_close_file(request* r, response* feedback);
int task_remove_file(request* r, response* feedback);
void showDirLogs();
void setLogFile();
void create_FileLog();
void createFiles_inDir(char* dirname,list* l);
void createFiles_fromDupList_inDir(char* dirname,dupFile_list* l);
void init_Stats();

int main(int argc, char *argv[]){

    printf("Scegliere un file .txt di configurazione: \n");
    showDirConfig();
    char c[PATHCONFIG];
    scanf("%s",c);
    setConfigFile(c);
    setUpServer(&configurazione);
    print_serverConfig();
    init_hash(&storage,configurazione);
    init_list(&files_rejected);
    init_Stats(&stats_op);

    pthread_t thread_manager;
    pthread_t thread_workers[configurazione.n_thread_workers];
 
/*
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT); 
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGHUP);

    if (pthread_sigmask(SIG_SETMASK, &mask,NULL) != 0) {
	    fprintf(stderr, "FATAL ERROR sigmask \n");
        cleanup();
        exit(EXIT_FAILURE); 
    }
   */
  
    if ((pipe(signal_pipe))==-1) {
	    perror("signalpipe");
        cleanup();
        exit(EXIT_FAILURE); 
    }
    if ((pipe(pipeWorker_manager))==-1) {
	    perror("pipeWM");
        cleanup();
        exit(EXIT_FAILURE); 
    }
    
    pthread_t sighandler_thread;
    /*
    if(pthread_create(&sighandler_thread,NULL, sigHandler,&mask) == -1){
        perror("Errore creazione sighandler_thread");
        cleanup();
        exit(EXIT_FAILURE);  
    }
    */
    if(pthread_create(&thread_manager,NULL, manager_thread_function,NULL) == -1){
        perror("Errore creazione thread_manager");
        cleanup();
        exit(EXIT_FAILURE);  
    }
    for(size_t i = 0; i < configurazione.n_thread_workers;i++){
        if(pthread_create(&thread_workers[i],NULL, worker_thread_function,NULL) == -1){
            perror("Errore creazione thread_workers");
            cleanup();
            exit(EXIT_FAILURE);  
        }
    }

    if(pthread_join(thread_manager,NULL) != 0){
        perror("Errore join thread_manager");
            cleanup();
            exit(EXIT_FAILURE);  
    }
    for(size_t i = 0; i < configurazione.n_thread_workers;i++){
        if(pthread_join(thread_workers[i],NULL) != 0){
            perror("Errore join thread_workers");
            cleanup();
            exit(EXIT_FAILURE);  
        }
    }
/*
    if(pthread_join(sighandler_thread, NULL) != 0){
        perror("Errore join sighandler_thread");
        goto _exit;
    }
 */   
    printf("Contenuto directory Logs:\n");
    showDirLogs();
    printf("Dare un nome al filelog.txt :\n");
    scanf("%s",c);
    setLogFile(c);
    create_FileLog();

    cleanup();
    return 0;
}


void showDirConfig(){
    DIR * dir;
    if((dir = opendir("./configs")) == NULL){
        perror("Errore apertura configs DIR");
        exit(EXIT_FAILURE);
    }
    struct dirent *file;
    while((errno=0, file = readdir(dir)) != NULL) {      
        if(!isdot(file->d_name)) printf(" -%s\n",file->d_name);
    }
    printf("\n");
    free(file);
    closedir(dir);
}
void setConfigFile(char * s){
    if(s != NULL) strcat(configFile_path,s);
    else{
        fprintf(stderr,"Errore passaggio fileconfig_path\n");
        exit(EXIT_FAILURE);
    }
    strcat(configFile_path,".txt");
}
void setUpServer(config *Server){
    char buff[NAME_MAX];
    FILE* conf;
    memset(buff,0,NAME_MAX );
    if((conf = fopen(configFile_path, "r"))  == NULL){
        perror("Errore apertura file conf");
        exit(EXIT_FAILURE);
    }
    
    if(fgets(buff,NAME_MAX,conf) != NULL ){
        Server->n_thread_workers = atoi(buff);
    }
    else{
        perror("Errore lettura N thread workers");
        exit(EXIT_FAILURE);
    }
    
    if(fgets(buff,NAME_MAX,conf) != NULL ){
        Server->max_n_file = atoi(buff);
    }
    else{
        perror("Errore lettura max n file");
        exit(EXIT_FAILURE);
    }

    if(fgets(buff,NAME_MAX,conf) != NULL ){
        Server->memory_capacity = atof(buff);
    }
    else{
        perror("Errore lettura memory capacity");
        exit(EXIT_FAILURE);
    }

    if(fgets(buff,NAME_MAX,conf) != NULL ){
        strcpy(Server->socket_name, buff);
    }
    else{
        perror("Errore lettura socket path");
        exit(EXIT_FAILURE);
    }

    fclose(conf);
}
void print_serverConfig(){
    printf("\nNumero di thread workers: %d\n",configurazione.n_thread_workers);
    printf("Numero massimo di file nel server : %d\n",configurazione.max_n_file);
    printf("Capacità di memoria del server (in MegaBytes) : %.2lf\n",configurazione.memory_capacity);
    printf("Nome del socket : %s\n",configurazione.socket_name);

}

void cleanup() {
  unlink(configurazione.socket_name);
}


void* manager_thread_function(void* args){
    cleanup();
    atexit(cleanup);
    int server_socket, client_socket,b, sig;
    SA serv_addr;
    

    if((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("Errore Creazione socket_s");
        exit(EXIT_FAILURE);
    }
    memset(&serv_addr, 0, sizeof(SA));
    strncpy(serv_addr.sun_path, configurazione.socket_name,NAME_MAX);
    serv_addr.sun_family = AF_UNIX; 

    if(bind(server_socket,(struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
        perror("Errore Bind socket_s");
        exit(EXIT_FAILURE);
    }

    if(listen(server_socket,50) == -1){
        perror("Errore Listen Socket_s");
        exit(EXIT_FAILURE);
    }
    fd_set set, tmpset;
    FD_ZERO(&set);
    FD_ZERO(&tmpset);

    FD_SET(server_socket, &set);        // aggiungo il listener fd al master set
    FD_SET(signal_pipe[0], &set);  // aggiungo il descrittore di lettura della signal_pipe
    FD_SET(pipeWorker_manager[0], &set);
    int fdmax = (server_socket > signal_pipe[0]) ? server_socket : signal_pipe[0];
    fdmax = (fdmax > pipeWorker_manager[0]) ? fdmax : pipeWorker_manager[0];

    printf("Pronto a ricevere connessioni!\n");
    int termina = 0;
    while(!termina){
        tmpset = set;
        if(select(fdmax+1,&tmpset,NULL,NULL,NULL) == -1){
            perror("Errore in select");
            exit(EXIT_FAILURE);
        }
        for(int i = 0;i <= fdmax;i++){
            if(FD_ISSET(i,&tmpset)){
                if(i == server_socket){ //ascolto
                    if((client_socket = accept(server_socket,NULL, 0)) == -1){
                        perror("Errore Accept socket_s");
                        exit(EXIT_FAILURE);
                    }
                    printf("Ricevuta connessione! Client_fd: %d\n",client_socket);
                    pthread_mutex_lock(&mutex);
                    push_q(&client_socket);
                    
                    pthread_mutex_unlock(&mutex);

                } 
                else{
                    if (i == signal_pipe[0]) {
                        // ricevuto un segnale, esco ed inizio il protocollo di terminazione
                        pthread_mutex_lock(&mutex_pipe_signal);
                        if((b = readn(signal_pipe[0],&sig,sizeof(int)) == -1)){
                            perror("Errore readn pipe");
                            exit(EXIT_FAILURE);
                        }
                        pthread_mutex_unlock(&mutex_pipe_signal); 
                        if(sig == SIGINT || sig == SIGQUIT){
                            pthread_mutex_lock(&mutex);
                            pthread_cond_broadcast(&cond_var);
                            pthread_mutex_unlock(&mutex);
                            termina = 1;
                            break;
                        }
                        if(sig == SIGHUP){
                            close(server_socket);  
                            FD_CLR(server_socket,&set);

                            if(isEmpty_q()){
                                pthread_mutex_lock(&mutex);
                                pthread_cond_broadcast(&cond_var);
                                pthread_mutex_unlock(&mutex);
                                termina = 1;
                            }
                            break;
                        }   
                    }
                    else{   
                        if(i == pipeWorker_manager[0]){
                            int fd_pipe;
                            pthread_mutex_lock(&mutex_pipe_WM);
                            if((b = readn(pipeWorker_manager[0],&fd_pipe,sizeof(int)) == -1)){
                                perror("Errore readn pipe");
                                exit(EXIT_FAILURE);
                            }   
                            FD_SET(fd_pipe,&set);
                            if(fdmax < fd_pipe) fdmax = fd_pipe;
                            flag_semaforo = 1;
                            pthread_cond_signal(&cond_var_pipe_WM);
                            pthread_mutex_unlock(&mutex_pipe_WM);
                        }
                        else{ //ascolto client-> readn -> metto in coda
                            request r;
                            memset(&r,0,sizeof(request));
                            pthread_mutex_lock(&mutex);
                            if((b = readn(i,&r,sizeof(r)) == -1)){
                                perror("Errore readn pipe");
                                exit(EXIT_FAILURE);
                            }
                            FD_CLR(i,&set);
                            if(i == fdmax) fdmax = update_fdmax(set,fdmax);
                            push_r(&r);
                            pthread_cond_signal(&cond_var);
                            pthread_mutex_unlock(&mutex);

                        }


                    }
                }   
            }
            
        }
    }
    return NULL;
}
int update_fdmax(fd_set set, int fd_max){
	int i, max = 0;

	for(i = 0; i <= fd_max; i++){
		if(FD_ISSET(i, &set)){
			if(i > max)
				max = i;
		}
	}
	return max;
}

static void *sigHandler(void *arg) {
    sigset_t *set = (sigset_t*)arg;

    while(1) {
        int sig,b;
        int r = sigwait(set, &sig);
        if (r != 0) {
            errno = r;
            perror("FATAL ERROR 'sigwait'");
            return NULL;
        }

        switch(sig) {
        case SIGINT:
        case SIGHUP:
        case SIGQUIT:
            pthread_mutex_lock(&mutex_pipe_signal);
            if((b = writen(signal_pipe[1],&sig,sizeof(int))) == -1){
                perror("Errore writen pipe");
                exit(EXIT_FAILURE);
            }
            pthread_mutex_unlock(&mutex_pipe_signal);
            close(signal_pipe[1]);  // notifico il listener thread della ricezione del segnale
            return NULL;
        default:  ; 
        }
    }
    return NULL;	   
}

void* worker_thread_function(void* args){
    while (1){
        request *r;
        int b;
        memset(&r,0,sizeof(r));        
        pthread_mutex_lock(&mutex);
        while(isEmpty_r()) pthread_cond_wait(&cond_var,&mutex);
        r = pop_r();
        pthread_mutex_unlock(&mutex);
        response feedback;
        do_task(r,&feedback);
        if((b = writen(r->socket_fd,&feedback,sizeof(feedback))) == -1){
            perror("Errore writen pipe");
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&mutex_pipe_WM);
        while(!flag_semaforo) pthread_cond_wait(&cond_var_pipe_WM,&mutex_pipe_WM);
        flag_semaforo = 0;
        if((b = writen(pipeWorker_manager[1],&r->socket_fd,sizeof(int))) == -1){
            perror("Errore writen pipe");
            exit(EXIT_FAILURE);
        }
        pthread_mutex_unlock(&mutex_pipe_WM);
        free_request(r);
    }
}


void do_task(request* r_from_client,response* feedback){
    //In caso di successo ritorna 1 else 0
    memset(feedback,0,sizeof(response));
    memset(feedback->content,0,sizeof(feedback->content));

    switch (r_from_client->type){

    case OPEN_FILE:
        if(task_openFile(r_from_client,feedback)){
            stats_op.n_openfile++;
        }    
        break;
    case READ_FILE:
        if(task_read_file(r_from_client,feedback)){
            stats_op.n_readfile++;
        } 
        break;

    case READ_N_FILE:
        if(task_read_N_file(r_from_client,feedback)){
            stats_op.n_readNfile++;
        }  
        break;
    case WRITE_FILE:
        if(task_write_file(r_from_client,feedback)){
            stats_op.n_writefile++;
        }  
        break;
    case APPEND_FILE:
        if(task_append_file(r_from_client,feedback)){
            stats_op.n_appendfile++;
        }  
        break;
    
    case REMOVE_FILE:
        if(task_remove_file(r_from_client,feedback)){
            stats_op.n_removefile++;
        }  
        break;
    case CLOSE_FILE:
        if(task_close_file(r_from_client,feedback)){
            stats_op.n_closefile++;
        }  
        break;
    
    case LOCK_FILE:
        if(task_lock_file(r_from_client,feedback)){
            stats_op.n_lockfile++;
        }  
        break;
    
    case UNLOCK_FILE:
        if(task_unlock_file(r_from_client,feedback)){
            stats_op.n_unlockfile++;
        }  
        break;
    
    default:
        break;
    }
    
    PRINT_ERRNO(r_from_client->type,errno);
}



int task_openFile(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
    //DA RIGUARDARE O_LOCK
    file_t* f = research_file(storage,r->file_name);
    int res = 0, b;
    switch (r->flags){
 
    case O_CREATE :
        if(f == NULL){
            f = init_file(r->file_name);
            f->opened_flag = 1;
            f->o_create_flag = 1;
            if(!ins_file_server(&storage,f,&files_rejected)){
                feedback->type = NO_SPACE_IN_SERVER;
                free(f);
            }
            else{
                feedback->type =  O_CREATE_SUCCESS;
                storage.n_files_free++;
                res = 1;
                if((f->fd = open(f->abs_path, O_RDWR)) == -1){
                    perror("Errore open in task_open");
                    res = 0;
                }
                if(!res) feedback->type = GENERIC_ERROR;
            }
        }
        else{
            free(f);
            feedback->type = FILE_ALREADY_EXIST;
        }
        break;
    
    case O_LOCK :
        if(f == NULL)  feedback->type = O_CREATE_NOT_SPECIFIED_AND_FILE_NEXIST;
        else{
            if(f->locked_flag == 1) feedback->type = CANNOT_ACCESS_FILE_LOCKED;
            else{
                if(f->opened_flag != 1){
                    f->opened_flag = 1;
                    storage.n_files_free++;
                    feedback->type = O_LOCK_SUCCESS;
                    res = 1;
                    if((f->fd = open(f->abs_path, O_RDWR)) == -1){
                        perror("Errore open in task_open");
                        res = 0;
                    }
                    if(!res) feedback->type = GENERIC_ERROR;
                }
                else{
                    feedback->type = FILE_ALREADY_OPENED;
                    res = 1;
                }
                
            }
        }
        break;

    case O_CREATE|O_LOCK :
        if(f == NULL){
            f = init_file(r->file_name);
            f->opened_flag = 1;
            f->o_create_flag = 1;
            f->locked_flag = 1;
            if(!ins_file_server(&storage,f,&files_rejected)){
                feedback->type = NO_SPACE_IN_SERVER;
                free(f);
            }
            else{
                feedback->type =  O_CREATE_LOCK_SUCCESS;
                storage.n_files_free++;
                res = 1;
                if((f->fd = open(f->abs_path, O_RDWR)) == -1){
                    perror("Errore open in task_open");
                    res = 0;
                }
                if(!res) feedback->type = GENERIC_ERROR;
            }
        }
    
        break;

    case O_NOFLAGS :
        if(f == NULL)  feedback->type = O_CREATE_NOT_SPECIFIED_AND_FILE_NEXIST;
        else{
            if(f->locked_flag == 1) feedback->type = CANNOT_ACCESS_FILE_LOCKED;
            else{
                if(f->opened_flag != 1){
                    f->opened_flag = 1;
                    storage.n_files_free++;  
                    res = 1;
                    if((f->fd = open(f->abs_path, O_RDWR)) == -1){
                        perror("Errore open in task_open");
                        res = 0;
                    }
                    if(!res) feedback->type = GENERIC_ERROR;
                }
                else{
                    feedback->type = FILE_ALREADY_OPENED;
                    res = 1;
                }
                
            }
        }   
      
    break;

    default: break;
    }
    

    if((b = writen(r->socket_fd,&feedback,sizeof(feedback))) == -1){
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
        while((s = fgets(feedback->content,MAX_LENGHT_FILE,to_read)) != NULL);
        fclose(to_read);
    }

    finetask:
    if((b = writen(r->socket_fd,&feedback,sizeof(feedback))) == -1){
        errno = EAGAIN;
    }
    return res;
}

int task_read_N_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
    int b, res = 0,n_to_read;
    if(r->c <= 0 || r->c >= storage.n_files_free) n_to_read = storage.n_files_free;
    else n_to_read = r->c;

    if((b = writen(r->socket_fd,&n_to_read,sizeof(int))) == -1){
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
                    if((f = fopen(temp.head->abs_path,"r")) == NULL){
                        perror("Errore apertura file in readNfile");
                        feedback->type = READ_N_FILE_FAILURE;
                        goto finetask;
                    }
                    char* s;
                    while((s = fgets(buff,MAX_LENGHT_FILE,f)) != NULL);
                    if((b = writen(r->socket_fd,buff,sizeof(buff))) == -1){
                        errno = EAGAIN;
                    }
                    contatore_file_letti++;
                    fclose(f);
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
    if((b = writen(r->socket_fd,&feedback,sizeof(feedback))) == -1){
        errno = EAGAIN;
    }
    return res;
}
int task_write_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 
    int b,res = 0;
    file_t* f = research_file(storage,r->file_name);
    if(f == NULL){
        feedback->type = FILE_NOT_EXIST;
    }
    else{
        if(!f->locked_flag || !f->opened_flag || !f->o_create_flag){
            feedback->type = WRITE_FILE_FAILURE;
        }
        else{
            list removed_files;
            if(ins_file_server(&storage,f,&removed_files)){
                feedback->type = WRITE_FILE_SUCCESS;
                res = 1;
                if(r->dirname != NULL){
                    createFiles_inDir(r->dirname,&removed_files);
                }
                else concatList(&files_rejected,&removed_files);    
            }
            else feedback->type = NO_SPACE_IN_SERVER;    
        }
    }
    if((b = writen(r->socket_fd,&feedback,sizeof(feedback))) == -1){
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
            if((f = fopen(r->file_name,"a")) == NULL){
                perror("Errore apertura appendfile");
            }
            fputs(r->buff,f);
            fclose(f);
            feedback->type = APPEND_FILE_SUCCESS;
            res = 1;
        }
        else{
            feedback->type = NO_SPACE_IN_SERVER;
        }
        pthread_mutex_unlock(&mutex_file);    
    }

finetask:
    if((b = writen(r->socket_fd,&feedback,sizeof(feedback))) == -1){
        errno = EAGAIN;
    }
    return res;
}

int task_unlock_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
    return 0;
}
int task_lock_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
    return 0;
}

int task_close_file(request* r, response* feedback){
    //In caso di successo ritorna 1 else 0
    int b,res = 0;
    file_t* file = research_file(storage,r->file_name);
    if(file == NULL){
        feedback->type = FILE_NOT_EXIST;
    }
    else{    
        if(file->opened_flag == 0){
            feedback->type = FILE_NOT_OPENED;
            goto finetask;
        }
        file->opened_flag = 0;
        feedback->type = CLOSE_FILE_SUCCESS;
        res = 1;
        if(close(file->fd) == -1){
            perror("Errore close file, in task_close_file");
            res = 0;
        }
        if(!res) feedback->type = GENERIC_ERROR;
        file->fd = -2;
    }

finetask:
    if((b = writen(r->socket_fd,&feedback,sizeof(feedback))) == -1){
        errno = EAGAIN;
    }
    return res;
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
        file->locked_flag = 1;
        pthread_mutex_lock(&mutex_file);
        remove_file_server(&storage,file);
        ins_tail_list(&files_rejected,file);
        pthread_mutex_unlock(&mutex_file);
        feedback->type = REMOVE_FILE_SUCCESS;
        res = 1;
    }

    finetask:
    if((b = writen(r->socket_fd,&feedback,sizeof(feedback))) == -1){
        errno = EAGAIN;
    }
    return res;
}

void showDirLogs(){
    DIR * dir;
    if((dir = opendir("./logs")) == NULL){
        perror("Errore apertura configs DIR");
        exit(EXIT_FAILURE);
    }
    struct dirent *file;
    while((errno=0, file = readdir(dir)) != NULL) {      
        if(!isdot(file->d_name)) printf(" -%s\n",file->d_name);
    }
    printf("\n");
    free(file);
    closedir(dir);
}

void setLogFile(char * s){
    if(s != NULL) strcat(logFile_path,s);
    else{
        fprintf(stderr,"Errore passaggio fileconfig_path\n");
        exit(EXIT_FAILURE);
    }
    strcat(logFile_path,".txt");
}

void create_FileLog(){
    
    FILE *f;
    if((f= fopen(logFile_path,"a")) == NULL){
        perror("Errore creazione filelog");
    }
    fprintf(f,"Numero di file attualmente nel server : %d\n",storage.n_file);
    fprintf(f,"Numero di file massimo memorizzato nel server : %d\n",storage.stat_max_n_file);
    fprintf(f,"Memoria attualmente utilizzata in Mbytes nel file storage : %.3lf\n",bytesToMb(storage.memory_used));
    fprintf(f,"Dimensione massima in Kbytes raggiunta dal file storage : %.2lf\n",bytesToKb(storage.stat_dim_file));
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

    if((new = fopen(newfile_path,"a")) == NULL){
      perror("Errore apertura in createFiles_dir");
    }

    if((to_read = fopen(l->head->abs_path,"r")) == NULL){
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

    if((new = fopen(newfile_path,"a")) == NULL){
      perror("Errore apertura in createFiles_fromDupList_inDir");
    }

    if((to_read = fopen(l->head->riferimento_file->abs_path,"r")) == NULL){
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

