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
#include <time.h>


#include <utils.h>
#include <request.h>
#include <response.h>

extern char* socket_path;
extern int client_fd;

int openConnection(const char* sockname, int msec, const struct timespec abstime){
    SA sockaddr;
    memset(&sockaddr, 0, sizeof(SA));
    strcpy(sockaddr.sun_path, sockname);
    sockaddr.sun_family = AF_UNIX; 
    
    printf("\nFILE DESCRIPTOR CLIENT_FD %d\n",client_fd);//Risultato aspettato : 0 Risultato effettivo : 0

    if(client_fd = socket(AF_UNIX, SOCK_STREAM, 0) == -1){
        perror("Errore Creazione socket_c");
        exit(EXIT_FAILURE);
    }

    printf("\nFILE DESCRIPTOR CLIENT_FD DOPO CHIAMATA SOCKET() %d\n",client_fd); //Risultato aspettato : NUMERO INTERO, Risultato effettivo : 0
	int test_fd;
    if(test_fd = socket(AF_UNIX, SOCK_STREAM, 0) == -1){
        perror("Errore Creazione ssssc");
        exit(EXIT_FAILURE);
    }
    printf("\nFILE DESCRIPTOR CLIENT_FD %d test_fd : %d\n ENTRAMBE DOPO CHIAMATA socket()\n",client_fd,test_fd);//Risultato aspettato : NUMERO INTERO, NUMERO MAGGIORE DEL PRIMO  Risultato effettivo : 0 , 0

    time_t before = time(NULL);
    time_t diff = 0;
    int tentativi = 0;
    while(abstime.tv_sec > diff){
         
        if(connect(client_fd,(struct sockaddr*)&sockaddr,sizeof(sockaddr)) != -1){
            printf("Connesso al server!\n");
            return 0;
        }
        else{
            if(errno == ENOTCONN || errno == ECONNREFUSED || errno == ENOENT){
                printf("Non connesso, s_server non è ancora pronto!\nTentativo: %d\n\n",tentativi+1);
                msleep(msec);
                diff = time(NULL) - before;
            }
            else{
                PRINT_ERRNO("Openconnection",errno);
                return -1;
            }  
        }
    }

    errno = ETIMEDOUT;
    printf("Sessione scaduta!\n");
    return -1;
}



int closeConnection(const char* sockname){
    if(!strcmp(socket_path, sockname)){
        errno = EBADF;
        return -1;
    }
    if(close(client_fd) == -1){
        errno = EBADF;
        return -1;
    }
    printf("Connessione terminata!\n");
    return 0;
}

