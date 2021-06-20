#include <utils.h>
#include <serverapi.h>
#include <myqueueopt.h>

#define MAX_SOCKET_PATH 256
#define PATHCWD "/mnt/c/Users/Davide Parrini/Desktop/Progetto/GitHub/ProgettoSol_FileStorageServer"
char testDirPath[NAME_MAX];
char* socket_path;
int client_fd;
static int flag_stamp_op = 0;
static int msec_between_req = 0;

size_t print_bytes_readNFiles = 0;

int arg_h(char* s);
int arg_f(char* newsock,char* oldsock);
int arg_w(char* s,char* dir_rejectedFile);
int writeFileDir(char* dirpath,char* dir_rejectedFile,int n,int flag_end,size_t* written_bytes,int tutto_ok);
int arg_W(char* s,char* dir_rejectedFile);
int arg_r(char* s,char* dirname);
int arg_R(int n,char* dirname);
int arg_t(int mill_sec);
//int arg_l(char* s);
//int arg_u(char* s);
int arg_c(char* s);
int arg_p();

int arg_o(char* s);
int arg_a(char* s);
int arg_C(char* s);

int main(int argc, char *argv[]){
    char test[NAME_MAX] = {"/test"};
    strcat(testDirPath,PATHCWD);
    strcat(testDirPath,test);
    socket_path = malloc(sizeof(char)*MAX_SOCKET_PATH);
    memset(socket_path,0,sizeof(char)*MAX_SOCKET_PATH);
    
    struct timespec timer_connection = {20,0};
    time_t msec = 100;
    if(argc < 2){
        printf("Pochi argomenti!\n");
        exit(EXIT_FAILURE);
    }
    /*
    printf("Setup client!\n");
    printf("Digitare secondi di attesa di connessione al server:\n");
    scanf("%ld",&timer_connection.tv_sec);
    printf("\nDigitare millisecondi, in caso di mancata connessione, verrà rinviata la richiesta di connessione:\n");
    scanf("%ld",&msec);
*/
    int opt, temp_opt = 0;
    int termina = 0;
    int f = 0, h = 0,p = 0;

    while((opt = getopt(argc, argv,"hf:w:W:D:r:R:d:t:l:u:c:po:O:a:C:")) != -1 && !termina){

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

        case 'a': 
            push_char(opt,optarg);
            break;

        case 'C': 
            push_char(opt,optarg);
            break;

        case ':': 
            if(optopt == 'R'){
                push_char(optopt,"0");
                break;
            }
            if(optopt == 't'){
                arg_t(0);
                break;
            }

            printf( "l'opzione '-%c' richiede un argomento\n", opt);
            break;

        case '?': 
            printf("l'opzione '-%c' non e' gestita\n", optopt);
            break;

        default: break;
        }
        msleep(msec_between_req); 
        temp_opt = opt;
    }
    if(!f){
        printf("Nome del socket non è stato passato!\nL'opzione '-f' è obbligatoria!\n");
        exit(EXIT_FAILURE);
    }

    if(openConnection(socket_path,msec,timer_connection) == -1){
        perror("Non connesso, errore in openConnection");
        exit(EXIT_FAILURE);
    }
    while(!isEmpty_charq()){
        char_t* c = pop_char();
        switch (c->opt){
        case 'w':
            if(c->next != NULL){
                if(c->next->opt == 'D') arg_w(c->optarg,c->next->optarg);
                else arg_w(c->optarg,NULL);
            }   
            else arg_w(c->optarg,NULL);
            break; 

        case 'W': 
            if(c->next != NULL){
                if(c->next->opt == 'D') arg_W(c->optarg,c->next->optarg);
                else arg_W(c->optarg,NULL);
            }   
            else arg_W(c->optarg,NULL);
            break; 

        case 'D': 
            break;

        case 'r':
            if(c->next != NULL){
                if(c->next->opt == 'd') arg_r(c->optarg,c->next->optarg);
                else arg_r(c->optarg,NULL);
            }   
            else arg_r(c->optarg,NULL);
            break;

        case 'R':
            if(c->next != NULL){
                if(c->next->opt == 'd'){
                    printf("sono qua\n\n\n\n\n\n");
                    arg_R(atoi(c->optarg),c->next->optarg);
                }
                else arg_R(atoi(c->optarg),NULL);
            }   
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

        case 'a': 
            arg_a(c->optarg);
            break;

        case 'C': 
            arg_C(c->optarg);
            break;

        default:
            break;
        }

        free_char_t(c);
        msleep(msec_between_req);  
    }


    if(closeConnection(socket_path) == -1){
        perror("Errore closeConnection");
        fprintf(stderr, "closeConnection Value of errno : %d\n", errno);
    }
    return 0;
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


