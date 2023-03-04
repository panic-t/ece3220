#include <stdlib.h>
#include <stdio.h>

int main() {
    printf("start\n");
    void *ptr = malloc(24);
    printf("malloced\n");
    free(ptr);
    printf("freed\n");
    return 0;
}