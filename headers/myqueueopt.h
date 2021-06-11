#ifndef MYQUEUEOPT_H_
#define MYQUEUEOPT_H_

#include <utils.h>

typedef struct o__{
    struct o__ *next;
    char opt;
    char* optarg;

}char_t;

void init_char_q(char opt, char* optarg);
void push_char(char opt, char* optarg);
char_t* pop_char();
int isEmpty_charq();
void free_char_t(char_t *c);


#endif