int openFile(const char* pathname, int flags){
    request r;
    response feedback;
    int b;

    memset(&r, 0, sizeof(request));
    memset(&feedback, 0, sizeof(response));
    r.flags = flags;
    r.type = OPEN_FILE; 
    r.socket_fd = client_fd;
    r.file_name = malloc(sizeof(char)* NAME_MAX);
    memset(&r.file_name,0,sizeof(r.file_name));
    memset(&feedback.content,0,sizeof(feedback.content));
    MY_REALPATH(openFile,pathname,r.file_name);

    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }

    free(r.file_name);

    if(b = readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;

    }

    switch (feedback.type){
    
    case OPEN_FILE_SUCCESS:
        printf("File ' %s '  aperto con successo\n",pathname);
        break;     

    case O_CREATE_SUCCESS:
        printf("File ' %s ' creato con successo\n",pathname);
        break;                 

    case LOCK_FILE_SUCCESS:
        printf("File ' %s '  locked con successo\n",pathname);
        break;       

     case O_CREATE_LOCK_SUCCESS:
        printf("File ' %s '  creato/locked con successo\n",pathname);
        break;       

    case O_CREATE_NOT_SPECIFIED_AND_FILE_NEXIST:
        errno = ENOENT;
        return -1;

    case NO_SPACE_IN_SERVER:
        errno = ENOSPC;
        return -1;  
        
    case FILE_ALREADY_OPENED:
        errno = 0;
        return 0;

    case FILE_ALREADY_EXIST:
        errno = EEXIST;
        return -1;

    case CANNOT_ACCESS_FILE_LOCKED:
        errno = EPERM;
        return -1;

    default: break;
    }
    errno = 0; 
    return 0;

}
int readFile(const char* pathname, void** buf, size_t* size){
    int b;
    request r;
    response feedback;

    memset(&r,0,sizeof(request));
    r.file_name = malloc(sizeof(char)*NAME_MAX);
    memset(&r.file_name,0,sizeof(r.file_name));
    memset(&feedback, 0, sizeof(response));
    memset(&feedback.content,0,sizeof(feedback.content));
    r.type = READ_FILE; 
    r.socket_fd = client_fd;

    memset(&feedback.content,0,sizeof(feedback.content));
    MY_REALPATH(readFile,pathname,r.file_name);
    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }

    free(r.file_name);

    if(b = readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }

    switch (feedback.type){
        
    case FILE_NOT_OPENED:
        *buf = NULL;
        *size = 0;
        errno = EACCES;
        return -1;

    case FILE_NOT_EXIST:
        *buf = NULL;
		*size = 0;
		errno = ENOENT;
        return -1;

    case CANNOT_ACCESS_FILE_LOCKED:
        *buf = NULL;
		*size = 0;
        errno = EPERM;
        return -1;

    case READ_FILE_SUCCESS:
        printf("File ' %s ' letto con successo\n", pathname);
        break;
          
    default: break;
    }
    *size = feedback.size;
    *buf = malloc(feedback.size+1);
    memset(buf, 0, feedback.size+1);
    memcpy(*buf,&feedback.content,*size);
    errno = 0;
    return 0;

}
int readNFiles(int N, const char* dirname){
    int b;
    request r;
    response feedback;
    memset(&r,0,sizeof(request));
    r.dirname = malloc(sizeof(char)* NAME_MAX);
    memset(&r.dirname,0,sizeof(r.dirname));
    memset(&feedback, 0, sizeof(response));
    memset(&feedback.content,0,sizeof(feedback.content));

    if(dirname != NULL){
        MY_REALPATH(writeFile,dirname,r.dirname);
    }
    else r.dirname = NULL;

    r.type = READ_N_FILE; 
    r.socket_fd = client_fd;
    r.c = N;

    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }

    free(r.dirname);
    int n_to_read;
    if(b = readn(client_fd,&n_to_read,sizeof(int)) == -1){
        errno = EAGAIN;
        return -1;
    }
    int i = 1;
    while(i <= n_to_read){
        size_t buff_size;
        if(b = readn(client_fd,&buff_size,sizeof(size_t)) == -1){
            errno = EAGAIN;
        }
        void *buff = malloc(buff_size+1);
        memset(&buff,0,buff_size);
        if(b = readn(client_fd,&buff,buff_size) == -1){
            errno = EAGAIN;
        }
        printf("****Contenuto file %d :****\n%s\n\n",i,(char*) buff);
        free(buff);
        i++;
    }

    if(b = readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }

    switch (feedback.type){
    case READ_N_FILE_FAILURE:
        errno = EPERM;
        return -1;
    
    case READ_N_FILE_SUCCESS:
        errno = 0;
        return feedback.c;
    
    default: printf("?????\n");
        return -1;
    }

}
int writeFile(const char* pathname, const char* dirname){
    int b;
    request r;
    response feedback;
    memset(&r,0,sizeof(request));
    r.file_name = malloc(sizeof(char)*NAME_MAX);
    memset(&r.file_name,0,sizeof(r.file_name));
    r.dirname = malloc(sizeof(char)* NAME_MAX);
    memset(&r.dirname,0,sizeof(r.dirname));
    memset(&feedback, 0, sizeof(response));
    memset(&feedback.content,0,sizeof(feedback.content));

    r.type = WRITE_FILE;
    r.socket_fd = client_fd;
    MY_REALPATH(writeFile,pathname,r.file_name);
    if(dirname != NULL){
        MY_REALPATH(writeFile,dirname,r.dirname);
    }
    else{
        r.dirname = NULL;
    }
    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }
    free(r.dirname);
    free(r.file_name);
    
    if(b = readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }
    //se il flag della risposta del server è settato a 1, significa
    //che è stata passata in precedenza openFile(pathname, O_CREATE| O_LOCK)
    else{
        switch (feedback.type){
        
        case WRITE_FILE_FAILURE:
            errno = EPERM;
            return -1;

        case FILE_NOT_EXIST:
            errno = ENOENT;
            return -1;

        case NO_SPACE_IN_SERVER:
            errno = ENOSPC;
            return -1;

        case WRITE_FILE_SUCCESS:
            printf("Write_file ' %s ' ha avuto successo\n",pathname);
            break;
                
        default:
            break;
        } 
    }    
    errno = 0;
    return 0;
}

