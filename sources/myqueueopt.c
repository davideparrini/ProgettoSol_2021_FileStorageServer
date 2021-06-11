#include <myqueueopt.h>

static char_t *head = NULL;
static char_t *tail = NULL;

void init_char_q(char opt,char* optarg){
    char_t* cq  = (char_t*) malloc(sizeof(char_t));
    memset(cq,0,sizeof(char_t));
	cq->opt = opt;
    cq->optarg = malloc(sizeof(char)*(strlen(optarg)+1));
    memset(cq->optarg,0,sizeof(cq->optarg));
    strcpy(cq->optarg,optarg);
    head = cq;
    tail = cq;
	cq->next = NULL;
}
void push_char(char opt, char* optarg){
    if(tail == NULL){
        init_char_q(opt,optarg); 
        return;
    }  
    tail->next = (char_t*) malloc(sizeof(char_t));
    memset(tail->next,0,sizeof(char_t));
    tail->next->optarg = malloc(sizeof(char)*(strlen(optarg)+1));
    memset(tail->next->optarg,0,sizeof(tail->next->optarg));
    
    tail->next->opt = opt;
    strcpy(tail->next->optarg,optarg);
    tail->next->next = NULL;
    tail = tail->next;
}
char_t* pop_char(){
    if(head == NULL){
        return NULL; 
    }
    else{
        char_t* c = head;
        head = head->next;
        if(head == NULL) tail = NULL;
        return c;
    }
}
int isEmpty_charq(){
    return (head == NULL) ? 1 : 0;
}

void free_char_t(char_t *c){
    free(c->optarg);
    free(c);
}