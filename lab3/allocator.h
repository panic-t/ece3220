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
next page in list 

*/

typedef struct data_metablock_tag {
    int free : 1;
    int index : 11;
    int size : 4;
} dmeta_t;

struct  interior_data_tag{
    int free_count: 16;
    int next_free: 16;
};

typedef struct page_metablock_tag {
    void *next, *prev;
    union {
        struct interior_data_tag ncus_data;
        size_t cus_size;
    } data;
} pmeta_t;