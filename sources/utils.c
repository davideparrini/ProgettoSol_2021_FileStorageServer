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
		printf("numero bytes letti(r) = %d, left %ld\n",r,left);
		if(r == 0) return 0;       
		left    -= r;
		bufptr  += r;
		printf("left %ld\n",left);
    }
	printf("size in readn %d\n",(int)size);
    return (int)size;
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
int myRealPath(const char* actualpath, char** resolvedPath){
	char abspath[NAME_MAX];
    memset(&abspath,0,sizeof(abspath));
    realpath(actualpath,abspath);
	if(abspath == NULL){
        perror("Errore realpath");
        return 0;
    }
    strncpy(*resolvedPath,abspath,strlen(abspath));
	return 1;
}