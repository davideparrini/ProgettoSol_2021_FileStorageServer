#ifndef REQUEST_H_
#define REQUEST_H_

#include <utils.h>

typedef enum e{
    
    open_file,
    read_file,
    read_N_file,
    write_file,
    append_file,
    lock_file,
    unlock_file,
    close_file,
    remove_file

}req_type;

typedef struct f{
    int socket_fd;
    int flags;
    int c; //contatore generico
    char file_name[NAME_MAX];
    int request_size;
    req_type type;
}request;


#endif