#ifndef MYQUEUE_H_
#define MYQUEUE_H_
//queue for request
typedef struct node{
    struct node* next;
    int *client_socket;
}node_t;

void init_q(int* client_socket);
void push_q(int *client_socket);
int* pop_q();
void rmv_q();

#endif