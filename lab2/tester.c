#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include "mythreads.h"

void *secondThread(void *arg);
void *third(void *arg);

int main() {
    threadInit();
    int t2 = threadCreate(&secondThread, NULL);
    threadCreate(&third, NULL);
    void **result = (void **)malloc(sizeof(void *));
    printf("2\n");
    threadYield();
    printf("4\n");
    threadJoin(t2, result);
    printf("6 %p\n", *result);
    free(result);
    threadExit(NULL);
}

void *secondThread(void *arg) {
    printf("1\n");
    //threadLock(1);
    threadYield();
    printf("3\n");
    threadYield();
    //threadUnlock(1);
    printf("5\n");
    return (void *)0x123123123123;
}

void *third(void *arg) {
    while(1) {
        //threadLock(1);
        printf("Shrek the Third\n");
        threadYield();
        //threadUnlock(1);
        printf("wenor\n");
        threadYield();
    }
}
