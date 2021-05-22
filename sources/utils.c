#include "utils.h"

int isNumber(const char* s, long* n) {
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

long bytesToMb(long bytes){
    return bytes/1048576;
}
