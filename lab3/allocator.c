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
#define PTR_SIZE sizeof(void *)
#define PAGE_SIZE 4096

//USE BUILTIN CLZ

pmeta_t *heads[11] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
pmeta_t *lasts[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
//FILL OUT CAPS WITH NUMBER OF BLOCKS PER PAGE

//allocates size bytes from memory, returns pointer to memory
void *malloc(size_t size) {
    if(size<1) {
        return NULL;
    }
    int scode = 8*sizeof(unsigned int)-__builtin_clz((unsigned int)(size)); //size code
    char *phold, *ret;
    if(scode<2)
        scode = 1;
    if(scode>10)
        scode = 11;
    scode--;
    //scode should now be at index appropriate for size
    if(scode==10) { //custom blocks
        phold = (char *)mmap(NULL, sizeof(pmeta_t)+sizeof(dmeta_t)+size, 
PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        ((pmeta_t *)phold)->next = heads[10];
        if(heads[10]!=NULL) {
            heads[10]->prev = (pmeta_t *)phold;
        }
        ((pmeta_t *)phold)->prev = NULL;
        ((pmeta_t *)phold)->data.cus_size = size;
        phold = phold+sizeof(pmeta_t);
        ((dmeta_t *)phold)->size = scode;
        return (void *)(phold+sizeof(dmeta_t));
    }
    //noncustom blocks
    if(lasts[scode]==NULL||lasts[scode]->data.ncus_data.free_count==0) {
        //cannot access cached page/does not exist
        for(phold=(char *)heads[scode]; phold!=NULL&&
((pmeta_t *)phold)->data.ncus_data.free_count==0; phold=
(char *)((pmeta_t *)phold)->next); //incs phold to nonfull page or NULL if all full
        if(phold==NULL) { //for all full/empty list
            uint16_t capacity = (PAGE_SIZE-sizeof(pmeta_t))/(sizeof(dmeta_t)+
dec_size(scode));
            phold = (char *)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            ((pmeta_t *)phold)->next = heads[scode];
            if(heads[scode]!=NULL)
                heads[scode]->prev = (pmeta_t *)phold;
            heads[scode] = (pmeta_t *)phold;
            ((pmeta_t *)phold)->prev = NULL;
            ((pmeta_t *)phold)->data.ncus_data.free_count = capacity;
            ((pmeta_t *)phold)->data.ncus_data.next_free = 0;
            phold = phold+sizeof(pmeta_t);
            for(int i=0; i<capacity-1; i++) {
                ((dmeta_t *)phold)->index = i;
                ((dmeta_t *)phold)->size = scode;
                *((uint16_t *)(phold+sizeof(dmeta_t))) = i+1;
                phold = phold+sizeof(dmeta_t)+dec_size(scode);
            }
            ((dmeta_t *)phold)->index = capacity-1;
            ((dmeta_t *)phold)->size = scode;
            *((uint16_t *)(phold+sizeof(dmeta_t))) = END_FREE;
            //new page should be fully initialized
            phold = (char *)heads[scode];
        }
        lasts[scode] = (pmeta_t *)phold;
    } else {
        phold = (char *)lasts[scode];
    }
    //phold now holds nonempty page to get block from
    ((pmeta_t *)phold)->data.ncus_data.free_count--;
    ret = phold+sizeof(pmeta_t)+(((pmeta_t *)phold)->data.ncus_data.next_free)*
(sizeof(dmeta_t)+dec_size(scode))+sizeof(dmeta_t);
    ((pmeta_t *)phold)->data.ncus_data.next_free = *((uint16_t *)ret);
    //moved nextfree index
    return (void *)ret;
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
    dmeta_t *block;
    pmeta_t *page;
    block = ((dmeta_t *)ptr)-1;
    if(block->size!=10) { //noncustoms 
        page = (pmeta_t *)(((char *)block)-(sizeof(pmeta_t)+block->index*
(dec_size(block->size)+sizeof(dmeta_t))));
        *((uint16_t *)(block+1)) = page->data.ncus_data.next_free;
        page->data.ncus_data.next_free = block->index;
        page->data.ncus_data.free_count++;
        if(page->data.ncus_data.free_count!=(PAGE_SIZE-sizeof(pmeta_t))/
(sizeof(dmeta_t)+dec_size(block->size)))//if page is not empty
            //also see about trying to make this a constant lookup
            return;
        if(page==heads[block->size]) //page is head
            heads[block->size] = page->next;
        else //page is not head
            page->prev->next = page->next;
        if(page->next!=NULL) //page is not tail
            page->next->prev = page->prev;
        //now unmap empty page
        munmap((void *)page, PAGE_SIZE);
    } else { //customs
        page = ((pmeta_t *)block)-1;
        if(page==heads[10]) //page is head
            heads[10] = page->next;
        else //page is not head
            page->prev->next = page->next;
        if(page->next!=NULL) //page is not tail
            page->next->prev = page->prev;
        munmap((void *)page, sizeof(pmeta_t)+sizeof(dmeta_t)+
        page->data.cus_size);
    }
    return;
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

//PRIVATE; size decoder
size_t dec_size(int scode) {
    switch(scode) {
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
        return 1;
    default:
        return 0;
    }
}