#ifndef UTILS_H_
#define UTILS_H_

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>


#if !defined(NAME_MAX)
#define NAME_MAX 1000
#endif

#if !defined(O_CREATE)
#define O_CREATE 0
#endif

#if !defined(O_LOCK)
#define O_LOCK 1
#endif



typedef struct sockaddr_un SA;

typedef struct S{
    int n_thread_workers;
    int max_n_file;
    int memory_capacity;
    char socket_name[NAME_MAX];
    char log_file_path[NAME_MAX];
}config;

#define SYSCALL_EXIT(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }

#define MY_REALPATH(from_where,path,resolved_path)\
    if(realpath(path,resolved_path) == NULL){ \
		perror(#from_where);\
		int errno_copy = errno;	 \
        exit(errno_copy); \
	}

#define PRINT_ERRNO(from_where,err) \
    if(err != 0){ \
        perror(#from_where); \
        exit(err); \
    } \

static inline int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=read((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;   
        left    -= r;
	bufptr  += r;
    }
    return size;
}


static inline int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=write((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;  
        left    -= r;
	bufptr  += r;
    }
    return 1;
}


int isNumber(const char* s, long* n);
int isdot(const char dir[]);
char* cwd();
long bytesToMb(long bytes);

#endif 