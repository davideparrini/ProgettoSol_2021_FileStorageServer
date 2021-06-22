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

int getlistFiletoReject_createfileInDir(const char *dirname,int fd_receptor);

extern char* socket_path;
extern int client_fd;
extern size_t print_bytes_readNFiles;

int openConnection(const char* sockname, int msec, const struct timespec abstime){
    SA sockaddr;
    memset(&sockaddr, 0, sizeof(SA));
    strcpy(sockaddr.sun_path, sockname);
    sockaddr.sun_family = AF_UNIX; 

    if((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1){
        perror("Errore Creazione socket_c");
        exit(EXIT_FAILURE);
    }
    time_t before = time(NULL);
    time_t diff = 0;
    int tentativi = 0;
    time_t tempo_rimanente;
    while(abstime.tv_sec > diff){
         
        if(connect(client_fd,(struct sockaddr*)&sockaddr,sizeof(sockaddr)) != -1){
            printf("Connesso al server!\n\n");
            return 0;
        }
        else{
            if(errno == ENOTCONN || errno == ECONNREFUSED || errno == ENOENT){
                tentativi++;
                tempo_rimanente = abstime.tv_sec - diff;
                printf("Non connesso, il server non è ancora pronto!\nTentativo numero: %d\tCountdown alla chiusura della connessione: %lds\n\n",tentativi,tempo_rimanente);
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

    if(strcmp(socket_path, sockname) != 0){
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
    memset(&r.pathfile,0,NAME_MAX);

    strncpy(r.pathfile,pathname,NAME_MAX);

    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }

    if(b = readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }
    switch (feedback.type){
    
    case OPEN_FILE_SUCCESS:
        printf("File ' %s '  aperto con successo\n",pathname);
        break;     

    case O_CREATE_SUCCESS:
        printf("File ' %s ' o_creato con successo\n",pathname);
        break;                 

    case LOCK_FILE_SUCCESS:
        printf("File ' %s '  o_locked con successo\n",pathname);
        break;       

     case O_CREATE_LOCK_SUCCESS:
        printf("File ' %s '  o_creato/locked con successo\n",pathname);
        break;       

    case O_CREATE_NOT_SPECIFIED_AND_FILE_NEXIST:
        errno = ENOENT;
        return -1;

    case NO_SPACE_IN_SERVER:
        errno = ENOMEM;
        return -1;  
        
    case FILE_ALREADY_OPENED:        
        printf("File ' %s ' è già aperto\n",pathname);
        break;

    case FILE_ALREADY_EXIST:
        errno = EEXIST;
        return -1;

    case CANNOT_ACCESS_FILE_LOCKED:
        errno = EPERM;
        return -1;

    case GENERIC_ERROR:
		errno = EINTR;
        return -1;

    
    case FILE_NOT_EXIST:
		errno = ENOENT;
        return -1;

    default: break;

    }

    errno = 0; 
    return 0;
}

int readFile(const char* pathname, void** buf, size_t* size){
    int b, flag_ok;
    request r;
    response feedback;

    memset(&r,0,sizeof(request));
    memset(&r.pathfile,0,NAME_MAX);
    memset(&feedback, 0, sizeof(response));
    r.type = READ_FILE; 

    strncpy(r.pathfile,pathname,NAME_MAX);


    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }

    if(readn(client_fd,&flag_ok,sizeof(int)) == -1){
        perror("readn flag_ok in readFile");
        exit(EXIT_FAILURE);
    }

    if(flag_ok){

        size_t len;

        if(readn(client_fd,&len,sizeof(size_t)) == -1){
            perror("GET CONTENT ERROR IN WRITEN");
            exit(EXIT_FAILURE);
        }
        
        char content[len];
        memset(content,0,len);

        if(readn(client_fd,&content,len) == -1){
            perror("GET CONTENT ERROR IN WRITEN 2");
            exit(EXIT_FAILURE);
        }

        *size = len;
        *buf = malloc(len);
        memset(*buf, 0, *size);
        memcpy(*buf,content,len);
    }
    if(b = readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }

    switch (feedback.type){
        
    case FILE_NOT_OPEN:
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

    case CANNOT_READ_EMPTY_FILE:
        *buf = NULL;
		*size = 0;
        errno = ENODATA;
        return -1;

    case READ_FILE_SUCCESS:
        printf("File ' %s ' letto con successo\n", pathname);
        break;
          
    default: break;
    }
    errno = 0;
    return 0;

}
int readNFiles(int N, const char* dirname){
    request r;
    response feedback;
    int flag_dirname = 0;
    memset(&r,0,sizeof(request));
    memset(&feedback, 0, sizeof(response));

    if(strlen(dirname) != 0) {
        flag_dirname = 1;
    }
    r.type = READ_N_FILE; 
    r.c = N;

    if(writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }

    int n_to_read = 0;
    if(readn(client_fd,&n_to_read,sizeof(int)) == -1){
        errno = EAGAIN;
        return -1;
    }
    int i = 1;
    while(i <= n_to_read){

        char pathfile[NAME_MAX];
        if(readn(client_fd,&pathfile,NAME_MAX) == -1){
            errno = EAGAIN;
        }
        char* namefile = basename(pathfile);

        size_t buff_size = 0; 

        if( readn( client_fd, &buff_size, sizeof(size_t) ) == -1){
            errno = EAGAIN;
        }

        print_bytes_readNFiles += buff_size;

        if(!buff_size) buff_size = 18;

        char content[buff_size];
        memset(content,0,buff_size);

        if( readn( client_fd, &content, buff_size) == -1){
            errno = EAGAIN;
        }

        printf("****Contenuto file %d ' %s ':****\n%s\n\n", i, namefile, content);
        
        if(flag_dirname){
            char* s_dir = strdup(dirname);
            strcat(s_dir,"/");
            char* newfile_path = strcat(s_dir,namefile);
            int fd_new;
            if((fd_new = creat(newfile_path,0777)) == -1){
                perror("Errore creazione file in readNfiles");
                exit(EXIT_FAILURE);
            }

            if( write( fd_new, content, buff_size ) == -1){
                perror("Errore write in readNfiles");
                exit(EXIT_FAILURE);
            }
            close(fd_new);
            free(s_dir);
        }
        i++;
    }

    if(readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }

    if(n_to_read == 0){
        errno = ENODATA;
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
    int flag_dirname  = 0;
    request r;
    response feedback;
    memset(&r,0,sizeof(request));
    memset(&r.pathfile,0,sizeof(r.pathfile));
    memset(&feedback, 0, sizeof(response));


    if(strlen(dirname) != 0) flag_dirname = 1;

    r.flags = flag_dirname;
    r.type = WRITE_FILE;

    strncpy(r.pathfile,pathname,NAME_MAX);


    if(writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }

    if(flag_dirname) getlistFiletoReject_createfileInDir(dirname,client_fd);

    if(readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }

    switch (feedback.type){
    
    case WRITE_FILE_FAILURE:
        errno = EPERM;
        return -1;

    case FILE_NOT_EXIST:
        errno = ENOENT;
        return -1;

    case NO_SPACE_IN_SERVER:
        errno = ENOMEM;
        return -1;

    case CANNOT_SEND_FILES_REJECTED_BY_SERVER:
		errno = ECOMM;
        return -1;

    case WRITE_FILE_SUCCESS:
        printf("Write_file ' %s ' ha avuto successo\n",pathname);
        break;
            
    default:
        break;
    } 
        
    errno = 0;
    return 0;
}

int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname){

    int flag_dirname = 0, flags_ok; //se i flag open e lock  del file sono settati su open = 1 e lock = 0
                        // allora flag_ok = 1 (avvio la procedura di invio del contenuto da append ), altrimenti 0 
    request r;
    response feedback;
    memset(&r,0,sizeof(request));
    memset(&r.pathfile,0,sizeof(r.pathfile)); 
    memset(&feedback, 0, sizeof(response));
    strncpy(r.pathfile,pathname,NAME_MAX);
    if(strlen(dirname) != 0) flag_dirname = 1;

    r.request_size = size;
    r.type = APPEND_FILE;
    r.flags = flag_dirname;
    
    

    if(writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }

    if(readn(client_fd,&flags_ok,sizeof(int)) == -1){
        errno = EAGAIN;
        return 0;
    }

    if(flags_ok){
        char content[size]; 
        memset(content,0,size);
        memcpy(content,buf,size);
        if(writen(client_fd,content,size) == -1){
            perror("SEND CONTENT ERROR IN WRITEN");
            return 0;
        }

        if(flag_dirname) getlistFiletoReject_createfileInDir(dirname,client_fd);
        
    }

    if(readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }

    switch (feedback.type){
        
    case FILE_NOT_OPEN:
		errno = EACCES;
        return -1;

    case FILE_NOT_EXIST:
		errno = ENOENT;
        return -1;

    case CANNOT_ACCESS_FILE_LOCKED:
        errno = EPERM;
        return -1;

    case NO_SPACE_IN_SERVER:
		errno = ENOMEM;
        return -1;

    case CANNOT_SEND_FILES_REJECTED_BY_SERVER:
		errno = ECOMM;
        return -1;

    case GENERIC_ERROR:
		errno = EINTR;
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
    memset(&r.pathfile,0,sizeof(r.pathfile));;
    memset(&feedback, 0, sizeof(response));

    r.type = CLOSE_FILE;

    if(!strlen(pathname)){
        errno = ENOENT;
        return -1;
    }

    strncpy(r.pathfile,pathname,NAME_MAX);


    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }
    
    if(b = readn(client_fd,&feedback,sizeof(feedback)) == -1){
        errno = EAGAIN;
        return -1;
    }
    switch (feedback.type){
        
    case FILE_NOT_OPEN:
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
    memset(&r.pathfile,0,sizeof(r.pathfile));
    memset(&feedback, 0, sizeof(response));
    r.type = REMOVE_FILE;

    if(!strlen(pathname)){
        errno = ENOENT;
        return -1;
    }
    strncpy(r.pathfile,pathname,NAME_MAX);

    if(b = writen(client_fd,&r,sizeof(r)) == -1){
        errno = EAGAIN;
        return -1;
    }
    
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




int getlistFiletoReject_createfileInDir(const char *dirname,int fd_receptor){

	int nToWrite;
	if(readn(fd_receptor,&nToWrite,sizeof(int)) == -1){
		errno = EAGAIN;
		return -1;
	}

	char* s_dir = strdup(dirname);
	strcat(s_dir,"/");

	while(nToWrite > 0){
		char pathfile[NAME_MAX-1];
		memset(pathfile,0,NAME_MAX);

		if(readn(fd_receptor,&pathfile,NAME_MAX) == -1){
			errno = EAGAIN;
			return -1;
		}

		size_t sizeFile;

		if(readn(fd_receptor,&sizeFile,sizeof(size_t)) == -1){
			errno = EAGAIN;
			return -1;
		}
		char content[sizeFile-1];
		memset(content,0,sizeFile);

		if(readn(fd_receptor,&content,sizeFile) == -1){
			errno = EAGAIN;
			return -1;
		}


		char* namefile = basename(pathfile);
		char* newfile_path = strcat(s_dir,namefile);
		
		int fd_new,lung;
		if((fd_new = creat(newfile_path,0777)) == -1){
			perror("Errore creazione file in readNfiles");
			exit(EXIT_FAILURE);
		}
		if( write(fd_new, content,sizeFile) == -1){
			perror("Errore wrtite in createFiles_fromDupList_inDir");
			exit(EXIT_FAILURE);
		}
        close(fd_new);
        free(newfile_path);
        //free(&content);
    }

  free(s_dir);
	
}