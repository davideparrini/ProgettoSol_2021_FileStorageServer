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


#define MAX_SOCKET_PATH 256

extern config configurazione;
extern int socket_c;
int flag_stamp_op = 0;
int msec_between_req = 0;

int arg_h(const char* s);
int arg_f(const char* s,char* sockname);
int arg_w(const char* s);
int arg_W(const char* s);
int arg_D(const char* s);
int arg_r(const char* s);
int arg_R(int n);
int arg_d(const char* s);
int arg_t(int mill_sec);
int arg_l(const char* s);
int arg_u(const char* s);
int arg_c(const char* s);
int arg_p();

//PRINT_ERRNO va chiamata qua nel client.c

int main(int argc, char const *argv[]){
    char* socket_name = malloc(sizeof(char)*MAX_SOCKET_PATH);
    memset(socket_name,0,strlen(socket_name));
    int opt;
    struct timespec timer_connection;
    timer_connection.tv_nsec = 0;
    timer_connection.tv_sec = 20;

    
    if(argc < 2){
        printf("Pochi argomenti!\n");
        exit(EXIT_FAILURE);
    }

    if(openConnection(configurazione.n_thread_workers,100,timer_connection) == -1){
        fprintf(stderr, "openConnection Value of errno : %d\n", errno);
        exit(EXIT_FAILURE);
    }

    int r = 0,R = 0, w = 0, W = 0,f = 0, h = 0,p = 0;

    while(opt = getopt(argc, argv,"hf:w:W:D:r:R:d:t:l:u:c:p") != -1){
        switch (opt){
        case 'h': 
            if(h){
                printf("Non si può ripetere l'argomento 'h'!\n");
                break;
            }
            arg_h(argv[0]); 
            msleep(msec_between_req); 
            break;

        case 'f':
            if(f){
                printf("Non si può ripetere l'argomento 'f'!\n");
                break;
            } 
            arg_f(optarg,socket_name); 
            msleep(msec_between_req); 
            break;

        case 'w':   
            arg_w(optarg);
            w = 1; 
            msleep(msec_between_req); 
            break; 

        case 'W': 
            arg_W(optarg); 
            W = 1; 
            msleep(msec_between_req); 
            break;

        case 'D': 
            arg_D(optarg); 
            msleep(msec_between_req); 
            break;

        case 'r': 
            arg_r(optarg); 
            r = 1; 
            msleep(msec_between_req); 
            break;
        case 'R': 
            arg_R(atoi(optarg)); 
            R = 1; 
            msleep(msec_between_req); 
            break;

        case 'd': 
            arg_d(optarg); 
            msleep(msec_between_req); 
            break;

        case 't': 
            arg_t(atoi(optarg)); 
            msleep(msec_between_req); 
            break;

        case 'l': 
            arg_l(optarg);
            msleep(msec_between_req); 
            break;

        case 'u': 
            arg_u(optarg); 
            msleep(msec_between_req); 
            break;

        case 'c': 
            arg_c(optarg); 
            msleep(msec_between_req); 
            break;

        case 'p':
            if(p){
                printf("Non si può ripetere l'argomento 'p'!\n");
                break;
            }  
            arg_p();
            msleep(msec_between_req);  
            break;

        case ':': 
            if(optopt == 'R'){
                int n = 0;
                arg_R(n);
                break;
            }
            if(optopt == 't'){
                int n = 0;
                arg_t(n);
                break;
            }

            printf( "l'opzione '-%c' richiede un argomento\n", optopt);
            break;

        case '?': 
            printf("l'opzione '-%c' non e' gestita\n", optopt);
            break;

        default:
            break;
        }

    }
    msec_between_req = 0;


    if(closeConnection == -1){
        fprintf(stderr, "closeConnection Value of errno : %d\n", errno);
    }

}



int arg_h(const char* s){
    printf("Helper message!\nUsage: %s \n-f filename\n-w dirname[,n=0] \
    \n-W file1[,file2]..\n-D dirname\n-r file1[,file2]..\n-R [n=0]\n-d dirname\n \
    -t time\n-l file1[,file2]..\n-u file1[,file2]..\n-c file1[,file2]..\n-p\n");
    return 0;
}


int arg_f(const char* s,char *sockname){
    memset(sockname,0,MAX_SOCKET_PATH);
    strncpy(sockname,s,MAX_SOCKET_PATH);
    printf("Ora il socket su cui connettersi è : %s\n",sockname);
    return 0;
}

int arg_w(const char* s){
    char* dirname = malloc( sizeof(char) * NAME_MAX);
    memset(dirname,0,sizeof(dirname));
    char* token = strtok(s,",");
    strncpy(dirname,token,NAME_MAX);
    token = strtok(NULL,"");
    int n = atoi(token);
    free(token);




}
int arg_W(const char* s);
int arg_D(const char* s);

int arg_r(const char* s){
    char* token = strtok(s,",");
    char* buff;
    size_t size;
    int res = 0;

    while(token != NULL){
        if(openFile(token,O_NOFLAGS) == -1){
            perror("openFile arg_r");
            return -1;
        }
    
        if(res = readFile(token,&buff,&size) == -1){
            perror("readFile arg_r");
            res = -1;
        }
        pritnf("*Contenuto File:\n%s\n",buff);

        if(flag_stamp_op){
            time_t t_op = time(NULL);
            PRINT_OP(readFile,token,res,&t_op,size);
        } 
        if(closeFile(token) == -1){
            perror("closeFile arg_r");
            return -1;
        }
        memset(buff,0,size);
        token = strtok(NULL,",");
    }
    free(token);
    free(buff);
    return res;
}

int arg_R(int n);
int arg_d(const char* s);

int arg_t(int mill_sec){
    msec_between_req = mill_sec;
    return 0;
}

int arg_l(const char* s);
int arg_u(const char* s);
int arg_c(const char* s){
    int res = 0;
    char* token = strtok(s,",");
    while(token != NULL){
        if(removeFile(token) == -1){
            perror("Errore removeFile");
            res = -1;
        }
        if(flag_stamp_op){
            time_t t_op = time(NULL);
            PRINT_OP(removeFile,token,res,&t_op,0);
        } 
        token = strtok(NULL,",");
    }
    return res; 
}

int arg_p(){
    if(flag_stamp_op) flag_stamp_op = 0;
    else flag_stamp_op = 1;
    return 0;
}
