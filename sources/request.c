#include <request.h>
static req_t *head = NULL;
static req_t *tail = NULL;

void init_r(request *req_client){

    req_t* r  = (req_t*) malloc(sizeof(req_t));
    memset(r,0,sizeof(req_t));
	r->req = req_client;
    head = r;
    tail = r;
	r->next = NULL;
}
void push_r(request *r){
    if(tail == NULL){
        init_r(r); 
        return;
    }  
    tail->next = (req_t*) malloc(sizeof(req_t));
    memset(tail->next,0,sizeof(req_t));
    tail->next->req = r;
    tail->next->next = NULL;
    tail = tail->next;
}
request* pop_r(){
    if(head == NULL){
        return NULL; 
    }
    else{
        request *res = head->req;
        req_t *temp = head;
        head = head->next;
        if(head == NULL) tail = NULL;
        free(temp);
        return res;
    }
}
void rmv_r(){
    req_t* r = head;
	if (r == NULL)
		return;
	rmv_r(r->next);
	free(r);
	return;
}
int isEmpty_r(){
    return (head == NULL) ? 1 : 0;
}