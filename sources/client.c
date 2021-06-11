#include <utils.h>
#include <serverapi.h>
#include <myqueueopt.h>

#define MAX_SOCKET_PATH 256

char* socket_path;
static int flag_stamp_op = 0;
static int msec_between_req = 0;

int arg_h(char* s);
int arg_f(char* s,char* sockname);
int arg_w(char* s,char* dir_rejectedFile);
int writeFileDir(char* dirpath,char* dir_rejectedFile,int n,int flag_end,size_t* written_bytes);
int arg_W(char* s,char* dir_rejectedFile);
int arg_r(char* s,char* dirname);
int arg_R(int n,char* dirname);
int arg_t(int mill_sec);
//int arg_l(char* s);
//int arg_u(char* s);
int arg_c(char* s);
int arg_p();

int arg_o(char* s);
int arg_O(int n);
int arg_a(char* s);

int main(int argc, char *argv[]){
    socket_path = malloc(sizeof(char)*MAX_SOCKET_PATH);
    memset(socket_path,0,strlen(socket_path));
    
    struct timespec timer_connection;
    timer_connection.tv_nsec = 0;
    timer_connection.tv_sec = 20;

    
    if(argc < 2){
        printf("Pochi argomenti!\n");
        exit(EXIT_FAILURE);
    }

    int opt, temp_opt = 0;
    int termina = 0;
    int f = 0, h = 0,p = 0;

    while((opt = getopt(argc, argv,"hf:w:W:D:r:R:d:t:l:u:c:po:O:a:")) != -1 && !termina){

        switch (opt){
        case 'h': 
            if(h){
                printf("Non si può ripetere l'argomento 'h'!\n");
                break;
            }
            arg_h(argv[0]);
            exit(EXIT_SUCCESS);

        case 'f':
            if(f){
                printf("Non si può ripetere l'argomento 'f'!\n");
                break;
            } 
            f = 1;
            arg_f(optarg,socket_path);
            break;

        case 'w':   
            push_char(opt,optarg);
            break; 

        case 'W': 
            push_char(opt,optarg);
            break;

        case 'D': 
            if(temp_opt != 'w' && temp_opt != 'W'){
                printf("Errore! Opzione D non può essere utilizzata senza prima aver utilizzato -w o -W\n");
                termina = 1;
                break;
            }
            push_char(opt,optarg);
            break;

        case 'r':
            push_char(opt,optarg);
            break;

        case 'R':
            push_char(opt,optarg); 
            break;

        case 'd': 
             if(temp_opt != 'r' && temp_opt != 'R'){
                printf("Errore! Opzione d non può essere utilizzata senza prima aver utilizzato -r o -R\n");
                termina = 1;
                break;
            }
            push_char(opt,optarg);
            break;

        case 't': 
            arg_t(atoi(optarg)); 
            break;

        case 'l': 
            //push_char(opt,optarg);
            break;

        case 'u': 
            //push_char(opt,optarg); 
            break;

        case 'c': 
            push_char(opt,optarg);
            break;

        case 'p':
            if(p){
                printf("Non si può ripetere l'argomento 'p'!\n");
                break;
            }  
            p = 1;
            arg_p();           
            break;

        case 'o': 
            push_char(opt,optarg);
            break;

        case 'O': 
            push_char(opt,optarg);
            break;

        case 'a': 
            push_char(opt,optarg);
            break;

        case ':': 
            if(optopt == 'R'){
                push_char(opt,"0");
                break;
            }
            if(optopt == 'O'){
                push_char(opt,"0");
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
    }
    if(!f){
        printf("Nome del socket non è stato passato!\nL'opzione '-f' è obbligatoria!\n");
        exit(EXIT_FAILURE);
    }

    if(openConnection(socket_path,100,timer_connection) == -1){
        perror("Non connesso, errore in openConnection");
        exit(EXIT_FAILURE);
    }
    while(!isEmpty_charq()){
        char_t* c = pop_char();
        switch (c->opt){
        case 'w':   
            if(c->next->opt == 'd') arg_w(c->optarg,c->next->optarg);
            else arg_w(c->optarg,NULL);
            break; 

        case 'W': 
            if(c->next->opt == 'd') arg_w(c->optarg,c->next->optarg);
            else arg_w(c->optarg,NULL);
            break;

        case 'D': 
            break;

        case 'r':
            if(c->next->opt == 'd') arg_r(c->optarg,c->next->optarg);
            else arg_r(c->optarg,NULL);
            break;

        case 'R':
            if(c->next->opt == 'd') arg_R(atoi(c->optarg),c->next->optarg);
            else arg_R(atoi(c->optarg),NULL); 
            break;

        case 'd': 
            break;

        case 'l': 
            //arg_l(c->optarg);
            break;

        case 'u': 
            //arg_u(c->optarg);
            break;

        case 'c': 
            arg_c(c->optarg);
            break;

        case 'o': 
            arg_o(c->optarg);
            break;

        case 'O': 
            arg_O(atoi(c->optarg));
            break;

        case 'a': 
            arg_a(c->optarg);
            break;

        default:
            break;
        }
        free_char_t(c);
        msleep(msec_between_req);  
    }


    if(closeConnection(socket_path) == -1){
        fprintf(stderr, "closeConnection Value of errno : %d\n", errno);
    }

}



int arg_h(char* s){
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


int arg_f(char* s,char *sockname){
    memset(sockname,0,MAX_SOCKET_PATH);
    strncpy(sockname,s,MAX_SOCKET_PATH);
    printf("Ora il socket su cui connettersi è : %s\n",sockname);
    return 0;
}

int arg_w(char* s,char* dir_rejectedFile){
    int esito = 0;
    size_t contatore_bytes_scritti = 0;
    char* dirname = strtok(s,",");
    char absPathdir[NAME_MAX];
    memset(absPathdir,0,sizeof(absPathdir));
    char* nfile = strtok(NULL,",");
    MY_REALPATH("arg_w",dirname,absPathdir);
    int n, flag_end = 0;
    if(nfile != NULL) n = atoi(nfile);
    else n = 0;
    if(n == 0) flag_end = 1;

    esito = writeFileDir(absPathdir,dir_rejectedFile,n,flag_end,&contatore_bytes_scritti);
    if(flag_stamp_op){
            time_t t_op = time(NULL);
            PRINT_OP("arg_w","writeNfile",&t_op,esito,contatore_bytes_scritti);
        } 

    return esito;
}

int writeFileDir(char* dirpath,char* dir_rejectedFile,int n,int flag_end,size_t* written_bytes){
    if (chdir(dirpath) == -1) {
        printf("Errore cambio directory\n");
        return -1;
    }
    DIR * dir;
    if((dir = opendir(".")) == NULL){
        perror("Aprendo cwd in writeFileDir");
        return -1;
    }
    struct dirent *file;
    while(n>0 && (errno=0, file = readdir(dir)) != NULL) {  
        struct stat statbuf;
        if (stat(file->d_name, &statbuf)==-1) {	
            perror("stat in writeFileDir");
            return -1;
        }    
        if(S_ISDIR(statbuf.st_mode)){
            if(!isdot(file->d_name)){
                if(writeFileDir(file->d_name,dir_rejectedFile,n,flag_end,written_bytes) != -1){
                    if (chdir("..") == -1) {
                        printf("Impossibile risalire alla directory padre.\n");
                        return -1;
                    }
                }
            }
        }
        else{
            if(!writeFile(file->d_name,dir_rejectedFile)){
                *written_bytes += statbuf.st_size;
                if(flag_end) n++;
                n--;
            }
        }
    }
    if (errno != 0) perror("readdir in writeFileDir");
    closedir(dir);
    return 0;
}
int arg_W(char* s,char* dir_rejectedFile){
    int esito = 0;
    char* token = strtok(s,",");
    size_t size = 0;
    struct stat statbuf;
    while(token != NULL){
        if((esito = writeFile(token,dir_rejectedFile)) == -1){
            perror("readFile arg_r");
            esito = -1;
        }
        if (stat(token, &statbuf)==-1) {	
            perror("stat in writeFileDir");
            esito = -1;
        }
        size += statbuf.st_size;
        token = strtok(NULL,",");
    }
    if(flag_stamp_op){
            time_t t_op = time(NULL);
            PRINT_OP("arg_W writeFile",s,&t_op,esito,size);
        }
    free(token);
    return esito;
}

int arg_r(char* s,char* dirname){
    char* token = strtok(s,",");
    char absPath[NAME_MAX];
    memset(absPath,0,sizeof(absPath));
    char* buff;
    size_t size;
    int esito = 0;

    while(token != NULL && !esito){
        if((esito = readFile(token,(void**)&buff,&size)) == -1){
            perror("readFile arg_r");
        }
        printf("*Contenuto File:\n%s\n",buff);  
        if(dirname != NULL){
            MY_REALPATH("arg_r",token,absPath);
            strcat(absPath,dirname);
            FILE* new;
            
            if((new = fopen(absPath,"a")) == NULL){
                perror("Errore creazione file in arg_r");
                esito = -1;
            }
            struct stat statbuf;
            if(stat(token, &statbuf)==-1) {	
                perror("stat in arg_r");
                esito = -1;
            }
            if(!statbuf.st_size) fputs(buff,new);
            else printf("File %s già presente nella dir %s\n",token,dirname);
            
            fclose(new);
        }    
        memset(buff,0,size);
        memset(absPath,0,sizeof(absPath));
        token = strtok(NULL,",");
    }
    free(token);
    free(buff);
    if(flag_stamp_op){
        time_t t_op = time(NULL);
        PRINT_OP("readFile",s,&t_op,esito,size);
    } 
    return esito;
}
int arg_R(int n, char* dirname){
    int termina = 0,esito = 0,nzero = 0;
    if(nzero == 0) nzero = 1;
    while(!termina && n > 0){

    }
    if((esito = readNFiles(n,dirname)) == -1){
        perror("readNFile arg_r");
    }
    if(flag_stamp_op){
        time_t t_op = time(NULL);
        size_t size = 0;
        PRINT_OP("arg_R","N file",&t_op,esito,size);
    } 
    return esito;
}

int arg_t(int mill_sec){
    msec_between_req = mill_sec;
    return 0;
}

int arg_c(char* s){
    int esito = 0;
    char* token = strtok(s,",");
    while(token != NULL && !esito){
        if((esito = removeFile(token)) == -1){
            perror("Errore removeFile");
        }
        if(flag_stamp_op){
            time_t t_op = time(NULL);
            size_t size = 0;
            PRINT_OP("removeFile",token,&t_op,esito,size);
        } 
        token = strtok(NULL,",");
    }
    return esito; 
}

int arg_p(){
    if(flag_stamp_op) flag_stamp_op = 0;
    else flag_stamp_op = 1;
    return 0;
}

int arg_o(char* s){
    int esito = 0,o_flag = O_NOFLAGS;
    char* flag = strtok(s," ");

    if(strcmp(flag,"O_CREATE")) o_flag = O_CREATE;
    if(strcmp(flag,"O_LOCK")) o_flag = O_LOCK;
    if(strcmp(flag,"O_NOFLAGS")) o_flag = O_NOFLAGS;


    char* token = strtok(NULL,",");
    while(token != NULL && !esito){
        if((esito = openFile(token,o_flag)) == -1){
            perror("Errore openFile");
        }
        if(flag_stamp_op){
            time_t t_op = time(NULL);
            size_t size = 0;
            PRINT_OP("openFile",token,&t_op,esito,size);
        } 
        token = strtok(NULL,",");
    }
    return esito; 
}
int arg_O(int n){
    int esito = 0, termina = 0, nzero = 0;
    if(n == 0) nzero = 1;
    while(!termina && n > 0){
        
    }
    return esito;
}

int arg_a(char* s){
    int esito = 0;
    char* token = strtok(s," : ");
    char* content = strtok(NULL,"^?^?^""£_");
    if(appendToFile(token,(void*)content,sizeof(content),NULL) == -1){
        perror("Errore appendToFile");
        esito = -1;
    }
    if(flag_stamp_op){
        time_t t_op = time(NULL);
        PRINT_OP("appendToFile",token,&t_op,esito,sizeof(content));
    } 

    return esito; 
}