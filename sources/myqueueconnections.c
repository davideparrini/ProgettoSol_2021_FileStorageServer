#include <myqueueconnections.h>
#include <stdlib.h>
#include <string.h>
//queue for request
static node_t *head = NULL;
static node_t *tail = NULL;


void init_q(int client_socket){

    node_t* q  = (node_t*) malloc(sizeof(node_t));
    memset(q,0,sizeof(node_t));
	q->client_socket = client_socket;
    head = q;
    tail = q;
	q->next = NULL;
}

void push_q(int client_socket){
    if(tail == NULL){
        init_q(client_socket); 
        return;
    }  
    tail->next = (node_t*) malloc(sizeof(node_t));
    memset(tail->next,0,sizeof(node_t));
    tail->next->client_socket = client_socket;
    tail->next->next = NULL;
    tail = tail->next;
}
void removeConnection_q(int client_socket){
    node_t *cor = head;
    node_t *prec = NULL;
    while(cor != NULL){
        if(cor->client_socket == client_socket){
            node_t *toFree = cor; 
            if(prec == NULL){
                cor = cor->next;
                head = cor;
            }
            else{
                prec->next = cor->next;
                cor = cor->next;     
            }
            toFree->next = NULL;
            free(toFree);
            return;
        }
        prec = cor;
        cor = cor->next;
    }
}
int pop_q(){
    if(head == NULL){
        return -1; 
    }
    else{
        int res = head->client_socket;
        node_t *temp = head;
        head = head->next;
        if(head == NULL) tail = NULL;
        free(temp);
        return res;
    }
}

void rmv_q() {
    node_t* q = head;
	if (q == NULL)
		return;
	rmv_q(q->next);
	free(q);
	return;
}

int isEmpty_q(){
    return (head == NULL) ? 1 : 0;
}
