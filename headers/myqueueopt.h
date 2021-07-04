#ifndef MYQUEUEOPT_H_
#define MYQUEUEOPT_H_

#include <utils.h>

//struttura dati adatta a memorizzare un opzione ed un argomento di una passata di getopt
typedef struct o__{
    struct o__ *next;
    char opt;
    char* optarg;
} char_t;

//coda con doppio riferimento di char_t
typedef struct{
    char_t* head;
    char_t* tail;
    int size;

} char_queue;



//inizializza la char_queue q
void init_char_queue(char_queue* q);

//inizializza con un char_t la char_queue q
void init_char_t(char opt,char* optarg,char_queue* q);

//inserisce in coda un char_t nella char_queue q
void push_char(char opt, char* optarg,char_queue* q);

//rimuove il char_t in testa della char_queue q
char_t* pop_char(char_queue* q);

//verifica se la coda è vuota, == 1 è vuota, == 0 non è vuota
int isEmpty_charq(char_queue q);

//libera memoria del char_t
void free_char_t(char_t *c);


#endif