#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <request.h>
#include <utils.h>
#include <myhashstoragefile.h>
#include <serverapi.h>


#define MAX_SOCKET_PATH 512

extern config configurazione;
extern int socket_c;

int arg_h(const char* s);
int arg_f(const char* s,char* sockname);
int arg_w(const char* s);
int arg_W(const char* s);
int arg_D(const char* s);
int arg_r(const char* s);
int arg_R(const char* s);
int arg_d(const char* s);
int arg_t(const char* s);
int arg_l(const char* s);
int arg_u(const char* s);
int arg_c(const char* s);
int arg_p(const char* s);



int main(int argc, char const *argv[]){
    char* socket_name = malloc(sizeof(char)*MAX_SOCKET_PATH);
    int opt;
    struct timespec t;
    t.tv_nsec = 0;
    t.tv_sec = 20;
    if(argc < 2){
        printf("Pochi argomenti!\n");
        exit(EXIT_FAILURE);
    }


    while(opt = getopt(argc, argv,"hf:w:W:D:r:R:d:t:l:u:c:p") != -1){
        switch (opt){
        case 'h': arg_h(argv[0]);  break;
        case 'f': arg_f(optarg,socket_name);  break;
        case 'w': arg_w(optarg);  break; 
        case 'W': arg_W(optarg);  break;
        case 'D': arg_D(optarg);  break;
        case 'r': arg_r(optarg);  break;
        case 'R': arg_R(optarg);  break;
        case 'd': arg_d(optarg);  break;
        case 't': arg_t(optarg);  break;
        case 'l': arg_l(optarg);  break;
        case 'u': arg_u(optarg);  break;
        case 'c': arg_c(optarg);  break;
        case 'p': arg_p(optarg);  break;
        case ':': {
            if(optopt == 'R'){
                char z = '0';
                arg_R(z);
            }
            printf( "l'opzione '-%c' richiede un argomento\n", optopt);
        } break;
        case '?': {
            printf("l'opzione '-%c' non e' gestita\n", optopt);
        } break;
        default:
            break;
        }

    }
    if(openConnection(configurazione.n_thread_workers,100,t) == -1){
        fprintf(stderr, "openConnection Value of errno : %d\n", errno);
        exit(EXIT_FAILURE);
    }


    if(closeConnection == -1){
        fprintf(stderr, "closeConnection Value of errno : %d\n", errno);
    }

}



int arg_h(const char* s){
    printf("Helper message!\nUsage: %s \n-f filename\n-w dirname[,n=0] \
    \n-W file1[,file2]..\n-D dirname\n-r file1[,file2]..\n-R [n=0]\n-d dirname\n \
    -t time\n-l file1[,file2]..\n-u file1[,file2]..\n-c file1[,file2]..\n-p\n");
    return -1;
}


int arg_f(const char* s,char *sockname){
    memset(sockname,0,MAX_SOCKET_PATH);
    strncpy(sockname,s,MAX_SOCKET_PATH);
    printf("Ora il socket su cui connettersi è : %s\n",sockname);
    return -1;
}

int arg_w(const char* s){


}
int arg_W(const char* s);
int arg_D(const char* s);
int arg_r(const char* s);
int arg_R(const char* s);
int arg_d(const char* s);
int arg_t(const char* s);
int arg_l(const char* s);
int arg_u(const char* s);
int arg_c(const char* s);
int arg_p(const char* s);
