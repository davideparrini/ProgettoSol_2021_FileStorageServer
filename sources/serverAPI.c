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

extern config configurazione;

int socket_c;

int openConnection(const char* sockname, int msec, const struct timespec abstime){
    SA sockaddr;
    memset(&sockaddr, 0, sizeof(SA));
    strncpy(sockaddr.sun_path, sockname ,strlen(sockname)+1);
    sockaddr.sun_family = AF_UNIX; 
  
	if(socket_c = socket(AF_UNIX, SOCK_STREAM, 0) == -1){
        perror("Errore Creazione socket_c");
        exit(EXIT_FAILURE);
    }
    time_t before = time(NULL);
    time_t diff = 0;
    int tentativi = 0;
    while(abstime.tv_sec > diff){
        if( connect(socket_c,(SA*) &sockaddr,sizeof(sockaddr)) != -1){
            printf("Connesso al server!\n");
            return 0;
        }
        else{
            if(errno == ENOTCONN || errno == ECONNREFUSED || errno == ENOENT){
                printf("Non connesso, s_server non è ancora pronto!\nTentativo: %d\n\n",tentativi+1);
                usleep(msec * 1000);
                diff = time(NULL) - before;
            }
            else{
                printf("Errore connessione c_socket!\n");
                return -1;
            }  
        }
    }

    errno = ETIMEDOUT;
    printf("Sessione scaduta!\n");
    return -1;
}



int closeConnection(const char* sockname){
    if(!strcmp(configurazione.socket_name, sockname)){
        errno = EBADF;
        return -1;
    }
    if(close(socket_c) == -1){
        errno = EBADF;
        return -1;
    }
    printf("Connessione terminata!\n");
    return 0;
}

int openFile(const char* pathname, int flags){
    request r;
    response feedback;
    int b_n;

    memset(&r, 0, sizeof(request));
    memset(&feedback, 0, sizeof(response));
    r.flags = flags;
    r.type = open_file; 
    r.socket_fd = socket_c;
    memset(&r.file_name,0,sizeof(r.file_name));
    if(realpath(pathname,r.file_name) == NULL){
        perror("Error bad path openfile\n");
        return -1;
    }
    if(b_n = writen(socket_c,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }
    if(b_n = readn(socket_c,&feedback,sizeof(response)) == -1){
        errno = EAGAIN;
        return -1;

    }

    switch (feedback){
    
    case OPEN_FILE_SUCCESS:
        printf("File aperto con successo\n");
        return 0;    

    case O_CREATE_SUCCESS:
        printf("File creato con successo\n");
        return 0;                

    case LOCK_FILE_SUCCESS:
        printf("File locked con successo\n");
        return 0;       

    case O_CREATE_NOT_SPECIFIED_AND_FILE_NEXIST:
        errno = ENOENT;
        return -1;

    case CANNOT_CREATE_FILE:
        errno = EPERM;
        return -1;  
        
    case FILE_ALREADY_OPENED:
        return 0;

    case FILE_ALREADY_EXIST:
        errno = EEXIST;
        return -1;

    case CANNOT_ACCESS_FILE_LOCKED:
        errno = EPERM;
        return -1;

    default: break;
    }

    return -1;

}
int readFile(const char* pathname, void** buf, size_t* size);
int readNFiles(int N, const char* dirname);
int writeFile(const char* pathname, const char* dirname);
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);
int lockFile(const char* pathname);
int unlockFile(const char* pathname);
int closeFile(const char* pathname);
int removeFile(const char* pathname);