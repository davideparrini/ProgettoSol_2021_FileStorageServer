#ifndef RESPONSE_H_
#define RESPONSE_H_

#define MAX_LENGHT_FILE 100000
typedef enum{
    O_CREATE_NOT_SPECIFIED_AND_FILE_NEXIST,
    CANNOT_CREATE_FILE,
    FILE_ALREADY_OPENED,
    FILE_ALREADY_EXIST,
    CANNOT_ACCESS_FILE_LOCKED,
    FILE_NOT_EXIST,

    OPEN_FILE_SUCCESS,
    READ_FILE_SUCCESS,
    O_CREATE_SUCCESS,
    LOCK_FILE_SUCCESS,

    NO_SPACE_IN_SERVER,

    FILE_NOT_OPENED

}response_type;

typedef struct s{
    response_type type;
    int size;
    int c; //contatore generico
    char content[MAX_LENGHT_FILE];
}response;

#endif