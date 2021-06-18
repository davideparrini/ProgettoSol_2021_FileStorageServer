#include <utils.h>

int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r = 0;
    char *bufptr = (char*)buf;
    while(left>0) {
		if ((r=read((int)fd ,bufptr,left)) == -1) {
			if (errno == EINTR) continue;
			return -1;
		}
		if(r == 0) return 0;       
		left    -= r;
		bufptr  += r;
    }
    return size;
}

int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=write((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;  
    left    -= r;
	bufptr  += r;
    }
    return 1;
}

int isdot(const char dir[]) {
	int l = strlen(dir);
	
	if ( (l>0 && dir[l-1] == '.') ) return 1;
	return 0;
}

int isNumber(const char* s, long* n){
	if (s==NULL) return 1;
	if (strlen(s)==0) return 1;
	char* e = NULL;
	errno=0;
	long val = strtol(s, &e, 10);
	if (errno == ERANGE) return 2;    // overflow
	if (e != NULL && *e == (char)0) {
		*n = val;
		return 0;   // successo 
	}
	return 1;   // non e' un numero
}

double bytesToMb(size_t bytes){
    return (double) bytes/1048576;
}
double bytesToKb(size_t bytes){
    return (double) bytes/1024;
}
size_t MbToBytes(double Mb){
  	return (size_t) Mb*1048576;
}
size_t KbToBytes(double Kb){
  	return (size_t) Kb*1024;
}
int msleep(unsigned int tms) {
 	return usleep(tms * 1000);
}

int findFile_getAbsPath(const char dirPartenza[], const char nomefile[],char** resolvedpath){
	if(chdir(dirPartenza) == -1){
		return 0;
	}
	DIR* dir;
	if((dir = opendir(".")) == NULL){
		perror("errore apertura dir");
		return -1;
	}
	struct dirent *file;
	while( (errno=0, file = readdir(dir) ) != NULL) {
    	struct stat statbuf;
    	if (stat(file->d_name, &statbuf)==-1) {
			return -1;
		}
		if(S_ISDIR(statbuf.st_mode)){
			if(!isdot(file->d_name)){
				if(findFile_getAbsPath(file->d_name,nomefile,resolvedpath) != 0){
					if(chdir("..") == -1){
						perror("errore apertura dir padre");
						return -1;
					}
				}
			}
		}
		else{
			if(!strncmp(file->d_name, nomefile, strlen(nomefile))){
				getcwd(*resolvedpath,NAME_MAX);
				strncat(*resolvedpath,"/",2);
				strncat(*resolvedpath,nomefile,NAME_MAX);
			}
		
		}
	}
	PRINT_ERRNO(readdir,errno);
	closedir(dir);
	return 1;
}


int findDir_getAbsPath(const char dirPartenza[], const char dirToSearch[],char **resolvedpath){
	if(chdir(dirPartenza) == -1){
		return 0;
	}
	DIR* dir;
	if((dir = opendir(".")) == NULL){
		perror("errore apertura dir");
		return -1;
	}
	struct dirent *file;
	while( (errno=0, file = readdir(dir) ) != NULL) {
    	struct stat statbuf;
    	if (stat(file->d_name, &statbuf)==-1) {
			return -1;
		}
		if(S_ISDIR(statbuf.st_mode)){
			if(!isdot(file->d_name)){
				if(!strncmp(file->d_name, dirToSearch, strlen(dirToSearch))){
					getcwd(*resolvedpath,NAME_MAX);
					strncat(*resolvedpath,"/",2);
					strncat(*resolvedpath,dirToSearch,NAME_MAX);
					break;
				}
				else{
					if(findDir_getAbsPath(file->d_name,dirToSearch,resolvedpath) != 0){
						if(chdir("..") == -1){
							perror("errore apertura dir padre");
							return -1;
						}
					}
				}
			}	
		}
	}
	PRINT_ERRNO(readdir,errno);
	closedir(dir);
	return 1;
}



int sendContent(int socket_fd,void* content,size_t dim,int needFlagsOk){
	int flags_ok = 1;
	if(needFlagsOk){
		if(readn(socket_fd,&flags_ok,sizeof(int)) == -1){
			errno = EAGAIN;
			return 0;
		}
	}
	if(flags_ok){
		char buffer[dim];
		memset(buffer,0,dim+1);
		memcpy(buffer,content,dim+1);

		if(writen(socket_fd,&dim,sizeof(size_t)) == -1){
			errno = EAGAIN;
			return 0;
		}

		if(writen(socket_fd,buffer,dim+1) == -1){
			perror("SEND CONTENT ERROR IN WRITEN");
			return 0;
		}
		printf("content\n %s\n\n\n",buffer);
		return 1;
	}

}
int getContent(int client_fd, void *buff){
	
	size_t	size;
	if(readn(client_fd,&size,sizeof(size_t)) == -1){
        perror("GET CONTENT ERROR IN WRITEN");
		return 0;
    }
    char content[size];
    memset(content,0,size);

    if(readn(client_fd,&content,size) == -1){
        perror("GET CONTENT ERROR IN WRITEN 2");
		return 0;
    }
	buff = malloc(size);
	memset(buff,0,size);
	memcpy(buff,content,size);
	return 1;

}