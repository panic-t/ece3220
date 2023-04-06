
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SIZE_FROM 1024
#define SIZE_TO 6000

int main() {
    char *ptr = malloc(SIZE_FROM);
    char *cmpptr = malloc(SIZE_FROM);
    memset(ptr, 0x72, SIZE_FROM);
    memset(ptr+30, 0xaf, 30);
    memcpy(cmpptr, ptr, SIZE_FROM);
    ptr = realloc(ptr, SIZE_TO);
    if(memcmp(ptr, cmpptr, SIZE_FROM)==0) 
        printf("good\n");
    else
        printf("bad\n");
    return 0;
}