int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname){

    int b;
    request r;
    response feedback;
    memset(&r,0,sizeof(request));
    r.file_name = malloc(sizeof(char)*NAME_MAX);
    memset(&r.file_name,0,sizeof(r.file_name));
    r.dirname = malloc(sizeof(char)* NAME_MAX);
    memset(&r.dirname,0,sizeof(r.dirname));
    memset(&feedback, 0, sizeof(response));
    memset(&feedback.content,0,sizeof(feedback.content));
    r.buff = malloc(size);
    memset(r.buff,0,size);

    r.buff = buf;
    r.request_size = size;
    r.type = APPEND_FILE;
    r.socket_fd = client_fd;
    MY_REALPATH(appendToFile,pathname,r.file_name);
    if(dirname != NULL){
        MY_REALPATH(appendToFile,dirname,r.dirname);
    }
    else{
        r.dirname = NULL;
    }
    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }
    free(r.dirname);
    free(r.file_name);
    free(r.buff);

    if(b = readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }

    switch (feedback.type){
        
    case FILE_NOT_OPENED:
		errno = EACCES;
        return -1;

    case FILE_NOT_EXIST:
		errno = ENOENT;
        return -1;

    case CANNOT_ACCESS_FILE_LOCKED:
        errno = EPERM;
        return -1;

    case NO_SPACE_IN_SERVER:
		errno = ENOSPC;
        return -1;

    case APPEND_FILE_SUCCESS:
        printf("Append_file ' %s ' ha avuto successo\n",pathname);
        break;
          
    default: break;
    }
    errno = 0;
    return 0;   
}

int lockFile(const char* pathname);
int unlockFile(const char* pathname);
int closeFile(const char* pathname){
    int b;
    request r;
    response feedback;
    memset(&r,0,sizeof(request));
    r.file_name = malloc(sizeof(char)*NAME_MAX);
    memset(&r.file_name,0,sizeof(r.file_name));;
    memset(&feedback, 0, sizeof(response));
    memset(&feedback.content,0,sizeof(feedback.content));

    r.type = CLOSE_FILE;
    r.socket_fd = client_fd;
    MY_REALPATH(closeFile,pathname,r.file_name);

    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }
    free(r.file_name);
    
    if(b = readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }
    switch (feedback.type){
        
    case FILE_NOT_OPENED:
		errno = 0;
        return 0;

    case FILE_NOT_EXIST:
		errno = ENOENT;
        return -1;

    case CLOSE_FILE_SUCCESS:
        printf("closeFile ' %s ' ha avuto successo\n",pathname);
        break;
          
    default: break;
    
    }
    errno = 0;
    return 0;  
}

int removeFile(const char* pathname){
    
    int b;
    request r;
    response feedback;
    memset(&r,0,sizeof(request));
    r.file_name = malloc(sizeof(char)*NAME_MAX);
    memset(&r.file_name,0,sizeof(r.file_name));;
    memset(&feedback, 0, sizeof(response));
    memset(&feedback.content,0,sizeof(feedback.content));

    r.type = REMOVE_FILE;
    r.socket_fd = client_fd;
    MY_REALPATH(appendToFile,pathname,r.file_name);
   
    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }
    free(r.file_name);
    
    if(b = readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }

    switch (feedback.type){
        
    case FILE_NOT_LOCKED:
		errno = ENOLCK;
        return -1;

    case FILE_NOT_EXIST:
		errno = ENOENT;
        return -1;

    case CANNOT_ACCESS_FILE_LOCKED:
        errno = EPERM;
        return -1;


    case REMOVE_FILE_SUCCESS:
        printf("Remove_file ' %s ' ha avuto successo\n",pathname);
        break;
          
    default: break;
    
    }
    errno = 0;
    return 0;  
}