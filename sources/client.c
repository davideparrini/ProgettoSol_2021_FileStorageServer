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
int arg_D(const char* s,char* temp_optarg);
int arg_r(const char* s);
int arg_R(int n);
int arg_d(const char* s,char* temp_optarg);
int arg_t(int mill_sec);
int arg_l(const char* s);
int arg_u(const char* s);
int arg_c(const char* s);
int arg_p();


int main(int argc, char const *argv[]){
    char* socket_name = malloc(sizeof(char)*MAX_SOCKET_PATH);
    memset(socket_name,0,strlen(socket_name));
    
    struct timespec timer_connection;
    timer_connection.tv_nsec = 0;
    timer_connection.tv_sec = 20;

    
    if(argc < 2){
        printf("Pochi argomenti!\n");
        exit(EXIT_FAILURE);
    }

    if(openConnection(configurazione.n_thread_workers,100,timer_connection) == -1){
        perror("Non connesso, errore in openConnection");
        exit(EXIT_FAILURE);
    }

    char* temp_optarg = malloc(sizeof(char)*NAME_MAX);
    int opt, temp_opt = 0;
    int r = 0,R = 0, w = 0, W = 0,f = 0, h = 0,p = 0,d = 0,D =0;

    while(opt = getopt(argc, argv,"hf:w:W:D:r:R:d:t:l:u:c:p") != -1){
        if(temp_opt != 0){
            switch (temp_opt){

            case 'r':
                if(opt == 'd') arg_d(optarg,temp_optarg);
                else arg_r(temp_optarg);
                break;

            case 'R':
                if(opt == 'd') arg_d(optarg,temp_optarg);
                else arg_R(atoi(temp_optarg));
                break;

            case 'w':
                if(opt == 'D') arg_D(optarg,temp_optarg);
                else arg_w(temp_optarg);
                break;

            case 'W':
                if(opt == 'D') arg_D(optarg,temp_optarg);
                else arg_W(temp_optarg);
                break;
            
            default: break;
            }
        }
        switch (opt){
        case 'h': 
            if(h){
                printf("Non si può ripetere l'argomento 'h'!\n");
                break;
            }
            arg_h(argv[0]);
            break;

        case 'f':
            if(f){
                printf("Non si può ripetere l'argomento 'f'!\n");
                break;
            } 
            arg_f(optarg,socket_name);
            break;

        case 'w':   
            memset(temp_optarg,0,sizeof(temp_optarg));
            strcpy(temp_optarg,optarg);
            break; 

        case 'W': 
            memset(temp_optarg,0,sizeof(temp_optarg));
            strcpy(temp_optarg,optarg);
            break;

        case 'D': 
            if(temp_opt != 'w' && temp_opt != 'W'){
                printf("Errore! Opzione D non può essere utilizzata senza prima aver utilizzato -w o -W\n");
                break;
            }
            break;

        case 'r':
            memset(temp_optarg,0,sizeof(temp_optarg));
            strcpy(temp_optarg,optarg);
            break;

        case 'R':
            memset(temp_optarg,0,sizeof(temp_optarg));
            strcpy(temp_optarg,optarg); 
            break;

        case 'd': 
            if(!r && !R ){
                printf("Errore! Opzione d non può essere utilizzata senza prima aver utilizzato -r o -R\n");
                break;
            }
            break;

        case 't': 
            arg_t(atoi(optarg)); 
            break;

        case 'l': 
            arg_l(optarg);
            break;

        case 'u': 
            arg_u(optarg); 
            break;

        case 'c': 
            arg_c(optarg);
            break;

        case 'p':
            if(p){
                printf("Non si può ripetere l'argomento 'p'!\n");
                break;
            }  
            arg_p();           
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

        default: break;
        }
        temp_opt = opt;
        msleep(msec_between_req);  
    }

    switch(temp_opt){

    case 'r':
        arg_r(temp_optarg);
        break;

    case 'R':
        arg_R(atoi(temp_optarg));
        break;

    case 'w':
        arg_w(temp_optarg);
        break;

    case 'W':
        arg_W(temp_optarg);
        break;

    default: break;            
    }
    msec_between_req = 0;
    r = 0,R = 0, w = 0, W = 0,f = 0, h = 0,p = 0;



    if(closeConnection(socket_c) == -1){
        fprintf(stderr, "closeConnection Value of errno : %d\n", errno);
    }

}



int arg_h(const char* s){
    printf("Helper message!\n\nUsage: %s\n\n",s);
    printf("-f filename\tspecifica il nome del socket AF_UNIX a cui connettersi\n\n");
    printf("-w dirname[,n=0]\tinvia al server n (se non specificato o 0 invia tutti i file) file nella cartella ‘dirname’ a seguito di capacity misses. Può essere utilizzato solo in seguito ai comandi '-w' o '-W'\n\n");
    printf("-W file1[,file2]..\t lista di nomi di file da scrivere nel server separati da ‘,’(esempio: -r pippo,pluto,minni)\n\n");
    printf("-D dirname\t  cartella in memoria secondaria dove vengono scritti (lato client) i file che il server rimuove a\n\n");
    printf("-r file1[,file2]..\tlista di nomi di file da leggere dal server separati da ‘,’ (esempio: -r pippo,pluto,minni)\n\n");
    printf("-R [n=0]\t tale opzione permette di leggere ‘n’ file qualsiasi attualmente memorizzati nel server; se n=0 (o non è specificato) allora vengono letti tutti i file presenti nel server\n\n");
    printf("-d dirname\tcartella in memoria secondaria dove scrivere i file letti dal server con l’opzione ‘-r’ o ‘-R’. Può essere utilizzato solo in seguito ai comandi '-r' o '-R'\n\n");
    printf("-t time\ttempo in millisecondi che intercorre tra l’invio di due richieste successive al server, se non specificato è pari a 0.0s\n\n");
    printf("-l file1[,file2]..\t lista di nomi di file su cui acquisire la mutua esclusione\n\n");
    printf("-u file1[,file2]..\tlista di nomi di file su cui rilasciare la mutua esclusione\n\n");
    printf("-c file1[,file2]..\tlista di file da rimuovere dal server se presenti\n\n");
    printf("-p\tabilita le stampe sullo standard output per ogni operazione\n\n");
    printf("\nLe opzioni '-h' '-p' '-f' non possono essere ripetute, eventuali ripetizioni verranno ignorate\n\n");
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
int arg_D(const char* s,char* temp_optarg);

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
int arg_d(const char* s,char* temp_optarg);

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
