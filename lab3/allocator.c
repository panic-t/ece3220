/* <ryan ware>
ECE 3220 S23
project 3
allocator.c
*/

#include <stdio.h>
#include <stdlib.h>
#include "allocator.h"

#define END_FREE 0xFFFF
#define PTR_SIZE sizeof(void *)
#define PAGE_SIZE 4096

//USE BUILTIN CLZ


//allocates size bytes from memory, returns pointer to memory
void *malloc(size_t size) {
    if(size<1) {
        return NULL;
    }

}

//allocates size*n cleared bytes from memory, returns pointer to memory
void *calloc(size_t n, size_t size) {
    void *ret = malloc(n*size);
    if(ret!=NULL) {
        memset(ret, 0, n*size);
    }
    return ret;
}

//reallocates memory at ptr to size bytes, returns new ptr
void *realloc(void *ptr, size_t size) {
    if(ptr==NULL) {
        return NULL;
    }
    void *ret = malloc(size);
    if(ret==NULL) {
        free(ptr);
        return NULL;
    }
    size_t copy_size = real_size(ptr); //get old size then determine smaller
    copy_size = (copy_size>=size) ? size : copy_size;
    memcpy(ret, ptr, copy_size);
    free(ptr);
    return ret;
}

//returns ptr to unallocated memory
void free(void *ptr) {
    if(ptr==NULL) {
        return;
    }
}

//PRIVATE; returns real size of block given the pointer, returns 0 on invalid ptr
size_t real_size(void *ptr) {
    switch((((dmeta_t *)ptr)-1)->size) {
    case 0:
        return 2;
    case 1:
        return 4;
    case 2:
        return 8;
    case 3:
        return 16;
    case 4:
        return 32;
    case 5:
        return 64;
    case 6:
        return 128;
    case 7:
        return 256;
    case 8:
        return 512;
    case 9:
        return 1024;
    case 10:
        return (((pmeta_t *)(((dmeta_t *)ptr)-1))-1)->data.cus_size;
    default:
        return 0;
    }
}