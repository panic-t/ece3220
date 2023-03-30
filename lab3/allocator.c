/* <ryan ware>
ECE 3220 S23
project 3
*/

#include <stdio.h>
#include <stdlib.h>


//header block
//XXXX

//USE BUILTIN CLZ

//allocates size bytes from memory, returns pointer to memory
void *malloc(size_t size) {
    if(size<1) {
        return NULL;
    }

}

//allocates size*n cleared bytes from memory, returns pointer to memory
void *calloc(size_t n, size_t size) {
    if(size*n<1) {
        return NULL;
    }
}

//reallocates memory at ptr to size bytes, returns new ptr
void *realloc(void *ptr, size_t size) {
    if(ptr==NULL) {
        return NULL;
    }
    //what to do when size<1
}

//returns ptr to unallocated memory
void free(void *ptr) {
    if(ptr==NULL) {
        return;
    }
}