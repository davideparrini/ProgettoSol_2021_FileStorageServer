#ifndef MYQUEUE_H_
#define MYQUEUE_H_

typedef struct node{
    struct node* next;
    int *client_socket;
}node_t;

void push_q(int *client_socket);
int* pop_q();

#endif
