/* <ryan ware>
ECE 3220 S23
project 3
allocator.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "allocator.h"

#define END_FREE 0xFFFF
#define PAGE_SIZE 4096

pmeta_t *heads[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
pmeta_t *tails[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

//allocates size bytes from memory, returns pointer to memory
void *malloc(size_t size) {
    if(size<1) {
        return NULL;
    }
    int scode = 8*sizeof(unsigned int)-__builtin_clz((unsigned int)(size-1)); //size code
    char *phold;
    if(scode<2)
        scode = 1;
    if(scode>10)
        scode = 11;
    scode--;
    //scode should now be at index appropriate for size
    if(scode==10) { //custom blocks
        phold = (char *)mmap(NULL, sizeof(size_t)+sizeof(dmeta_t)+size, 
PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        *((size_t *)phold) = size;
        ((dmeta_t *)phold)->size = scode;
        return (void *)(phold+sizeof(dmeta_t));
    }
    //noncustoms
    if(heads[scode]==NULL) { //no free blocks, make new page and format
        tails[scode] = (pmeta_t *)mmap(NULL, PAGE_SIZE, 
PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        heads[scode] = tails[scode];
        phold = (char *)heads[scode];
        uint16_t capacity = (PAGE_SIZE-sizeof(pmeta_t))/(sizeof(dmeta_t)+
(2<<scode));
        ((pmeta_t *)phold)->next_free = 0;
        phold = phold+sizeof(pmeta_t);
        for(int i=0; i<capacity-1; i++) {
            ((dmeta_t *)phold)->index = i;
            ((dmeta_t *)phold)->size = scode;
            *((uint16_t *)(phold+sizeof(dmeta_t))) = i+1;
            phold = phold+sizeof(dmeta_t)+(2<<scode);
        }
        ((dmeta_t *)phold)->index = capacity-1;
        ((dmeta_t *)phold)->size = scode;
        *((uint16_t *)(phold+sizeof(dmeta_t))) = END_FREE;
    }

    //heads[scode] now holds nonempty page to get block from
    phold = ((char *)heads[scode])+sizeof(pmeta_t)+(heads[scode]->next_free)*
(sizeof(dmeta_t)+(2<<scode))+sizeof(dmeta_t);
    heads[scode]->next_free = *((uint16_t *)phold);
    //moved nextfree index
    //check if page is full, if full, kick from list
    if(heads[scode]->next_free==END_FREE) {
        heads[scode] = heads[scode]->next;
        if(heads[scode]==NULL) //if list is empty, empty tail accordingly
            tails[scode] = NULL;
    }
    return (void *)phold;
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
    char *data = ((char *)ptr)-sizeof(dmeta_t *);
    size_t copy_size;
    if(((dmeta_t *)data)->size==10) {
        data = data-sizeof(size_t);
        copy_size = *data;
    } else
        copy_size = 2<<(((dmeta_t *)data)->size);
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
    dmeta_t *block = ((dmeta_t *)ptr)-1;
    if(block->size!=10) { //noncustoms 
        pmeta_t *page = (pmeta_t *)(((char *)block)-(sizeof(pmeta_t)+block->index*
((2<<(block->size))+sizeof(dmeta_t))));
        if(page->next_free==END_FREE) { //if freeing from a full page
            if(heads[block->size]==NULL)
                heads[block->size] = page;
            else
                tails[block->size]->next = page;
            tails[block->size] = page;
            page->next = NULL;
        }
        *((uint16_t *)ptr) = page->next_free;
        page->next_free = block->index;
    } else { //customs
        size_t *size = ((size_t *)block)-1;
        munmap((void *)size, *size+sizeof(dmeta_t)+sizeof(size_t));
    }
    return;
}
