/* <ryan ware>
ECE 3220 S23
project 3
allocator.h
*/


/* data metablock - 2B (standard)
EIIIIIIIIIIISSSS
E - empty
I - index
S - size (2^(S+1))

custom data metablock - always 0x000A (2B)
will have page block immediately preceding

page metablock - ([size_t]+2[void *]B)
next page in list, prev page in list

*/

typedef struct data_metablock_tag {
    unsigned int index : 12;
    unsigned int size : 4;
} dmeta_t;


typedef struct page_metablock_tag {
    struct page_metablock_tag *next;
    uint16_t next_free;
} pmeta_t;


void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
