#include <myqueueopt.h>

void init_char_queue(char_queue* q){
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void init_char_t(char opt,char* optarg,char_queue* q){
    char_t* cq  = (char_t*) malloc(sizeof(char_t));
    memset(cq,0,sizeof(char_t));
	cq->opt = opt;
    cq->optarg = malloc(sizeof(char)*(strlen(optarg)+1));
    memset(cq->optarg,0,sizeof(char)*(strlen(optarg)+1));
    strcpy(cq->optarg,optarg);
    cq->next = NULL;

    q->head = cq;
    q->tail = cq;
    q->size++;
}



void push_char(char opt, char* optarg,char_queue* q){
    if(q->tail == NULL){
        init_char_t(opt,optarg,q); 
        return;
    }  
    q->tail->next = (char_t*) malloc(sizeof(char_t));
    memset(q->tail->next,0,sizeof(char_t));
    q->tail->next->optarg = malloc(sizeof(char)*(strlen(optarg)+1));
    memset(q->tail->next->optarg,0,sizeof(char)*(strlen(optarg)+1));
    
    q->tail->next->opt = opt;
    strcpy(q->tail->next->optarg,optarg);
    q->tail->next->next = NULL;
    q->tail = q->tail->next;
    q->size++;
}
char_t* pop_char(char_queue* q){
    if(q->head == NULL){
        return NULL; 
    }
    else{
        char_t* c = q->head;
        q->head = q->head->next;
        if(q->head == NULL) q->tail = NULL;
        q->size--;
        return c;
    }
}
int isEmpty_charq(char_queue q){
    return (q.head == NULL && !q.size) ? 1 : 0;
}
void free_char_t(char_t *c){
    free(c->optarg);
    free(c);
}