int arg_f(char* newsock,char* oldsock){
    strcpy(oldsock,newsock);
    printf("Connesso al socket: %s\n",oldsock);
    return 0;
}

int arg_w(char* s,char* dir_rejectedFile){
    int esito = 0;
    size_t contatore_bytes_scritti = 0;
    char* dirname = strtok(s,",");
    char* dirpath = malloc(sizeof(char) * NAME_MAX);
    findDir_getAbsPath(testDirPath ,dirname,&dirpath);

    char* nfile = strtok(NULL,",");
    int n, flag_end = 0;
    if(nfile != NULL) n = atoi(nfile);
    else n = 0;
    if(n == 0) flag_end = 1;

    esito = writeFileDir(dirpath,dir_rejectedFile,n,flag_end,&contatore_bytes_scritti,1);
    if(flag_stamp_op){
            time_t t_op = time(NULL);
            PRINT_OP("Writefile arg_w","writeNfile",&t_op,esito,contatore_bytes_scritti);
        } 

    return esito;
}

int writeFileDir(char* dirpath,char* dir_rejectedFile,int n,int flag_end,size_t* written_bytes,int tutto_ok){
    if(n == 0) return tutto_ok;

    if (chdir(dirpath) == -1) {
        printf("Errore cambio directory\n");
        return 0;
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
                if(writeFileDir(file->d_name,dir_rejectedFile,n,flag_end,written_bytes,tutto_ok) != 0){
                    if (chdir("..") == -1) {
                        printf("Impossibile risalire alla directory padre.\n");
                        return -1;
                    }
                }
            }
        }
        else{
            char* buf = malloc(sizeof(char) * NAME_MAX);
            findFile_getAbsPath(testDirPath,file->d_name,&buf);
            if(!writeFile(buf,dir_rejectedFile)){
                *written_bytes += statbuf.st_size;
                if(flag_end) n++;
                n--;
            }
            else{
                tutto_ok = 0;
            }
        }
    }
    if (errno != 0) perror("readdir in writeFileDir");
    closedir(dir);
    return 1;
}
int arg_W(char* s,char* dir_rejectedFile){
    int esito = 0;
    char* token = strtok(s,",");
    size_t size = 0;
    struct stat statbuf;

    char* bufdir = malloc(sizeof(char) * NAME_MAX);
    if(dir_rejectedFile != NULL){
        findDir_getAbsPath(testDirPath,dir_rejectedFile,&bufdir);
    }

    while(token != NULL){
        char* buf = malloc(sizeof(char) * NAME_MAX);
        findFile_getAbsPath(testDirPath,token,&buf);
        if((esito = writeFile(buf,dir_rejectedFile)) == -1){
            perror("Errore writeFile arg_W");
        }
        if (stat(buf, &statbuf)==-1) {	
            perror("Errore stat in writeFileDir");
        }
        size += statbuf.st_size;
        free(buf);
        token = strtok(NULL,",");
    }
    if(flag_stamp_op){
        time_t t_op = time(NULL);
        PRINT_OP("WriteFile arg_W",s,&t_op,esito,size);
    }
    free(bufdir);
    free(token);
    return esito;
}

int arg_r(char* s,char* dirname){
    char* token = strtok(s,",");
    char *absPath = malloc(sizeof(char) * NAME_MAX);
    memset(absPath,0, sizeof(char) * NAME_MAX);
    int esito = 0;
    size_t read_bytes = 0;
    if(dirname != NULL){
        findDir_getAbsPath(testDirPath, dirname, &absPath);
    }
    while(token != NULL && !esito){
        void* buff = NULL;
        size_t size = 0;
        
        char* path = malloc(sizeof(char) * NAME_MAX);
        findFile_getAbsPath(testDirPath,token,&path);
        if(!strlen(path)) esito = -1;
        else{
            
            if((esito = readFile(path,&buff,&size)) == -1){
                perror("Errore readFile");
            }
            if(!esito){
                printf("***Contenuto File***:\n%s\n\n",(char*)buff);  
                if(dirname != NULL){
                    char *dup = strndup(absPath,NAME_MAX);
                    strcat(dup,token);
                    int new_fd;            
                if((new_fd = open(dup,O_WRONLY|O_CREAT|O_TRUNC,0777)) == -1){
                        perror("Errore creazione file in arg_r");
                        esito = -1;
                    }
                    struct stat statbuf;
                    if(stat(dup, &statbuf)==-1) {	
                        perror("stat in arg_r");
                        esito = -1;
                    }
                    if(!statbuf.st_size){
                        if( write(new_fd,buff,size) == -1 ){
                            perror("Errore scrittura file in arg_r");
                        }
                    }
                    else printf("File %s già presente nella dir %s\n",token,dirname);
                    free(dup);
                    close(new_fd);
                }
            }
        }
        read_bytes += size;
        size = 0;   
        free(path); 
        free(buff);
        token = strtok(NULL,",");
    }
    free(absPath); 
    free(token);
    if(flag_stamp_op){
        time_t t_op = time(NULL);
        PRINT_OP("readFile",s,&t_op,esito,read_bytes);
    } 
    return 1;
}
int arg_R(int n, char* dirname){
    int esito = 0;
    char* bufdir = malloc(sizeof(char) * NAME_MAX);
    memset(bufdir,0, sizeof(char) * NAME_MAX);
    printf("dir %s\n\n",dirname);
    if(dirname != NULL){
        findDir_getAbsPath(testDirPath,dirname,&bufdir);
    }
    if((esito = readNFiles(n,bufdir)) == -1){
        perror("readNFile arg_R");
    }
    if(flag_stamp_op){
        time_t t_op = time(NULL);
        PRINT_OP("readNFile arg_R","N files",&t_op,esito,print_bytes_readNFiles);
        print_bytes_readNFiles = 0;
    } 
    free(bufdir);
    return esito;
}

