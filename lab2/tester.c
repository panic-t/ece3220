#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include "mythreads.h"

void *secondThread(void *arg);

int main() {
    int t2 = threadCreate(&secondThread, NULL);
    void **result;
    printf("2\n");
    threadYield();
    printf("4\n");
    threadJoin(t2, result);
    printf("6 %p\n", result);
    threadExit(NULL);
}

void *secondThread(void *arg) {
    printf("1\n");
    threadYield();
    printf("3\n");
    threadYield();
    printf("5\n");
    return (void *)0x12312312;
}