#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PIPENAME "/tmp/data_pipe"
#define PACKSIZE 80

void *(*orig_malloc)(size_t size) = NULL;
void (*orig_free)(void *ptr) = NULL;

void *malloc(size_t size) {
    printf("malloc shim\n");
    if(orig_malloc == NULL)
        orig_malloc = dlsym(RTLD_NEXT, "malloc");
    void *ret = (*orig_malloc)(size);
    FILE *pipe;
    char packet[PACKSIZE];
    if((pipe = fopen(PIPENAME, "w"))==NULL) {
        perror("error opening write pipe.\n");
        exit(0);
    }
    sprintf(packet, "m %p %li", ret, size);
    fputs(packet, pipe);
    fclose(pipe);
    return ret;
}

void free(void *ptr) {
    printf("free shim\n");
    if(orig_free == NULL)
        orig_free = dlsym(RTLD_NEXT, "free");
    (*orig_free)(ptr);
    FILE *pipe;
    char packet[PACKSIZE];
    if((pipe = fopen(PIPENAME, "w"))==NULL) {
        perror("error opening write pipe.\n");
        exit(0);
    }
    sprintf(packet, "f %p 0", ptr);
    fputs(packet, pipe);
    fclose(pipe);
    return;
}