int arg_t(int mill_sec){
    msec_between_req = mill_sec;
    printf("Attesa tra una richiesta e l'altra di %d msec\n",msec_between_req);
    return 0;
}

int arg_c(char* s){
    int esito = 0;
    char* token = strtok(s,",");
    while(token != NULL && !esito){
        char* buf = malloc(sizeof(char) * NAME_MAX);
        findFile_getAbsPath(testDirPath,token,&buf);
        if((esito = removeFile(buf)) == -1){
            perror("Errore removeFile");
        }
        if(flag_stamp_op){
            time_t t_op = time(NULL);
            size_t size = 0;
            PRINT_OP("removeFile",token,&t_op,esito,size);
        } 
        token = strtok(NULL,",");
        free(buf);
    }
    return esito; 
}

int arg_p(){
    if(flag_stamp_op) {
        printf("Stampe disabilitate!\n");
        flag_stamp_op = 0;
    }
    else{
        printf("Stampe abilitate!\n");
        flag_stamp_op = 1;
    }
    return 0;
}

int arg_o(char* s){
    int esito = 0,o_flag;
    char* flag = strtok(s,":");

    if(!strcmp(flag,"O_CREATE")) o_flag = O_CREATE;
    else if(!strcmp(flag,"O_LOCK")) o_flag = O_LOCK;
    else if(!strcmp(flag,"O_CREATE-O_LOCK")) o_flag = O_CREATE|O_LOCK;
    else if(!strcmp(flag,"O_NOFLAGS")) o_flag = O_NOFLAGS;
    char* token = strtok(NULL,",");
    while(token != NULL){
        char* buf = malloc(sizeof(char) * NAME_MAX);
        findFile_getAbsPath(testDirPath,token,&buf);
        if(!strlen(buf)){ 
            strcat(buf,testDirPath);
            strcat(buf,"/");
            strcat(buf,token);
        }
        if((esito = openFile(buf,o_flag)) == -1){
            perror("Errore openFile");
        }
        if(flag_stamp_op){
            time_t t_op = time(NULL);
            size_t size = 0;
            PRINT_OP("openFile",token,&t_op,esito,size);
        } 
        free(buf);
        token = strtok(NULL,",");
    }
    return esito; 
}


int arg_a(char* s){
    int esito = 0;
    char* token = strtok(s,":");
    char* content = strtok(NULL,"");
    char* buffer = malloc(sizeof(char) * NAME_MAX);

    findFile_getAbsPath(testDirPath,token,&buffer);
    if(!strlen(buffer)) esito = -1;
    else{
        if(appendToFile(buffer,(void*)content,strlen(content),NULL) == -1){
            perror("Errore appendToFile");
            esito = -1;
        }
    }
    if(flag_stamp_op){
        time_t t_op = time(NULL);
        PRINT_OP("appendToFile",token,&t_op,esito,sizeof(content));
    } 
    free(buffer);
    return esito; 
}

int arg_C(char* s){
    int esito = 0;
    char* token = strtok(s,",");
    while(token != NULL && !esito){
        char* buf = malloc(sizeof(char) * NAME_MAX);
        findFile_getAbsPath(testDirPath,token,&buf);
        if((esito = closeFile(buf)) == -1){
            perror("Errore closeFile");
        }
        if(flag_stamp_op){
            time_t t_op = time(NULL);
            size_t size = 0;
            PRINT_OP("closeFile",token,&t_op,esito,size);
        } 
        token = strtok(NULL,",");
        free(buf);
    }
    return esito; 
}