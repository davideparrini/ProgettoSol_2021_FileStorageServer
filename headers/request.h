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
    char *file_name;
    char *dirname;
    void *buff;
    size_t request_size;
    req_type type;
}request;


#endif