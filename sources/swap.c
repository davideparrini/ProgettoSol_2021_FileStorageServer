#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
void swap(int* a, int* b){
    int temp = *a;
    *a = *b;
    *b= temp;
}
int main(){
    char *pathname = "caio";
    char*s = malloc(sizeof(char)*strlen(pathname)+1);
    memset(s,0,sizeof(s));
    printf("%s   %ld\n", s, strlen(pathname));

    strncpy(s,pathname,strlen(pathname));
    printf("%s aaaa\n",s );
  /*  int a = 5,b = 10;
    int msec = 1000;
    struct timespec abstime;
    abstime.tv_sec = 10;
    time_t before = time(NULL);
    time_t diff = 0;
    while(abstime.tv_sec > diff){
        printf("no swap\na: %d\nb: %d\n",a,b);
        swap(&a,&b);
        printf("swapped\na: %d\nb: %d\n",a,b);
        usleep(msec * 500);
        diff = time(NULL) - before;

    }
    */
}