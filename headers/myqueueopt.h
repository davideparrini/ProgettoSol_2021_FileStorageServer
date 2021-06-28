#ifndef MYQUEUEOPT_H_
#define MYQUEUEOPT_H_

#include <utils.h>

typedef struct o__{
    struct o__ *next;
    char opt;
    char* optarg;

} char_t;


typedef struct{
    char_t* head;
    char_t* tail;
    int size;

} char_queue;




void init_char_queue(char_queue* q);
void init_char_t(char opt,char* optarg,char_queue* q);

void push_char(char opt, char* optarg,char_queue* q);
char_t* pop_char(char_queue* q);

int isEmpty_charq(char_queue q);
void free_char_t(char_t *c);


#endif