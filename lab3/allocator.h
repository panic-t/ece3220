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

struct interior_data_tag{
    uint16_t free_count;
    uint16_t next_free;
};

typedef struct page_metablock_tag {
    struct page_metablock_tag *next, *prev;
    union {
        struct interior_data_tag ncus_data;
        size_t cus_size;
    } data;
} pmeta_t;
