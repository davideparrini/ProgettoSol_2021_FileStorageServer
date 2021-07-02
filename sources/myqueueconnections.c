#include <myqueueconnections.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//queue for request
static connection_t *head = NULL;
static connection_t *tail = NULL;


void init_q(int client_socket){

    connection_t* q  = (connection_t*) malloc(sizeof(connection_t));
    memset(q,0,sizeof(connection_t));
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
    tail->next = (connection_t*) malloc(sizeof(connection_t));
    memset(tail->next,0,sizeof(connection_t));
    tail->next->client_socket = client_socket;
    tail->next->next = NULL;
    tail = tail->next;
}
void removeConnection_q(int client_socket){
    connection_t *cor = head;
    connection_t *prec = NULL;
    while(cor != NULL){
        if(cor->client_socket == client_socket){
            connection_t *toFree = cor;
            
            if(prec == NULL){
                cor = cor->next;
                head = cor;
            }
            else{
                prec->next = cor->next;
                cor = cor->next;     
            }
            if(head == NULL) tail = NULL;
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
        connection_t *temp = head;
        head = head->next;
        if(head == NULL) tail = NULL;
        free(temp);
        return res;
    }
}
void print_q(){
    connection_t *temp = head;
    while(temp != NULL){
        printf("Connessione n %d\n",temp->client_socket);
        temp = temp->next;
    }
    printf("Fine printq\n");
}
void rmv_q() {
    connection_t* q = head;
	if (q == NULL)
		return;
	rmv_q(q->next);
	free(q);
	return;
}

int isEmpty_q(){
    return (head == NULL) ? 1 : 0;
}
