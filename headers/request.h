#ifndef REQUEST_H_
#define REQUEST_H_

#include <utils.h>

typedef enum e{
    
    OPEN_FILE,
    READ_FILE,
    READ_N_FILE,
    WRITE_FILE,
    APPEND_FILE,
    LOCK_FILE,
    UNLOCK_FILE,
    CLOSE_FILE,
    REMOVE_FILE

}req_type;

typedef struct f{
    int socket_fd;
    int flags;
    int c; //contatore generico
    char pathfile[NAME_MAX];
    char dirpath[NAME_MAX];
    void *buff;
    size_t request_size;
    req_type type;
}request;

typedef struct r__{
    struct r__* next;
    request *req;
}req_t;

void init_r(request *r);
void push_r(request *r);
request* pop_r();
int isEmpty_r();
void free_request();
void rmv_r();

#endif