#ifndef REQUEST_H_
#define REQUEST_H_

#include "file.h"

typedef enum e{
    
    open_file,
    read_file,
    write_file,
    append_file,
    lock_file,
    unlock_file,
    close_file,
    remove_file

}req_type;

typedef struct f{
    int socket_fd;
    file_t file;
    char request_task[100];
    req_type type;
}request;


#endif