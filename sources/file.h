#ifndef FILE_H_
#define FILE_H_

#include <time.h>

typedef struct f{
    
    time_t last_modified;
    char *pathaname;
    int dim;

}file_t;


#endif