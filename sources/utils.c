#include <utils.h>


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

double bytesToMb(long bytes){
    return (double) bytes/1048576;
}
double bytesToKb(long bytes){
    return (double) bytes/1024;
}
unsigned long MbToBytes(double Mb){
  return (unsigned long) Mb*1048576;
}
unsigned long KbToBytes(double Kb){
  return (unsigned long) Kb*1024;
}
int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}