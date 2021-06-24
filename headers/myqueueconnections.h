#ifndef MYQUEUECONNECTIONS_H_
#define MYQUEUECONNECTIONS_H_
//queue for connection
typedef struct node__{
    struct node__* next;
    int client_socket;
}node_t;

void init_q(int client_socket);
void push_q(int client_socket);
int pop_q();
void rmv_q();
int isEmpty_q();
void print_q();
void removeConnection_q(int client_socket);
#endif
