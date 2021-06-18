#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>  
#include <time.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/select.h>
#include <limits.h>
#include <ctype.h>
#include <signal.h>
#include <libgen.h>
#include <getopt.h>
#include <dirent.h>

#include <request.h>
#include <response.h>


#if !defined(NAME_MAX)
#define NAME_MAX 1000
#endif

#define O_NOFLAGS 00

#define O_CREATE 01

#define O_LOCK 10




#define BUFSIZE 1024
typedef struct sockaddr_un SA;

typedef struct S{
    int n_thread_workers;
    int max_n_file;
    double memory_capacity;
    char socket_name[NAME_MAX];
}config;


#define SYSCALL_EXIT(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }

#define MY_GETCWD(from_where,buf)\
    if(getcwd(buf,sizeof(buf)) == NULL){ \
		perror(#from_where);\
		int errno_copy = errno;	 \
        exit(errno_copy); \
	}
    
#define PRINT_ERRNO(from_where,err) \
    if(err != 0){ \
        perror(#from_where); \
        printf("Errno value: %d\n",errno); \
    } 

#define PRINT_OP(nome_operazione,file_riferimento,orario,esito,bytes) \
    printf("Tipo operazione: %s\nSul file: %s\nData/Ora: %sEsito: ",nome_operazione,file_riferimento,ctime(orario) );\
    if(!esito){ \
        printf("Successo!\n"); \
        if(bytes != 0) printf("bytes letti/scritti : %lubytes\n\n",(unsigned long)bytes);        \
        else printf("\n");        \
    }           \
    else printf("Fallimento!\n\n")

int readn(long fd, void *buf, size_t size);
int writen(long fd, void *buf, size_t size);

int msleep(unsigned int tms);
int isNumber(const char* s, long* n);
int isdot(const char dir[]);


double bytesToKb(size_t bytes);
double bytesToMb(size_t bytes);
size_t KbToBytes(double Kb);
size_t MbToBytes(double Mb);
int findFile_getAbsPath(const char nomedir[], const char nomefile[],char **resolvedpath);
int findDir_getAbsPath(const char dirPartenza[], const char dirToSearch[],char **resolvedpath);

int sendContent(int socket_fd,void* content,size_t dim,int needFlagsOk);
int getContent(int client_fd, void *buff);
#endif 